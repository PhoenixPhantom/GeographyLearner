#ifndef LEARNER_WIDGET_H
#define LEARNER_WIDGET_H

#include <QWidget>
#include <QProcess>
#include "../utils/json.hpp"
using json = nlohmann::json;

#include <QCheckBox>
#include <random>
#include "pcg/include/pcg_random.hpp"


class QPushButton;
class QTextBrowser;
class QLabel;
class QTextLine;
class QLineEdit;
class MapWidget;
class QFile;

class LearnerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LearnerWidget(const std::filesystem::path& learningset, QWidget* parent = nullptr);
    ~LearnerWidget();

protected:
    void resizeEvent(QResizeEvent* event) override;
private slots:
    void onInputSubmitted();
    void onRestart();
    void onCitiesToggled(){ if(useCities == nullptr) return; toggleType({"Stadt"}, useCities->isChecked()); }
    void onStatesToggled(){ if(useStates == nullptr) return;  toggleType({"Bundesstaat", "Staat"}, useStates->isChecked()); }
    void onRegionsToggled(){ if(useRegions == nullptr) return;  toggleType({"Region"}, useRegions->isChecked()); }
    void onSeaOceansToggled(){ if(useSeaOceans == nullptr) return;  toggleType({"Meeresteil", "Ozean"}, useSeaOceans->isChecked()); }
    void onIslandPeninsulasToggled(){ if(useIslandPeninsulas == nullptr) return;  toggleType({"Insel", "Halbinsel"}, useIslandPeninsulas->isChecked()); }

    void onMountainRangesToggled(){ if(useMountainRangesMountains == nullptr) return;  toggleType({"Gebirge", "Gebirgskette"}, useMountainRangesMountains->isChecked()); }
    void onRiversToggled(){ if(useRivers == nullptr) return;  toggleType({"Fluss"}, useRivers->isChecked()); }
    void onLakesToggled(){ if(useLakes == nullptr) return;  toggleType({"See"}, useLakes->isChecked()); }
    void onContinentsToggled(){ if(useContinents == nullptr) return;  toggleType({"Kontinent"}, useContinents->isChecked()); }
    void onCityAdvancedToggled(){ if(advancedCityQuestions == nullptr) return;  toggleAdvancedType({"Stadt"}, advancedCityQuestions->isChecked()); }
    void onStateAdvancedToggled(){ if(advancedStateQuestions == nullptr) return;  toggleAdvancedType({"Bundesstaat"}, advancedStateQuestions->isChecked()); }
    void onRiverAdvancedToggled(){ if(advancedRiverQuestions == nullptr) return;  toggleAdvancedType({"Fluss"}, advancedRiverQuestions->isChecked()); }
private:
    MapWidget* map;
    QLabel* taskDescription;
    QLineEdit* answerLine;
    QPushButton* acceptButton; 

    json configData;
    QLabel* progressInfo;
    QPushButton* restartButton;
    QCheckBox* useCities;
    QCheckBox* useRegions;
    QCheckBox* useStates;
    QCheckBox* useSeaOceans;
    QCheckBox* useLakes;
    QCheckBox* useIslandPeninsulas;
    QCheckBox* useMountainRangesMountains;
    QCheckBox* useMountains;
    QCheckBox* useRivers;
    QCheckBox* useContinents;
    QCheckBox* advancedRiverQuestions;
    QCheckBox* advancedCityQuestions;
    QCheckBox* advancedStateQuestions;

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
};

#endif //MAIN_WIDGET_H
