#ifndef PROJECT_EDIT_WIDGET_H
#define PROJECT_EDIT_WIDGET_H

#include <QWidget>
#include <filesystem>
#include <qlineedit.h>
#include <qstandarditemmodel.h>

class QLabel;
class QListWidget;
class QPushButton;
class QListWidgetItem;



class ProjectEditWidget : public QWidget{
   Q_OBJECT

public:
    explicit ProjectEditWidget(const std::filesystem::path& project, QWidget* parent = nullptr);
    ~ProjectEditWidget();

signals:
    void projectUpdated(const std::filesystem::path& newPath);
    void projectRemoved();
 
private slots:
    void onChooseImage();
    void onApplyChanges();
    void onOpenProject();
    void onTestProject();
    void onDeleteProject();

private:
    QLineEdit* title;
    QPushButton* chooseImage;
    QPushButton* openProject;
    QPushButton* testProject;
    QPushButton* applyChanges;
    QPushButton* deleteProject;

    std::filesystem::path projectConfigPath;
    std::filesystem::path projectImagePath;

    void loadImage();
};

#endif
