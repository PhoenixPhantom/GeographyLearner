#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H

#include <QWidget>
#include <QProcess>
#include "json.hpp"

//#define EDITDATA "Data/configData.json"

#ifndef EDITDATA
#include <QCheckBox>
#endif

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
#ifndef EDITDATA
    void onRestart();
    void onCitesToggled(){ toggleType({"City"}, useCities->isChecked()); }
    void onSeaOceansToggled(){ toggleType({"Sea", "Ocean"}, useSeaOceans->isChecked()); }
    void onIslandPeninsulasToggled(){ toggleType({"Island", "Peninsula"}, useIslandPeninsulas->isChecked()); }
    void onMountainRangesToggled(){ toggleType({"MountainRange"}, useMountainRanges->isChecked()); }
    void onRiversToggled(){ toggleType({"River"}, useRivers->isChecked()); }
    void onContinentsToggled(){ toggleType({"Continent"}, useContinents->isChecked()); }
#endif
private:
    MapWidget* map;
    QLabel* selector;
    QLabel* taskDescription;
    QLineEdit* answerLine;
    QPushButton* acceptButton; 

    json configData;
    static std::string getType(const json& information, json const* entry);
    static json& getNamed(json& target, const std::string& name, const std::string& type = "");
    static std::vector<json*> getAllOfType(json& target, const std::string& type);
#ifdef EDITDATA
    QTextBrowser* fileBrowser;

    void onEditDataInputSubmitted();
    void updateJson();
    static json restructureJson(const json& target);
    static std::string extractArgument(const std::string& container, const std::string& finder);
    static void extractArgumentList(const std::string& container, const std::string& finder, std::vector<std::string>& located);
#else
    QLabel* progressInfo;
    QPushButton* restartButton;
    QCheckBox* useCities;
    QCheckBox* useSeaOceans;
    QCheckBox* useIslandPeninsulas;
    QCheckBox* useMountainRanges;
    QCheckBox* useRivers;
    QCheckBox* useContinents;

    std::vector<json*> allUsed;
    std::vector<json*> allDone;
    std::vector<json*> allPending;
    json* currentQuestion;
    json& getCurrentQuestion(){ return *currentQuestion; }
    void validateInput();
    void toggleType(const std::vector<std::string>& types, bool include);
    void generateQuestion();
    void updateProgress();
#endif
};

#endif //MAIN_WIDGET_H
