#include "ProjectEditWidget.h"
#include "EditorWidget.h"
#include "../app/LearnerWidget.h"
#include <QtWidgets>
#include <qnamespace.h>

#define deleteSafe(ptr) delete ptr; ptr = nullptr;

ProjectEditWidget::ProjectEditWidget(const std::filesystem::path& project, QWidget* parent) : projectConfigPath(project){
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    setWindowTitle("Projekteinstellungen");

    chooseImage = new QPushButton(this);
    chooseImage->setFixedSize(QSize(128, 128));
    chooseImage->setIconSize(chooseImage->minimumSize());
    QString associatedImage = QString::fromStdString(projectConfigPath.string()); 
    associatedImage.remove(".json");
    associatedImage.append(".png");
    projectImagePath = associatedImage.toStdString();
    loadImage();
    connect(chooseImage, &QPushButton::released, this, &ProjectEditWidget::onChooseImage);
    mainLayout->addWidget(chooseImage);

    QVBoxLayout* propertiesLayout = new QVBoxLayout(this);
    title = new QLineEdit(QString::fromStdString(projectConfigPath.stem().string()), this);
    propertiesLayout->addWidget(title);
 
    applyChanges = new QPushButton(tr("Speichern"), this);
    connect(applyChanges, &QPushButton::released, this, &ProjectEditWidget::onApplyChanges);
    propertiesLayout->addWidget(applyChanges);

    openProject = new QPushButton(tr("Öffnen"), this);
    connect(openProject, &QPushButton::released, this, &ProjectEditWidget::onOpenProject);
    propertiesLayout->addWidget(openProject);

    testProject = new QPushButton(tr("Testen"), this);
    connect(testProject, &QPushButton::released, this, &ProjectEditWidget::onTestProject);
    propertiesLayout->addWidget(testProject);    
   
    deleteProject = new QPushButton(tr("Löschen"), this);
    deleteProject->setStyleSheet("QPushButton{ color: red; };");
    connect(deleteProject, &QPushButton::released, this, &ProjectEditWidget::onDeleteProject);
    propertiesLayout->addWidget(deleteProject);
    
    mainLayout->addLayout(propertiesLayout);
    setLayout(mainLayout);
}

ProjectEditWidget::~ProjectEditWidget(){
    deleteSafe(title);
    deleteSafe(chooseImage);
    deleteSafe(applyChanges);
    deleteSafe(openProject);
    deleteSafe(deleteProject);
}

void ProjectEditWidget::loadImage(){
    QImage image = QImage(QString::fromStdString(projectImagePath.string())
            ).scaled(chooseImage->minimumSize(), Qt::KeepAspectRatioByExpanding);

    QPixmap previewImage = QPixmap(chooseImage->minimumSize());
    previewImage.fill(Qt::transparent);

    QPen pen;
    pen.setColor(Qt::transparent);
    pen.setJoinStyle(Qt::RoundJoin);

    QBrush imageBrush;
    QPainter painter(&previewImage);
    if(image.rect().isNull()){
        painter.setBrush(QBrush(Qt::white));
    }
    else painter.setBrush(image);

    painter.setPen(pen);
    painter.drawRoundedRect(0, 0, previewImage.width(), previewImage.height(), 50, 50, Qt::SizeMode::RelativeSize);
    painter.setRenderHints(QPainter::LosslessImageRendering | QPainter::Antialiasing);
    painter.end();

    chooseImage->setIcon(previewImage);
}


void ProjectEditWidget::onChooseImage(){
    QString fileName = QFileDialog::getOpenFileName(this, tr("Projektkarte"),
            "/home/Images", tr("Bilddateien (*.png)"));
    std::filesystem::path newImagePath = fileName.toStdString();
    if(!std::filesystem::exists(newImagePath)) return;

    if(newImagePath != projectImagePath) {
        std::filesystem::copy(newImagePath, projectImagePath,
                std::filesystem::copy_options::overwrite_existing
                | std::filesystem::copy_options::recursive);
    }

    loadImage();
}

void ProjectEditWidget::onApplyChanges(){
    std::string oldName = projectConfigPath.stem().string();
    std::string newName = title->text().toStdString();
    if(oldName != newName){
        std::filesystem::path newConfigPath = projectConfigPath.parent_path() / (newName + ".json");
        std::filesystem::path newImagePath = projectConfigPath.parent_path() / (newName + ".png");

        std::rename(projectConfigPath.string().c_str(), newConfigPath.string().c_str());
        std::rename(projectImagePath.string().c_str(), newImagePath.string().c_str());
        projectConfigPath = newConfigPath;
        projectImagePath = newImagePath;
    }
    emit projectUpdated(projectConfigPath);
}

void ProjectEditWidget::onOpenProject(){
    EditorWidget* editor = new EditorWidget(projectConfigPath, parentWidget());
    editor->showMaximized();
    editor->setAttribute(Qt::WA_DeleteOnClose);
}

void ProjectEditWidget::onTestProject(){
    LearnerWidget* learner = new LearnerWidget(projectConfigPath, parentWidget());
    learner->showMaximized();
    learner->setAttribute(Qt::WA_DeleteOnClose);
}

void ProjectEditWidget::onDeleteProject(){
    //delete whole project folder if there is one
    const std::string& projectName = projectConfigPath.stem().string();
    if(projectConfigPath.parent_path().filename().string().find(projectName) != std::string::npos){ 
        std::filesystem::remove_all(projectConfigPath.parent_path());
        return;
    }

    //otherwise delete the files known to be part of the project
    std::filesystem::remove(projectConfigPath);
    std::filesystem::remove(projectImagePath);
    emit projectRemoved();
}


#undef deleteSafe
