#ifndef GIT_ACTIONS_H
#define GIT_ACTIONS_H

#include "git2/credential.h"
#include <filesystem>
#include <git2.h>
#include <qprogressdialog.h>
#include <qthread.h>
#include <string>

class MultiProgressDialog : public QWidget{
    Q_OBJECT
    public:
        explicit MultiProgressDialog(QWidget *parent = nullptr, int bars = 1,
                const QString& windowTitle = tr("Synchronisation"));
        ~MultiProgressDialog();
        QProgressBar* getBar(int index){ return progressBars[index];}
        QLabel* getLabel(int index){ return progressLabels[index]; }

    private:
        uint64_t finishedBars;
        QList<QProgressBar*> progressBars;
        QList<QLabel*> progressLabels;

};

struct ProgressData { int p; MultiProgressDialog* progress; int mpdIndex; };
struct FetchheadData { char branchToMerge[100]; git_oid branchOidToMerge; };
struct AcquireCredsData { QWidget* parent; };

struct MergeOptions {
    ~MergeOptions(){ free((char **)heads); free(annotated); }
    const char **heads = nullptr;
    size_t heads_count = 0;

    git_annotated_commit **annotated = nullptr;
    size_t annotated_count = 0;

    uint8_t no_commit : 1 = 0;
};

struct CheckoutOptions{
    explicit CheckoutOptions(QWidget* parent) : CheckoutOptions(new MultiProgressDialog(parent, 1), 0){};
    explicit CheckoutOptions(MultiProgressDialog* dialog, int beginIndex);
    ~CheckoutOptions();

    MultiProgressDialog* progressDialog;
    ProgressData progressData;
    git_checkout_options gitOptions;
};




class GitManager{
    public:
    enum GitError{
        Success = 0,
        Open,
        Clone,
        Lookup,
        Fetch,
        Commit,
        Merge,
        BadRepo,
        ResolveRefish,
        Create,
        Fastforward,
        Add,
        Push,
        Pull,
        Index,
        BadAccess,
        RepoNotSetUp,
        RepoAlreadySetUp
    };

    explicit GitManager(QWidget* parent);
    ~GitManager();

    inline bool repoLoaded(){ return repo != nullptr; };
#define checkIsRepoSetup if(repo == nullptr) return RepoNotSetUp;


    [[nodiscard]]
    GitError readRepo(const std::string& repoPath, const std::string& repoUrl);

    [[nodiscard]]
    GitError gitClone(const std::string& url, const std::string& cloneToLocation);
    [[nodiscard]]
    GitError gitMerge(bool shouldCommit, const std::vector<std::string>& refishOptions = {"HEAD"}) const;
    [[nodiscard]]
    GitError gitFetch() const;
    [[nodiscard]]
    GitError gitPull() const;
    [[nodiscard]]
    GitError gitAdd(git_strarray* toAdd, bool allowNonUpdate) const;
    [[nodiscard]]
    GitError gitCommit(const std::string& commitMessage) const;
    [[nodiscard]]
    GitError gitPush() const;

    void setCredentialsHandler(const git_credential_acquire_cb& handler){ credentialsHandler = handler; };
    
private:
    QWidget* parentWidget;
    git_credential_acquire_cb credentialsHandler;
    git_repository* repo;
};

#endif //GIT_ACTIONS_H
