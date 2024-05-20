#include "GitActions.h"
#include "git2/repository.h"
#include <cstring>
#include <filesystem>
#include <git2.h>
#include <qboxlayout.h>
#include <qcoreapplication.h>
#include <qdatetime.h>
#include <qlabel.h>
#include <qnamespace.h>
#include <qprogressbar.h>
#include <qprogressdialog.h>

MultiProgressDialog::MultiProgressDialog(QWidget *parent, int bars, const QString& windowTitle) : finishedBars(0){
    QVBoxLayout* barsLayout = new QVBoxLayout();

    for(int i = 0; i < bars; i++){
        QProgressBar* bar = new QProgressBar(this);
        connect(bar, &QProgressBar::valueChanged, this, [this, i](int newVal){
                if(newVal >= progressBars[i]->maximum()){ finishedBars |= 1 << i; }
                for(int n = 0; n < progressBars.size(); n++){
                    if(!((finishedBars >> n) & 1)) return;
                }
                close();
        });
        QLabel* label = new QLabel(this);
        progressLabels.push_back(label);
        barsLayout->addWidget(label);

        barsLayout->addWidget(bar);
        progressBars.push_back(bar);
    }
    setMinimumSize(QSize(512, 128));
    setWindowModality(Qt::WindowModality::WindowModal);
    setWindowTitle(windowTitle);

    setLayout(barsLayout);
    show();
}

MultiProgressDialog::~MultiProgressDialog(){ 
    for(QProgressBar* bar : progressBars){
        delete bar;
    }
    for(QLabel* barLabel : progressLabels){
        delete barLabel;
    }
}

void processApplicationEvents(){
    static qint64 lastProcessed = QDateTime::currentMSecsSinceEpoch();
    qint64 ct = QDateTime::currentMSecsSinceEpoch();
    if(ct - 250 < lastProcessed) return;
    QCoreApplication::processEvents();
    lastProcessed = ct;
}

int fetchRepoProgress(const git_indexer_progress *stats, void *payload){
    ProgressData *pd = (ProgressData*)payload; 
    static qint64 lastProcessed = QDateTime::currentMSecsSinceEpoch();
    qint64 ct = QDateTime::currentMSecsSinceEpoch();
    if(ct - 250 < lastProcessed) return pd->p;

    processApplicationEvents();
    if(stats->received_objects > 0){
        if(pd->progress->getBar(pd->mpdIndex)->value() <= 100){
            pd->progress->getLabel(pd->mpdIndex)->setText("Differenzen wurden indexiert.");
            pd->progress->getBar(pd->mpdIndex)->setValue(100);
        }
        if(stats->received_objects == stats->total_objects){ 
            pd->progress->getLabel(pd->mpdIndex + 1)->setText("Objekte wurden empfangen. (" +
                    QString::number(stats->received_bytes) + "bytes)");
            pd->progress->getBar(pd->mpdIndex + 1)->setValue(100);
        }
        else{
            pd->progress->getLabel(pd->mpdIndex + 1)->setText("Objekte werden empfangen... " + 
                    QString::number(stats->received_objects) + "/" + QString::number(stats->total_objects) + " (" +
                    QString::number(stats->received_bytes) + "bytes)");
            pd->progress->getBar(pd->mpdIndex + 1)->setValue(float(100 * stats->received_objects)/
                    float(stats->total_objects));
        }

    }
    else{
        pd->progress->getLabel(pd->mpdIndex)->setText("Differenzen werden indexiert... " + 
                QString::number(stats->indexed_deltas) + "/" + QString::number(stats->total_deltas));
        pd->progress->getBar(pd->mpdIndex)->setValue(float(100 * stats->indexed_deltas)/float(stats->total_deltas));
    }

    return pd->p;
}

void checkoutRepoProgress(const char *path, size_t cur, size_t tot, void *payload){
    ProgressData *pd = (ProgressData*)payload;
    static qint64 lastProcessed = QDateTime::currentMSecsSinceEpoch();
    qint64 ct = QDateTime::currentMSecsSinceEpoch();
    if(ct - 250 < lastProcessed) return;

    processApplicationEvents();

    pd->progress->getLabel(pd->mpdIndex)->setText("Änderungen werden übernommen: " + QString::number(cur) + "/" + QString::number(tot));
    pd->progress->getBar(pd->mpdIndex)->setValue(float(100 * cur)/float(tot));
}

static int fetchhead_ref_cb(const char *name, const char *url, const git_oid *oid, unsigned int is_merge, void *payload)
{
    if ( is_merge )
    {
        FetchheadData* fetchData = (FetchheadData*)payload;
        strcpy_s( fetchData->branchToMerge, 100, name );
        memcpy( &fetchData->branchOidToMerge, oid, sizeof( git_oid ) );
    }
    return 0;
}

static void debugLog(const QString& str) { qDebug() << str; }

[[nodiscard]]
static int resolveRefish(git_annotated_commit **commit, git_repository *repo, const char *refish)
{
    git_reference *ref;
    git_object *obj;
    int err = 0;

    assert(commit != NULL);

    err = git_reference_dwim(&ref, repo, refish);
    if (err == GIT_OK) {
        git_annotated_commit_from_ref(commit, repo, ref);
        git_reference_free(ref);
        return 0;
    }

    err = git_revparse_single(&obj, repo, refish);
    if (err == GIT_OK) {
        err = git_annotated_commit_lookup(commit, repo, git_object_id(obj));
        git_object_free(obj);
    }

    return err;
}

    [[nodiscard]]
static int resolveHeads(git_repository *repo, MergeOptions *opts)
{
    git_annotated_commit** annotated = (git_annotated_commit**)calloc(opts->heads_count, sizeof(git_annotated_commit *));
    size_t annotated_count = 0;
    int err = 0;

    for (size_t i = 0; i < opts->heads_count; i++) {
        err = resolveRefish(&annotated[annotated_count++], repo, opts->heads[i]);
        if (err != 0) {
            debugLog(QString("failed to resolve refish %s: %s\n").arg(opts->heads[i]).arg(git_error_last()->message));
            annotated_count--;
            continue;
        }
    }

    if (annotated_count != opts->heads_count) {
        debugLog("unable to parse some refish\n");
        free(annotated);
        return -1;
    }

    opts->annotated = annotated;
    opts->annotated_count = annotated_count;
    return 0;
}

    [[nodiscard]]
static GitManager::GitError performFastForward(git_repository *repo, const git_oid *target_oid, int is_unborn)
{
    git_reference *target_ref;
    git_reference *new_target_ref;
    git_object *target = nullptr;
    int err = 0;

    if (is_unborn) {
        const char *symbolic_ref;
        git_reference *head_ref;

        // HEAD reference is unborn, lookup manually so we don't try to resolve it
        err = git_reference_lookup(&head_ref, repo, "HEAD");
        if (err != 0) {
            debugLog("failed to lookup HEAD ref\n");
            return GitManager::Fastforward;
        }

        // Grab the reference HEAD should be pointing to
        symbolic_ref = git_reference_symbolic_target(head_ref);

        // Create our master reference on the target OID
        err = git_reference_create(&target_ref, repo, symbolic_ref, target_oid, 0, NULL);
        if (err != 0) {
            debugLog("failed to create master reference\n");
            return GitManager::Fastforward;
        }

        git_reference_free(head_ref);
    } else {
        // HEAD exists, just lookup and resolve
        err = git_repository_head(&target_ref, repo);
        if (err != 0) {
            debugLog("failed to get HEAD reference\n");
            return GitManager::Fastforward;
        }
    }

    // Lookup the target object
    err = git_object_lookup(&target, repo, target_oid, GIT_OBJECT_COMMIT);
    if (err != 0) {
        debugLog(QString("failed to lookup OID %s\n").arg(git_oid_tostr_s(target_oid)));
        return GitManager::Fastforward;
    }

    //Checkout the result so the workdir is in the expected state
    CheckoutOptions checkoutOptions(nullptr);
    checkoutOptions.gitOptions.checkout_strategy = GIT_CHECKOUT_SAFE;
    err = git_checkout_tree(repo, target, &checkoutOptions.gitOptions);
    if (err != 0) {
        debugLog("failed to checkout HEAD reference\n");
        return GitManager::Fastforward;
    }

    //Move the target reference to the target OID 
    err = git_reference_set_target(&new_target_ref, target_ref, target_oid, NULL);
    if (err != 0) {
        debugLog("failed to move HEAD reference\n");
        return GitManager::Fastforward;
    }

    git_reference_free(target_ref);
    git_reference_free(new_target_ref);
    git_object_free(target);

    return GitManager::Success;
}

    [[nodiscard]]
static GitManager::GitError outputConflicts(git_index *index)
{
    git_index_conflict_iterator *conflicts;
    const git_index_entry *ancestor;
    const git_index_entry *our;
    const git_index_entry *their;
    int err = 0;

    if(git_index_conflict_iterator_new(&conflicts, index)){
        debugLog("failed to create conflict iterator");
        return GitManager::Index;
    }

    while ((err = git_index_conflict_next(&ancestor, &our, &their, conflicts)) == 0) {
        //TODO: replace with real conflict display
        debugLog(QString("conflict: a:%s o:%s t:%s\n").arg(
                    ancestor ? ancestor->path : "NULL").arg(
                        our->path ? our->path : "NULL").arg(
                            their->path ? their->path : "NULL"));
    }

    if (err != GIT_ITEROVER) {
        debugLog("error iterating conflicts\n");
        return GitManager::Index;
    }

    git_index_conflict_iterator_free(conflicts);
    return GitManager::Success;
}

    [[nodiscard]]
static GitManager::GitError createMergeCommit(git_repository *repo, git_index *index, struct MergeOptions *opts)
{
    git_oid tree_oid, commit_oid;
    git_tree *tree;
    git_signature *sign;
    git_reference *merge_ref = nullptr;
    git_annotated_commit *merge_commit;
    git_reference *head_ref;
    git_commit **parents = (git_commit**)calloc(opts->annotated_count + 1, sizeof(git_commit *));
    const char *msg_target = nullptr;
    size_t msglen = 0;
    char *msg;
    size_t i;
    int err;

    //Grab our needed references
    if(git_repository_head(&head_ref, repo)){
        debugLog("failed to get repo HEAD");
        free(parents);
        return GitManager::BadRepo;

    }

    if (resolveRefish(&merge_commit, repo, opts->heads[0])) {
        debugLog(QString("failed to resolve refish %s").arg(opts->heads[0]));
        free(parents);
        return GitManager::ResolveRefish;
    }

    //Maybe that's a ref, so DWIM it
    if(git_reference_dwim(&merge_ref, repo, opts->heads[0]) != 0){
        debugLog(QString("failed to DWIM reference %s").arg(git_error_last()->message));
        return GitManager::BadRepo;
    }

    //Grab a signature
    if(git_signature_now(&sign, "Me", "me@example.com") != 0){
        debugLog("failed to create signature");
        return GitManager::BadAccess;
    }

#define MERGE_COMMIT_MSG "Merge %s '%s'"
    //Prepare a standard merge commit message
    if (merge_ref != NULL) {
        if(git_branch_name(&msg_target, merge_ref)){
            debugLog("failed to get branch name of merged ref");
            return GitManager::BadRepo;
        }
    } else {
        msg_target = git_oid_tostr_s(git_annotated_commit_id(merge_commit));
    }

    msglen = snprintf(NULL, 0, MERGE_COMMIT_MSG, (merge_ref ? "branch" : "commit"), msg_target);
    if (msglen > 0) msglen++;
    msg = (char*)malloc(msglen);
    err = snprintf(msg, msglen, MERGE_COMMIT_MSG, (merge_ref ? "branch" : "commit"), msg_target);

    //This is only to silence the compiler
    if (err < 0) goto cleanup;

    //Setup our parent commits
    if(git_reference_peel((git_object **)&parents[0], head_ref, GIT_OBJECT_COMMIT)){
        debugLog("failed to peel head reference");
        free(parents);
        return GitManager::BadRepo;
    }
    for (i = 0; i < opts->annotated_count; i++) {
        git_commit_lookup(&parents[i + 1], repo, git_annotated_commit_id(opts->annotated[i]));
    }

    //Prepare our commit tree
    if(git_index_write_tree(&tree_oid, index)){
        debugLog("failed to write merged tree");
        free(parents);
        return GitManager::BadRepo;
    }
    if(git_tree_lookup(&tree, repo, &tree_oid)){
        debugLog("failed to lookup tree");
        free(parents);
        return GitManager::Lookup;
    }

    //Commit time !
    if(git_commit_create(&commit_oid, repo, git_reference_name(head_ref), sign, sign,
                NULL, msg, tree, opts->annotated_count + 1, parents) < 0){
        debugLog("failed to create commit");
        free(parents);
        return GitManager::Commit;
    }

    //We're done merging, cleanup the repository state
    git_repository_state_cleanup(repo);

cleanup:
    free(parents);
    if(err < 0) return GitManager::Commit;
    return GitManager::Success;
}

static  void opts_add_refish(MergeOptions* opts, const char* refish)
{
    size_t sz;

    assert(opts != NULL);

    sz = ++opts->heads_count * sizeof(opts->heads[0]);
    opts->heads = (const char**)realloc((void*)opts->heads, sz);
    opts->heads[opts->heads_count - 1] = refish;
}





CheckoutOptions::CheckoutOptions(MultiProgressDialog* dialog, int beginIndex){
    progressDialog = dialog;
    progressData = {0, progressDialog, beginIndex};

    gitOptions = GIT_CHECKOUT_OPTIONS_INIT;
    gitOptions.checkout_strategy = GIT_CHECKOUT_SAFE;
    gitOptions.progress_cb = checkoutRepoProgress;
    gitOptions.progress_payload = &progressData;
}

CheckoutOptions::~CheckoutOptions(){
    progressDialog->close();
    delete progressDialog;
}

GitManager::GitManager(QWidget* parent) : parentWidget(parent), credentialsHandler(nullptr), repo(nullptr){}

GitManager::~GitManager(){
    git_repository_free(repo);
}

GitManager::GitError GitManager::readRepo(const std::string& repoPath, const std::string& repoUrl){
    if(repo == nullptr){
        if(git_repository_open_ext(nullptr, repoPath.c_str(), GIT_REPOSITORY_OPEN_NO_SEARCH, nullptr) != 0){
            //repo doesn't even exist yet: clone
            return gitClone(repoUrl, repoPath);
        }
        else{
            int error = git_repository_open(&repo, repoPath.c_str());
            if(error < 0){
                return Open;
            } 
            else return Success;
        }
    }
    return RepoAlreadySetUp;
}

GitManager::GitError GitManager::gitClone(const std::string& url, const std::string& cloneToLocation){
    MultiProgressDialog* multiDialog = new MultiProgressDialog(parentWidget, 3);
    multiDialog->setWindowTitle("Synchronisation");

    ProgressData cloneData = {0, multiDialog, 0};
    git_clone_options cloneOpts = GIT_CLONE_OPTIONS_INIT;
    CheckoutOptions checkoutOptions(multiDialog, 2);
    cloneOpts.checkout_opts = checkoutOptions.gitOptions;
    cloneOpts.fetch_opts.callbacks.transfer_progress = fetchRepoProgress;
    cloneOpts.fetch_opts.callbacks.payload = &cloneData;

    if(repo == nullptr){
        git_repository_free(repo);
        repo = nullptr;
    }

    int error = git_clone(&repo, url.c_str(), cloneToLocation.c_str(), &cloneOpts);
    if(error < 0) return Clone;
    return Success;
}

GitManager::GitError GitManager::gitMerge(bool shouldCommit, const std::vector<std::string>& refishOptions) const{
    checkIsRepoSetup;
    MergeOptions opts;
    opts.no_commit = !shouldCommit;

    for(const std::string& option : refishOptions){
        opts_add_refish(&opts, option.c_str());
    }
    int state = git_repository_state(repo);
    if (state != git_repository_state_t::GIT_REPOSITORY_STATE_NONE) {
        debugLog(QString("repository is in unexpected state %d\n").arg(state));
        return BadRepo;
    }

    int err = resolveHeads(repo, &opts);
    if (err != 0) return BadRepo;
    git_merge_analysis_t analysis;
    git_merge_preference_t preference;
    if(git_merge_analysis(&analysis, &preference, repo,
                (const git_annotated_commit **)opts.annotated, opts.annotated_count) != 0){
        debugLog("merge analysis failed");
        return Merge;
    }

    if (analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) {
        debugLog("Already up-to-date\n");
        return Success;
    }
    else if (analysis & GIT_MERGE_ANALYSIS_UNBORN || (analysis & GIT_MERGE_ANALYSIS_FASTFORWARD &&
                !(preference & GIT_MERGE_PREFERENCE_NO_FASTFORWARD))) {
        //Fast-forward
        const git_oid *target_oid;
        if (analysis & GIT_MERGE_ANALYSIS_UNBORN) {
            debugLog("Unborn\n");
        } else {
            debugLog("Fast-forward\n");
        }

        // Since this is a fast-forward, there can be only one merge head
        target_oid = git_annotated_commit_id(opts.annotated[0]);
        assert(opts.annotated_count == 1);

        return performFastForward(repo, target_oid, (analysis & GIT_MERGE_ANALYSIS_UNBORN));
    }
    else if (analysis & GIT_MERGE_ANALYSIS_NORMAL) {
        git_merge_options merge_opts = GIT_MERGE_OPTIONS_INIT;
        CheckoutOptions checkoutOpts(nullptr);

        merge_opts.flags = 0;
        merge_opts.file_flags = GIT_MERGE_FILE_STYLE_DIFF3;

        checkoutOpts.gitOptions.checkout_strategy = GIT_CHECKOUT_FORCE|GIT_CHECKOUT_ALLOW_CONFLICTS;

        if (preference & GIT_MERGE_PREFERENCE_FASTFORWARD_ONLY) {
            debugLog("Fast-forward is preferred, but only a merge is possible\n");
            return Merge;
        }

        if(git_merge(repo, (const git_annotated_commit **)opts.annotated, opts.annotated_count,
                    &merge_opts, &checkoutOpts.gitOptions) != 0){
            debugLog("merge failed");
            return Merge;
        }
    }

    //If we get here, we actually performed the merge above
    git_index *index;
    if(git_repository_index(&index, repo) != 0){
        debugLog("failed to get repository index");
        return Merge;
    }

    if (git_index_has_conflicts(index)) {
        //Handle conflicts
        if(GitError err = outputConflicts(index); err != Success) return err;
    }
    else if (!opts.no_commit) {
        if(GitError err = createMergeCommit(repo, index, &opts); err != Success) return err;
        debugLog("Merge made\n");
    }

    return Success;
}

GitManager::GitError GitManager::gitFetch() const{
        checkIsRepoSetup;
        git_remote *remote = nullptr;
        //replace with fastforward
        int error = git_remote_lookup( &remote, repo, "origin" );
        if(error < 0){
            git_remote_free(remote);
            return Lookup;
        }

        MultiProgressDialog* multiDialog = new MultiProgressDialog(parentWidget, 2);
        multiDialog->setWindowTitle("Synchronisation");

        ProgressData fetchData = {0, multiDialog, 0};
        git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
        fetch_opts.callbacks.transfer_progress = fetchRepoProgress;
        fetch_opts.callbacks.payload = &fetchData;

        //this should not be needed as the repository is public
        //fetch_opts.callbacks.credentials = cred_acquire_cb;

        const git_indexer_progress *stats;

        error = git_remote_fetch( remote, {}, &fetch_opts, "fetch");

        git_remote_free(remote);

        multiDialog->close();
        delete multiDialog;
        if(error < 0){
            return Fetch;
        }
        return Success;
    }

GitManager::GitError GitManager::gitPull() const{
    checkIsRepoSetup;
    if(GitError err = gitFetch(); err != Success) return err;
    return gitMerge(false);
}

GitManager::GitError GitManager::gitAdd(git_strarray* toAdd, bool allowNonUpdate) const{
    checkIsRepoSetup;
    git_index_matched_path_cb matched_cb = nullptr;
    git_index *index = nullptr;

    // Grab the repository's index.
    if(git_repository_index(&index, repo) != 0){
        debugLog("Could not open repository index");
        return BadRepo;
    }

    // Perform the requested action with the index and files 
    if (!allowNonUpdate) {
        git_index_update_all(index, toAdd, nullptr, nullptr);
    } else {
        git_index_add_all(index, toAdd, 0, nullptr, nullptr);
    }

    // Cleanup memory
    git_index_write(index);
    git_index_free(index);
    return Success;
}

GitManager::GitError GitManager::gitCommit(const std::string& commitMessage) const{
    checkIsRepoSetup;
    git_tree *tree;
    git_index *index;
    git_object *parent = nullptr;
    git_signature *signature; 

    GitError returnError = Success;
    int error = git_revparse_single(&parent, repo, "HEAD");
    debugLog("printy statement");

    if (error == GIT_ENOTFOUND) {
        debugLog("HEAD not found. Creating first commit\n");
        error = 0;
    }
    else if (error != 0) {
        const git_error* err = git_error_last();
        if (err) debugLog(QString("ERROR %d: %s\n").arg(err->klass).arg(err->message));
        else debugLog(QString("ERROR %d: no detailed info\n").arg(error));
    }

    if (git_repository_index(&index, repo) != 0) {
        debugLog("Could not open repository index");
        returnError = Index;
        goto cleanup;

    }

    git_oid tree_oid;
    if (git_index_write_tree(&tree_oid, index) != 0) {
        debugLog("Could not write tree");
        returnError = Index;
        goto cleanup;
    }
    if (git_index_write(index) != 0) {
        debugLog("Could not write index");
        returnError = Index;
        goto cleanup;
    }

    if (git_tree_lookup(&tree, repo, &tree_oid)) {
        debugLog("Error looking up tree");
        returnError = Lookup;
        goto cleanup;
    }

    if (git_signature_default(&signature, repo)) {
        debugLog("Error creating signature");
        returnError = BadRepo;
        goto cleanup;
    }

    git_oid commit_oid;
    if(git_commit_create_v(&commit_oid, repo, "HEAD", signature, signature, nullptr,
                commitMessage.c_str(), tree, parent ? 1 : 0, parent)){
        debugLog("Error creating commit");
        returnError = Commit;
        goto cleanup;
    }


cleanup:
    git_index_free(index);
    git_signature_free(signature);
    git_tree_free(tree);
    git_object_free(parent);

    if(error < 0) return Commit;
    return returnError;
}

GitManager::GitError GitManager::gitPush() const{
    checkIsRepoSetup;
    git_push_options options;
    git_remote_callbacks callbacks;
    git_remote* remote = nullptr;
    char refspec[] = "refs/heads/master";
    char* prefspec = refspec;

    const git_strarray refspecs = { &prefspec, 1 };

    if(git_remote_lookup(&remote, repo, "origin" ) != 0){
        debugLog("Unable to lookup remote");
        return Lookup;
    }

    if(git_remote_init_callbacks(&callbacks, GIT_REMOTE_CALLBACKS_VERSION) != 0){
        debugLog("Error initializing remote callbacks");
        return BadRepo;
    }

    AcquireCredsData data = { parentWidget };
    callbacks.credentials = credentialsHandler;
    callbacks.payload = &data;

    if(git_push_options_init(&options, GIT_PUSH_OPTIONS_VERSION ) != 0){
        debugLog("Error initializing push");
        return BadRepo;
    }
    options.callbacks = callbacks;

    if(int pusherr = git_remote_push(remote, &refspecs, &options); pusherr != 0){
        //this can e.g. be an auth error
        debugLog("Error pushing");
        debugLog(QString::number(pusherr));
        return Push;
    }

    debugLog("pushed\n");
    return Success;
}
