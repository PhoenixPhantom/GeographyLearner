#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H

#include <QWidget>
#include <QProcess>
#include "json.hpp"

#define EDITDATA "Data/configData.json"

using json = nlohmann::json;

class QPushButton;
class QTextBrowser;
class QLabel;
class QTextLine;
class QLineEdit;
class MapWidget;
class QFile;

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget* parent = nullptr);
    ~MainWidget();

private slots:
    void onInputSubmitted();

private:
    MapWidget* map;
    QLabel* selector;
    QLabel* taskDescription;
    QLineEdit* answerLine;
    QPushButton* acceptButton; 

    json configData;

#ifdef EDITDATA
    QTextBrowser* fileBrowser;

    void onEditDataInputSubmitted();
    static std::string extractArgument(const std::string& container, const std::string& finder);
    static void extractArgumentList(const std::string& container, const std::string& finder, std::vector<std::string>& located);
 
#endif
};

#endif //MAIN_WIDGET_H
