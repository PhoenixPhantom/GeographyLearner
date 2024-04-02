#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H

#include <QWidget>
#include <QProcess>

class QPushButton;
class QTextBrowser;
class QLabel;
class QTextLine;
class QLineEdit;

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget* parent = nullptr);
    ~MainWidget();

private slots:
    void onButtonReleased();

private:
    QLabel* map;
    QLabel* selector;
    QLabel* taskDescription;
    QLineEdit* answerLine;
    QPushButton* acceptButton;
};

#endif //MAIN_WIDGET_H
