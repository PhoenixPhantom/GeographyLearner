#include "MainWidget.h"
#include "MapWidget.h"
#include <QtWidgets>

MainWidget::MainWidget(QWidget* parent) : QWidget(parent){

    map = new MapWidget(QPixmap(":/Images/BlankMap-World-noborders.png"), QSize(10, 10), this);
 
    taskDescription = new QLabel(this);
    taskDescription->setText("Answer the following question: ");
    
    answerLine = new QLineEdit(this);
    acceptButton = new QPushButton(tr("Push this button"));

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->addWidget(map, 0, 0, 1, 2);
    mainLayout->addWidget(taskDescription, 1, 0, 1, 2);
    
    mainLayout->addWidget(answerLine, 2, 0);
    mainLayout->addWidget(acceptButton, 2, 1);

    setLayout(mainLayout);
    setWindowTitle(tr("A simple geography learner"));

    connect(acceptButton, SIGNAL(released()), this, SLOT(onButtonReleased()));
}

MainWidget::~MainWidget(){
    delete map;
    delete taskDescription;
    delete answerLine;
    delete acceptButton;
}

void MainWidget::onButtonReleased()
{
    taskDescription->setText(taskDescription->text() + answerLine->text() + " ?");
    answerLine->setText("");    
}
