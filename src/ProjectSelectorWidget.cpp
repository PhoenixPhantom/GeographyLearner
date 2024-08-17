#include "ProjectSelectorWidget.h"
#include <qinputdialog.h>
#include <qprocess.h>
#include <qstandardpaths.h>
#if MAKE_EDITOR
#include "editor/ProjectEditWidget.h"
#include "editor/AcquireCredentialsWidget.h"
#else
#include "app/LearnerWidget.h"
#endif
#include "utils/GitActions.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <git2.h>
#include <iostream>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qpainter.h>
#if _WIN32
#include <cstdio>
#include <ShlObj.h>
#elif __APPLE__
#include <glob.h>
#else
#endif
#include <QProcess>
#include <qboxlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistwidget.h>
#include <qpushbutton.h>

#define deleteSafe(ptr) delete ptr; ptr = nullptr;

void LearningSetData::loadImage(){
    QString associatedImage = QString::fromStdString(learningsetPath.string()); 
    associatedImage.remove(".json");
    associatedImage.append(".png");
    QImage image = QImage(associatedImage).scaled(512, 256, Qt::KeepAspectRatioByExpanding);

    previewImage = QPixmap(512, 256);
    previewImage.fill(Qt::transparent);

    QPen pen;
    pen.setColor(Qt::transparent);
    pen.setJoinStyle(Qt::RoundJoin);

    QPainter painter(&previewImage);
    if(image.rect().isNull()){
        painter.setBrush(QBrush(Qt::white));
    }
    else painter.setBrush(image);
    painter.setPen(pen);
    painter.drawRoundedRect(0, 0, previewImage.width(), previewImage.height(), 50, 50);
    painter.setRenderHints(QPainter::LosslessImageRendering | QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.setPen(QPen(Qt::black));
    painter.setFont(QFont("Times", 20, QFont::Bold));
    painter.drawText(previewImage.rect(), Qt::AlignCenter, QString::fromStdString(learningsetPath.stem().string()));
    painter.end();
}

ProjectSelectorWidget::ProjectSelectorWidget(QWidget* parent) : QWidget(parent), couldUpdate(false),
    gitManager(this), currentFirst(0)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    setWindowTitle(tr("Lernsetauswahl"));
    
    title = new QLabel(tr("Vorhandene Lernsets"), this);
    QFont font = title->font();
    font.setWeight(QFont::ExtraBold);
    font.setPointSize(30);
    title->setFont(font);
    mainLayout->addWidget(title);

    QGridLayout* buttonLayout = new QGridLayout(this);
    for(int i = 0; i < 4; i++){
        QPushButton* button = new QPushButton(this);
        button->setFixedSize(QSize(512, 256));
        button->setIconSize(QSize(512, 256));
        connect(button, &QPushButton::released, this, [this, i](){ onOpenSet(currentFirst + i); });
        buttonLayout->addWidget(button, i/2, i%2);
        availableSetButtons[i] = button;
    }
    mainLayout->addLayout(buttonLayout);

    

    QHBoxLayout* editLayout = new QHBoxLayout(this);
#if MAKE_EDITOR
    gitManager.setCredentialsHandler(CredentialsManager::acquireCredentials);
    newSet = new QPushButton(this);
    newSet->setIcon(QIcon(":/Data/Plus.ico"));
    connect(newSet, &QPushButton::released, this, &ProjectSelectorWidget::onCreateNew);
    editLayout->addWidget(newSet, Qt::AlignLeft);

/*    removeSet = new QPushButton(this);
    removeSet->setIcon(QIcon(":/Data/Minus.ico"));
    connect(removeSet, &QPushButton::released, this, &ProjectSelectorWidget::onRemoved);
    editLayout->addWidget(removeSet, Qt::AlignLeft);*/

    editLayout->addSpacing(250);
#endif

    prevSets = new QPushButton(this);
    prevSets->setIcon(QIcon(":/Data/Left.ico"));
    QSizePolicy retainSize = prevSets->sizePolicy();
    retainSize.setRetainSizeWhenHidden(true);
    prevSets->setSizePolicy(retainSize);
    connect(prevSets, &QPushButton::released, this, [this](){ showSets(currentFirst - 4); });
    editLayout->addWidget(prevSets, Qt::AlignCenter);

    nextSets = new QPushButton(this);
    nextSets->setIcon(QIcon(":/Data/Right.ico"));
    retainSize = nextSets->sizePolicy();
    retainSize.setRetainSizeWhenHidden(true);
    nextSets->setSizePolicy(retainSize);
    connect(nextSets, &QPushButton::released, this, [this](){ showSets(currentFirst + 4); });
    editLayout->addWidget(nextSets, Qt::AlignCenter);

#if MAKE_EDITOR
    editLayout->addSpacing(250);

    publishAll = new QPushButton(this);
    publishAll->setIcon(QIcon(":/Data/Upload.ico"));
    connect(publishAll, &QPushButton::released, this, &ProjectSelectorWidget::onPublishAll);
    editLayout->addWidget(publishAll, Qt::AlignRight);
#endif

    updateList = new QPushButton(this);
    updateList->setIcon(QIcon(":/Data/Redo.ico"));
    connect(updateList, &QPushButton::released, this, &ProjectSelectorWidget::onListUpdate);
    editLayout->addWidget(updateList, Qt::AlignRight);

    mainLayout->addLayout(editLayout);
    setLayout(mainLayout);

    loadRepo();
}

ProjectSelectorWidget::~ProjectSelectorWidget(){
    /*Are all autmatically 'garbage-collected'
    deleteSafe(title);

    for(uint8_t i = 0; i < 4; i++){
        deleteSafe(availableSetButtons[i]);
    }
    free(availableSetButtons);

    deleteSafe(nextSets);
    deleteSafe(prevSets);
    deleteSafe(newSet);
    deleteSafe(removeSet);*/
}

void ProjectSelectorWidget::onOpenSet(int index){
    if(availableSets.size() > index && index >= 0){
#if MAKE_EDITOR
        ProjectEditWidget* editWidget = new ProjectEditWidget(availableSets[index].learningsetPath, this);
        connect(editWidget, &ProjectEditWidget::projectUpdated, this, [this, index, editWidget]
                (const std::filesystem::path& newPath){
                assert(index < availableSets.size());

                availableSets[index].learningsetPath = newPath;
                availableSets[index].loadImage();
                showSets(currentFirst);
                editWidget->close();
                editWidget->deleteLater();
        });
        connect(editWidget, &ProjectEditWidget::projectRemoved, this, [this, index, editWidget](){
                assert(index < availableSets.size());

                for(auto it = availableSets.begin(); it != availableSets.end(); it++){
                   if(availableSets[index] != *it) continue;
                   availableSets.erase(it);
                   break;
                }

                showSets(int(currentFirst/4)*4);
                editWidget->close();
                editWidget->deleteLater();
        });
        editWidget->setWindowModality(Qt::NonModal);
        editWidget->show();
        editWidget->setAttribute(Qt::WA_DeleteOnClose);
#else
    LearnerWidget* learner = new LearnerWidget(availableSets[index].learningsetPath, parentWidget());
    learner->showMaximized();
    learner->setAttribute(Qt::WA_DeleteOnClose);
#endif
    }
}


#if MAKE_EDITOR
void ProjectSelectorWidget::onPublishAll(){
    GitManager::GitError err;
    if(!gitManager.repoLoaded()){
        loadRepo();
        if(!gitManager.repoLoaded()) {
            return;
        }
    }

    /*git_strarray strarray;
    strarray.strings = new char*[1];
    strarray.strings[0] = learningSetsPath.string().data();
    strarray.count = 1;

    //this doesn't seem to really work
    if (err = gitManager.gitAdd(&strarray, true); err != GitManager::Success) {
        showError(err);
        delete[] strarray.strings;
        return;
    }

    delete[] strarray.strings;*/

    std::string gitPath;
    QString activePaths = getenv("PATH");
#if _WIN32
    //windows splitting
    QStringList paths = activePaths.split(";");
#else
    //linux and MacOS use the same splitting in the PATH env variable
    QStringList paths = activePaths.split(":");
#endif
#if __APPLE__
    activePaths = "/usr/bin:" + activePaths;
#endif

    for(const QString& path : paths){
        std::filesystem::path localPath = path.toStdWString();
        if(!std::filesystem::is_directory(localPath)){
            continue;
        }
        for(const std::filesystem::path& potentialGitFile :
                std::filesystem::directory_iterator(localPath)){
            if(potentialGitFile.stem() != "git") continue;
            gitPath = potentialGitFile.string();
            goto loopEnd;
        }
    }

loopEnd:


    if(gitPath.empty()){
        activePaths.replace(";", "\n");
    QMessageBox::critical(this, tr("Fehlende Komponente"), tr("Um fortfahren zu können wird ‘git‘ benötigt. \
Die Links zu den benötigten Komponenten können auf der Seite des GeographyLearners unter der Sektion \
Download im Dropdown zu den externen Programmen gefunden und installiert werden."));

        qDebug() << "Git installation wurde NICHT GEFUNDEN. \n PATH=" + activePaths;
        return;
    }

    QString string = QString::fromStdString(learningSetsPath.string());
#if _WIN32
    //windows is the only platform that uses stupid backslashes (\) in pathnames
    string.replace("\\", "\\\\");
#endif
    //There seem to be constant errors with libgit2::push (most likely due to GitHub's unique auth system)
    /*if(err = gitManager.gitPush(); err != GitManager::Success){
          showError(err);
          return;
    }*/
#if _WIN32
    if (string.contains(" ")) {
        string = "\'" + string + "\'";
    }
    int rval = system(("\"" + QString::fromStdString(gitPath) + "\" -C " + string + " add *").toUtf8());
#else
    int rval = system(("\"" + QString::fromStdString(gitPath) + "\" -C \'" + string + "\' add \'" + string + "/*\'").toUtf8());
    if (string.contains(" ")) {
        string = "\'" + string + "\'";
    }
#endif




    if (rval == 0){

        bool ok;
        QString commitMessage = 
            QInputDialog::getText(this, tr("Beschreibung"),
                    tr("Beschreibe hier in wenigen Stichworten, was du seit dem letzten Hochladen \
verändert hast\n(obligatorisch):"), QLineEdit::Normal, "", &ok);
        if (!(ok && !commitMessage.isEmpty())) return;

        std::filesystem::path configPath = learningSetsPath.parent_path().parent_path() / "GL.conf";
        bool showWarning = true;
        if(std::filesystem::exists(configPath)){
            std::ifstream configFile(configPath);
            char linebuff[6]; //2(key) + 2(': ') + 2(data)
            while(!configFile.fail() && !configFile.eof()){
                configFile.getline(linebuff, 6);
                //ensure correct format
                if(linebuff[2] != ':' || linebuff[3] != ' ') goto Continue;
                //Upload warning
                else if(linebuff[0] == 'U' && linebuff[1] == 'W'){
                    if(!showWarning) goto Continue;
                    showWarning = (linebuff[4] == '1');
                }

Continue:
                memset(linebuff, 0, 6);
            }
        }

        if(showWarning){
            if(QMessageBox::StandardButton response = QMessageBox::warning(this, tr("Uploadwarnung"),
                        tr("Wenn du fortfahrst, werden all deine Änderungen auf die öffentlich \
                            zugängliche Webseite des GeographyLearners geladen. (Bei ‘ignorieren‘ wird die Nachricht beim Nächsten Upload wieder angezeigt werden)"),
                        QMessageBox::Cancel | QMessageBox::Ok | QMessageBox::Ignore);
                    response == QMessageBox::Ok){
                std::ofstream stream(configPath);
                stream << "UW: 0";
                stream.close();
            }
            else if(response != QMessageBox::Ignore) return;
        }
        rval = system(("\"" + QString::fromStdString(gitPath) + "\" -C " + string + " commit -m \"" + commitMessage + "\"").toUtf8());
        if (rval != 0) {
            QMessageBox::warning(this, tr("Login nötig"), tr("Besuche die Webseite des GeographyLearners, gehe zur Sektion 'Download' \
                        und öffne das Dropdown zu den externen Programmen, um eine vereinfachte Anmeldeanleitung für die das git tool zu finden."), QMessageBox::Ok, QMessageBox::Ok);
            return;
        }

    }
    else {
        QMessageBox::information(this, tr("Fehler"), tr("Die Änderungen können nicht übernommen werden. (add changes failed)"), QMessageBox::Ok, QMessageBox::Ok);
        return;
    }


    rval = system(("\"" + QString::fromStdString(gitPath) + "\" -C " + string + " push https://github.com/PhoenixPhantom/GeographyLearnerLearningSets.git").toUtf8());
    if (rval == 0){
        QMessageBox::information(this, tr("Fertig"), tr("Die Synchronisation war erfolgreich."), QMessageBox::Ok, QMessageBox::Ok);
        return;
    }
    rval = system(("\"" + QString::fromStdString(gitPath) + "\" -C " + string + " pull https://github.com/PhoenixPhantom/GeographyLearnerLearningSets.git").toUtf8());
    if (rval == 0){
        QMessageBox::warning(this, tr("Merge"), tr("Es wurde eine mögliche Diskrepanz zwischen den loklen Lernsets\
und denen auf der Webseite erkannt. (Falls nötig, DRÜCKE im folgenden Fenster \":\" dann \"q\" und dann ENTER um fortzufahren)"), QMessageBox::Ok, QMessageBox::Ok);
   rval = system(("\"" + QString::fromStdString(gitPath) + "\" -C " + string + " merge").toUtf8());
        if (rval == 0){
            rval = system(("\"" + QString::fromStdString(gitPath) + "\" -C " + string + " push https://github.com/PhoenixPhantom/GeographyLearnerLearningSets.git").toUtf8());
            if (rval == 0){
                QMessageBox::information(this, tr("Fertig"), tr("Die Synchronisation war erfolgreich. (Verschiedene Versionen wurden kombiniert)"), QMessageBox::Ok, QMessageBox::Ok);
                return;
            }
            else {
                QMessageBox::warning(this, tr("Login nötig"), tr("Besuche die Webseite des GeographyLearners, gehe zur Sektion 'Download' \
und öffne das Dropdown zu den externen Programmen, um eine vereinfachte Anmeldeanleitung für die das git tool zu finden."), QMessageBox::Ok, QMessageBox::Ok);
            }
        }
        else {
            QMessageBox::warning(this, tr("Unvereinbare änderungen"), tr("Der jetzige Zustand der Lernsets ist unvereinbar mit dem auf der Webseite.\
Kontaktiere den Entwickler des Programmes für Hilfe."));
        }
    }
    else {
        QMessageBox::warning(this, tr("Keine Verbindung"), tr("Es kann keine Verbindung mit dem Zielspeicher hergestellt werden.\
KMöglicherweise ist der Computer nicht mit dem Internet verbunden oder eine alte Version des Learners wird verwendet."));
    }


    QMessageBox::information(this, tr("Upload fehlgeschlagen"), tr("Der Versuchte Upload ist fehlgeschlagen."), QMessageBox::Ok, QMessageBox::Ok);
    return;
}

void ProjectSelectorWidget::onCreateNew(){
    std::filesystem::path emptyPath = learningSetsPath / "Neues Set.json";
    for(int i = 1; std::filesystem::exists(emptyPath); i++){
        emptyPath = learningSetsPath / ("Neues Set_" + std::to_string(i) + ".json");
    }

    std::ofstream fileWriter(emptyPath);
    fileWriter << "{\n}" << std::endl;
    fileWriter.close();
    LearningSetData data;
    data.learningsetPath = emptyPath;
    data.loadImage();
    availableSets.push_back(data);

    showSets(int((availableSets.size()-1)/4)*4);
}
#endif

void ProjectSelectorWidget::onListUpdate(){
    loadRepo();
}



void ProjectSelectorWidget::loadRepo(){
    //Surely there must be a better way than whatever this mess is.
    std::filesystem::path appDataLocation;
#if _WIN32
    // KF_FLAG_CREATE force folder creation if not exists
    PWSTR appdata = NULL;
    if (SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &appdata) != S_OK) {
        std::cerr << "Cannot determine local AppData location!";
        std::abort();
    }
    appDataLocation = appdata;
    appDataLocation /= "GeographyLearner";
#elif __APPLE__
    //this expands the given path to lead to what the tilde symbol actually means (the home dir)
    glob_t globbuf;
    if(glob("~/Library/Application Support/", GLOB_TILDE, NULL, &globbuf) == 0){
        char** v = globbuf.gl_pathv; //list of matched pathnames
        char* expandedPath = v[0]; //number of matched pathnames, gl_pathc == 1
        appDataLocation = expandedPath;
        globfree(&globbuf);
    }
    appDataLocation /= "GeographyLearner";

#else //most likely __linux__
    appDataLocation = "~/.GeographyLearner";
#endif

    std::filesystem::path repoLocation = appDataLocation / "Repo";
    if(!std::filesystem::exists(repoLocation)){
        std::filesystem::create_directories(repoLocation);
    }
    std::string pathToRepo = repoLocation.string();
    if(!gitManager.repoLoaded()){
        //NOTE: this is the path that leads to craches
        if(GitManager::GitError err = gitManager.readRepo(pathToRepo, repoUrl); err != GitManager::Success){
            showError(err);
        }
    } 
    else{
        if(GitManager::GitError err = gitManager.gitPull(); err != GitManager::Success){
            showError(err);
        }
    }
    
    learningSetsPath = repoLocation;
    if(!std::filesystem::exists(learningSetsPath)){
        QMessageBox::critical(this, windowTitle(), tr("Die Struktur der Daten entspricht nicht dem erwarteten Schema: \
Keinerlei Daten konnten geladen werden..."), QMessageBox::Close, QMessageBox::Close);
        return;
    }

    std::map<std::filesystem::path, bool> existingPaths;

    for(const std::filesystem::path& subitem : std::filesystem::recursive_directory_iterator(learningSetsPath)){
        if(subitem.extension() == ".json"){
            existingPaths[subitem] = false;
        }   
    }
    
    //avoid recalculating all images
    std::vector<LearningSetData> toRemove;
    for(auto it = availableSets.begin(); it != availableSets.end(); it++){
        const std::filesystem::path& path = it->learningsetPath;
        if(existingPaths.contains(path)){
            existingPaths[path] = true;
            continue;
        }
        toRemove.push_back(*it);        
    }
    for(const LearningSetData& remove : toRemove){
        availableSets.erase(std::find(availableSets.begin(), availableSets.end(), remove));
    }

    for(const auto& newElement : existingPaths){
        if(newElement.second) continue;
        LearningSetData data;
        data.learningsetPath = newElement.first;
        data.loadImage();
        availableSets.push_back(data);
    }

    showSets(currentFirst);
}

void ProjectSelectorWidget::showSets(int first){
    if(first < 0) first = 0;
    if(first >= availableSets.size()) first = int((availableSets.size() - 1)/4)*4;

    currentFirst = first;
    nextSets->setEnabled(first + 4 < availableSets.size());
    nextSets->setVisible(nextSets->isEnabled());
    prevSets->setEnabled(first > 0);
    prevSets->setVisible(prevSets->isEnabled());

    for(int i = 0; i < 4; i++){
        QPushButton* relevantButton = availableSetButtons[i];
        if(i + first >= availableSets.size()){
            relevantButton->setIcon(QIcon());
            relevantButton->setEnabled(false);
            continue;
        }
        relevantButton->setEnabled(true);

        relevantButton->setText("");
        relevantButton->setIcon(availableSets[i + first].previewImage);

    }
}

void ProjectSelectorWidget::showError(GitManager::GitError error){
    QString errorMessage;
#if MAKE_EDITOR
    switch(error){
        case GitManager::Success:
            return;
        case GitManager::Open:
            errorMessage = "OPEN";
            break;
        case GitManager::Clone:
            errorMessage = "CLONE";
            break;
        case GitManager::Lookup:
            errorMessage = "LOOKUP";
            break;
        case GitManager::Fetch:
            errorMessage = "FETCH";
            break;
        case GitManager::Commit:
            errorMessage = "COMMIT";
            break;
        case GitManager::Merge:
            errorMessage = "MERGE";
            break;
        case GitManager::BadRepo:
            errorMessage = "BAD_REPO";
            break;
        case GitManager::ResolveRefish:
            errorMessage = "RESOLVE_REFISH";
            break;
        case GitManager::Create:
            errorMessage = "CREATE";
            break;
        case GitManager::Fastforward:
            errorMessage = "FAST_FORWARD";
            break;
        case GitManager::Add:
            errorMessage = "ADD";
            break;
        case GitManager::Push:
            errorMessage = "PUSH";
            break;
        case GitManager::Pull:
            errorMessage = "PULL";
            break;
        case GitManager::Index:
            errorMessage = "INDEX";
            break;
        case GitManager::BadAccess:
            errorMessage = "BAD_ACCESS";
            break;
        case GitManager::RepoNotSetUp:
            errorMessage = "REPO_NOT_SET_UP";
            break;
        case GitManager::RepoAlreadySetUp:
            errorMessage = "REPO_ALREADY_SET_UP";
            break;
        }
            QMessageBox::critical(this, windowTitle(), tr("Daten konnten nicht \
synchronisiert werden. (") + errorMessage + " ERROR)" + tr(" (Ist der Computer mit dem Internet verbunden?) \
Sollten in diesem Zustand Änderungen vorgenommen werden, so ist es gut möglich, \
dass diese nicht übernommen werden können."),
                    QMessageBox::Close, QMessageBox::Close);
#endif
}

#undef deleteSafe
