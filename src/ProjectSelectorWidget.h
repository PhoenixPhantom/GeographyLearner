#ifndef PROJECT_SELECTOR_WIDGET_H
#define PROJECT_SELECTOR_WIDGET_H

#include "utils/GitActions.h"

#include <QWidget>
#include <filesystem>
#include <qlineedit.h>
#include <qstandarditemmodel.h>
#include <string>
//#include "json.hpp"

//using json = nlohmann::json;

class QLabel;
class QListWidget;
class QPushButton;
class QListWidgetItem;


struct LearningSetData{
    std::filesystem::path learningsetPath;
    QPixmap previewImage;
    void loadImage();
    bool operator==(const LearningSetData& data){
        return data.learningsetPath == learningsetPath;
    }
    bool operator!=(const LearningSetData& data){
        return !operator==(data);
    }
};

class ProjectSelectorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectSelectorWidget(QWidget* parent = nullptr);
    ~ProjectSelectorWidget();
    inline static std::string repoUrl =// "https://github.com/libgit2/libgit2.git";
    //"ssh://git@github.com:PhoenixPhantom/GeographyLearner.git";
    "https://github.com/PhoenixPhantom/GeographyLearner.git";
 
private slots:
    void onOpenSet(int index);

#if MAKE_EDITOR
    void onPublishAll();
    void onCreateNew();
    void onRemoved();
    void projectDescriptionUpdated(int projectIndex);
#endif
    void onListUpdate();

private:
    QLabel* title;
    QPushButton* availableSetButtons[4];

    QPushButton* nextSets;
    QPushButton* prevSets;

#if MAKE_EDITOR
    QPushButton* newSet;
    QPushButton* removeSet;
    QPushButton* publishAll;
#endif 
    QPushButton* updateList;

    int currentFirst;
    std::vector<LearningSetData> availableSets;
    std::filesystem::path learningSetsPath;

    bool couldUpdate;
    GitManager gitManager;

    void loadRepo();
    void showSets(int first = 0);
    void showError(GitManager::GitError error);
};

#endif //PROJECT_SELECTOR_WIDGET_H
