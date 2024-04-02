#include "MainWidget.h"

#include <QtWidgets>

MainWidget::MainWidget(QWidget* parent) : QWidget(parent){
    
    map = new QLabel();
    map->setPixmap(QPixmap("BlankMap-World-noborders.png"));
    
    selector = new QLabel();
    selector->setPixmap(QPixmap("DotSelector.png"));
    
    taskDescription = new QLabel();
    taskDescription->setText("Answer the following question: ");
    
    answerLine = new QLineEdit();
    acceptButton = new QPushButton(tr("Push this button"));

    QGridLayout* mainLayout = new QGridLayout();
    QStackedLayout* layout = new QStackedLayout();
    layout->addWidget(map);
    layout->addWidget(selector);
//    layout->setCurrentWidget(selector);
    mainLayout->addLayout(layout, 0, 0);
    mainLayout->addWidget(taskDescription, 1, 0);
    QGridLayout* answerSublayout = new QGridLayout();
    answerSublayout->addWidget(answerLine, 0, 0);
    answerSublayout->addWidget(acceptButton, 0, 1);
    mainLayout->addLayout(answerSublayout, 2, 0);

    setLayout(mainLayout);
    setWindowTitle(tr("A simple geography learner"));

    connect(acceptButton, SIGNAL(released()), this, SLOT(onButtonReleased()));
}

MainWidget::~MainWidget(){
    delete map;
    delete selector;
    delete taskDescription;
    delete answerLine;
    delete acceptButton;
}

void MainWidget::onButtonReleased()
{
    taskDescription->setText(taskDescription->text() + answerLine->text() + " ?");
    answerLine->setText("");    
}
