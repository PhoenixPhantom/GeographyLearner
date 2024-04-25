#ifndef MAIN_WIDGET_H
#define MAIN_WIDGET_H

#include <QWidget>
#include <QProcess>
#include "json.hpp"

//#define EDITDATA "Data/configData.json"

#ifndef EDITDATA
#include <QCheckBox>
#include <random>
#include "pcg/include/pcg_random.hpp"
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

protected:
    void resizeEvent(QResizeEvent* event) override;
private slots:
    void onInputSubmitted();
#ifndef EDITDATA
    void onRestart();
    void onCitiesToggled(){ toggleType({"City"}, useCities->isChecked()); }
    void onSeaOceansToggled(){ toggleType({"Sea", "Ocean"}, useSeaOceans->isChecked()); }
    void onIslandPeninsulasToggled(){ toggleType({"Island", "Peninsula"}, useIslandPeninsulas->isChecked()); }
    void onMountainRangesToggled(){ toggleType({"MountainRange"}, useMountainRanges->isChecked()); }
    void onRiversToggled(){ toggleType({"River"}, useRivers->isChecked()); }
    void onContinentsToggled(){ toggleType({"Continent"}, useContinents->isChecked()); }
    void onCityAdvancedToggled(){ toggleAdvancedType({"City"}, advancedCityQuestions->isChecked()); }
    void onRiverAdvancedToggled(){ toggleAdvancedType({"River"}, advancedRiverQuestions->isChecked()); }
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
    static bool existsCoNamed(json& target, const std::string& name1, const std::string& name2,
            const std::string& type = "");
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
    QCheckBox* advancedRiverQuestions;
    QCheckBox* advancedCityQuestions;

	inline static pcg_extras::seed_seq_from<std::random_device> SeedSource;
	inline static pcg32 RandomGenerator = SeedSource;
    std::vector<json*> allUsed;
    std::vector<json*> allDone;
    std::vector<json*> allPending;

    std::vector<json*> temporary;
    
    json* currentQuestion;
    json& getCurrentQuestion(){ return *currentQuestion; }
    void validateInput();
    void toggleType(std::vector<std::string> types, bool include);
    void toggleAdvancedType(const std::vector<std::string>& types, bool include);
    void generateQuestion();
    void updateProgress();
    std::vector<json*>::const_iterator findAsTemporary(const std::string& type, const std::string& name) const;
    std::vector<json*>::const_iterator findAsTemporary(const std::string& type, json* data) const;
#endif
};

#endif //MAIN_WIDGET_H
