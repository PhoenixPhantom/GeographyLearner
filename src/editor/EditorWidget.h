#ifndef EDITOR_WIDGET_H
#define EDITOR_WIDGET_H

#include <QWidget>
#include <QProcess>
#include "../utils/json.hpp"

using json = nlohmann::json;

class QPushButton;
class QTextBrowser;
class QLabel;
class QTextLine;
class QLineEdit;
class MapWidget;
class QFile;

class EditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EditorWidget(QWidget* parent = nullptr);
    ~EditorWidget();

protected:
    void resizeEvent(QResizeEvent* event) override;
private slots:
    void onInputSubmitted();
private:
    MapWidget* map;
    QLabel* selector;
    QLabel* taskDescription;
    QLineEdit* answerLine;
    QPushButton* acceptButton; 

    json configData;
    QTextBrowser* fileBrowser;

    void onEditDataInputSubmitted();
    void updateJson();
    static json restructureJson(const json& target);
    static std::string extractArgument(const std::string& container, const std::string& finder);
    static void extractArgumentList(const std::string& container, const std::string& finder, std::vector<std::string>& located);
};

#endif //MAIN_WIDGET_H
