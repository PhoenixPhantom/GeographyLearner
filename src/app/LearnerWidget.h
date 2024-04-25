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
    explicit LearnerWidget(QWidget* parent = nullptr);
    ~LearnerWidget();

protected:
    void resizeEvent(QResizeEvent* event) override;
private slots:
    void onInputSubmitted();
    void onRestart();
    void onCitiesToggled(){ toggleType({"City"}, useCities->isChecked()); }
    void onSeaOceansToggled(){ toggleType({"Sea", "Ocean"}, useSeaOceans->isChecked()); }
    void onIslandPeninsulasToggled(){ toggleType({"Island", "Peninsula"}, useIslandPeninsulas->isChecked()); }
    void onMountainRangesToggled(){ toggleType({"MountainRange"}, useMountainRanges->isChecked()); }
    void onRiversToggled(){ toggleType({"River"}, useRivers->isChecked()); }
    void onContinentsToggled(){ toggleType({"Continent"}, useContinents->isChecked()); }
    void onCityAdvancedToggled(){ toggleAdvancedType({"City"}, advancedCityQuestions->isChecked()); }
    void onRiverAdvancedToggled(){ toggleAdvancedType({"River"}, advancedRiverQuestions->isChecked()); }
private:
    MapWidget* map;
    QLabel* selector;
    QLabel* taskDescription;
    QLineEdit* answerLine;
    QPushButton* acceptButton; 

    json configData;
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
};

#endif //MAIN_WIDGET_H
