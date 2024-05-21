#include "LearnerWidget.h"

#include <cstring>
#include <cstdlib>
#include <sstream>
#include <QtWidgets>
#include <QTextStream>
#include "../utils/jsonFormatUtils.h"
#include "../utils/MapWidget.h"

#include "qboxlayout.h"
#include <random>
#include <algorithm>

LearnerWidget::LearnerWidget(const std::filesystem::path& project, QWidget *parent) : 
    QWidget(parent), currentQuestion(nullptr)
{
    QGridLayout *mainLayout = new QGridLayout(this);

    QString associatedImage = QString::fromStdString(project.string()); 
    associatedImage.remove(".json");
    associatedImage.append(".png");
    map = new MapWidget(associatedImage, QSize(15, 15), false, this);
    mainLayout->addWidget(map, 0, 0, 1, 2, Qt::AlignCenter);

    taskDescription = new QLabel(this);
    taskDescription->setText("Drücke Enter, oder den \"Weiter\"-Knopf um zu beginnen...");
    mainLayout->addWidget(taskDescription, 1, 0, 1, 2);
 
    answerLine = new QLineEdit(this);
    connect(answerLine, SIGNAL(returnPressed()), this, SLOT(onInputSubmitted()));
    mainLayout->addWidget(answerLine, 2, 0);

    acceptButton = new QPushButton(tr("Weiter")); 
    connect(acceptButton, SIGNAL(released()), this, SLOT(onInputSubmitted()));
    mainLayout->addWidget(acceptButton, 2, 1);

    QFile configFile(QString::fromStdString(project.string()));
    configFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream textStream(&configFile);
    std::stringstream dataStream;
    dataStream << textStream.readAll().toStdString();
    dataStream >> configData;

    QVBoxLayout* menuLayout = new QVBoxLayout(this);
    progressInfo = new QLabel(this);
    menuLayout->addWidget(progressInfo, Qt::AlignTop);

    restartButton = new QPushButton(this);
    restartButton->setText("Neustarten");
    connect(restartButton, SIGNAL(released()), this, SLOT(onRestart()));
    menuLayout->addWidget(restartButton, Qt::AlignTop);

    if(jsonFormatUtils::containsQuestionRegarding(configData, jsonFormatUtils::Stadt)){
        useCities = new QCheckBox(this);
        useCities->setText("Städte");
        useCities->setChecked(true);
        connect(useCities, &QPushButton::clicked, this, &LearnerWidget::onCitiesToggled);
        menuLayout->addWidget(useCities, Qt::AlignBottom);
    } else useCities = nullptr;

    if(jsonFormatUtils::containsQuestionRegarding(configData, jsonFormatUtils::Staat) ||
            jsonFormatUtils::containsQuestionRegarding(configData, jsonFormatUtils::Bundesstaat)){
        useStates = new QCheckBox(this);
        useStates->setText("(Bundes-)Staaten");
        useStates->setChecked(true);
        connect(useStates, &QPushButton::clicked, this, &LearnerWidget::onStatesToggled);
        menuLayout->addWidget(useStates, Qt::AlignBottom);

    } else useStates = nullptr;

    
    if(jsonFormatUtils::containsQuestionRegarding(configData, jsonFormatUtils::Meeresteil) ||
            jsonFormatUtils::containsQuestionRegarding(configData, jsonFormatUtils::Ozean)){
        useSeaOceans = new QCheckBox(this);
        useSeaOceans->setText("Ozeane und Meeresteile");
        useSeaOceans->setChecked(true);
        connect(useSeaOceans, &QPushButton::clicked, this, &LearnerWidget::onSeaOceansToggled);
        menuLayout->addWidget(useSeaOceans, Qt::AlignBottom);
    } else useSeaOceans = nullptr;


    if(jsonFormatUtils::containsQuestionRegarding(configData, jsonFormatUtils::See)){
        useLakes = new QCheckBox(this);
        useLakes->setText("Seen");
        useLakes->setChecked(true);
        connect(useLakes, &QPushButton::clicked, this, &LearnerWidget::onLakesToggled);
        menuLayout->addWidget(useLakes, Qt::AlignBottom);

    } else useLakes = nullptr;
    
    if(jsonFormatUtils::containsQuestionRegarding(configData, jsonFormatUtils::Fluss)){
        useRivers = new QCheckBox(this);
        useRivers->setText("Flüsse");
        useRivers->setChecked(true);
        connect(useRivers, &QPushButton::clicked, this, &LearnerWidget::onRiversToggled);
        menuLayout->addWidget(useRivers, Qt::AlignBottom);

    } else useRivers = nullptr;

    if(jsonFormatUtils::containsQuestionRegarding(configData, jsonFormatUtils::Region)){
        useRegions = new QCheckBox(this);
        useRegions->setText("Regionen");
        useRegions->setChecked(true);
        connect(useRegions, &QPushButton::clicked, this, &LearnerWidget::onRegionsToggled);
        menuLayout->addWidget(useRegions, Qt::AlignBottom);

    } else useRegions = nullptr;

    if(jsonFormatUtils::containsQuestionRegarding(configData, jsonFormatUtils::Insel) ||
            jsonFormatUtils::containsQuestionRegarding(configData, jsonFormatUtils::Halbinsel)){
        useIslandPeninsulas = new QCheckBox(this);
        useIslandPeninsulas->setText("Inseln und Halbinseln");
        useIslandPeninsulas->setChecked(true);
        connect(useIslandPeninsulas, &QPushButton::clicked, this, &LearnerWidget::onIslandPeninsulasToggled);
        menuLayout->addWidget(useIslandPeninsulas, Qt::AlignBottom);

    } else useIslandPeninsulas = nullptr;

    if(jsonFormatUtils::containsQuestionRegarding(configData, jsonFormatUtils::Gebirge) ||
            jsonFormatUtils::containsQuestionRegarding(configData, jsonFormatUtils::Gebirgskette)){
        useMountainRangesMountains = new QCheckBox(this);
        useMountainRangesMountains->setText("Gebirge und Gebirgsketten");
        useMountainRangesMountains->setChecked(true);
        connect(useMountainRangesMountains, &QPushButton::clicked, this, &LearnerWidget::onMountainRangesToggled);
        menuLayout->addWidget(useMountainRangesMountains, Qt::AlignBottom);

    } else useMountainRangesMountains = nullptr;

    if(jsonFormatUtils::containsQuestionRegarding(configData, jsonFormatUtils::Kontinent)){
        useContinents = new QCheckBox(this);
        useContinents->setText("Kontinente");
        useContinents->setChecked(true);
        connect(useContinents, &QPushButton::clicked, this, &LearnerWidget::onContinentsToggled);
        menuLayout->addWidget(useContinents, Qt::AlignBottom);

    } else useContinents = nullptr;

    if(jsonFormatUtils::containsQuestionRegarding(configData, jsonFormatUtils::Fluss)){
        advancedRiverQuestions = new QCheckBox(this);
        advancedRiverQuestions->setText("Flüsse Advanced");
        advancedRiverQuestions->setChecked(false);
        connect(advancedRiverQuestions, &QPushButton::clicked, this, &LearnerWidget::onRiverAdvancedToggled);
        menuLayout->addWidget(advancedRiverQuestions, Qt::AlignBottom);

    } else advancedRiverQuestions = nullptr;

    if(jsonFormatUtils::containsQuestionRegarding(configData, jsonFormatUtils::Stadt)){
        advancedCityQuestions = new QCheckBox(this);
        advancedCityQuestions->setText("Städte Advanced");
        advancedCityQuestions->setChecked(false);
        connect(advancedCityQuestions, &QPushButton::clicked, this, &LearnerWidget::onCityAdvancedToggled);
        menuLayout->addWidget(advancedCityQuestions, Qt::AlignBottom);

    } else advancedCityQuestions = nullptr;

    if(jsonFormatUtils::containsQuestionRegarding(configData, jsonFormatUtils::Bundesstaat)){
        advancedStateQuestions = new QCheckBox(this);
        advancedStateQuestions->setText("Bundesstaaten Advanced");
        advancedStateQuestions->setChecked(false);
        connect(advancedStateQuestions, &QPushButton::clicked, this, &LearnerWidget::onStateAdvancedToggled);
        menuLayout->addWidget(advancedStateQuestions, Qt::AlignBottom);

    } else advancedStateQuestions = nullptr;

    toggleType({SUPPORTED_TYPES}, true);

    mainLayout->addLayout(menuLayout, 0, 2, 3, 1);
    setLayout(mainLayout);
    setWindowTitle(tr("Geography-Learner"));
}

LearnerWidget::~LearnerWidget() {
    for(json* temp : temporary){
        delete temp;
    }
    delete map;
    delete taskDescription;
    delete answerLine;
    delete acceptButton;
}


void LearnerWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

void LearnerWidget::onInputSubmitted()
{
    validateInput(); 
    answerLine->setText("");
}

void LearnerWidget::onRestart()
{
    allDone.clear();
    allPending = allUsed;
    updateProgress();
    generateQuestion();
}

void LearnerWidget::validateInput(){
    if(currentQuestion == nullptr) {
        generateQuestion();
        return;
    }
    if(allPending.empty()){
        map->setSelectorColor(QColor(0, 0, 0, 0)); //make invisible
        taskDescription->setText("Genug gelernt? Ansonsten den \"Neustarten\"-Knopf drücken");
        taskDescription->setStyleSheet("QLabel { color : green; }");
        return;
    }

    std::string input = answerLine->text().toStdString();
    if(std::find(temporary.begin(), temporary.end(), currentQuestion) != temporary.end()){
        uint8_t type = jsonFormatUtils::getSupportedTypesIndex(jsonFormatUtils::getType(configData, currentQuestion));
        if(type == jsonFormatUtils::Stadt || type == jsonFormatUtils::Bundesstaat){
            std::string state = getCurrentQuestion()["liegtInStaat"].template get<std::string>();
            if(state != input){
                if(!jsonFormatUtils::existsCoNamed(configData, input, state, jsonFormatUtils::Bundesstaat) &&
                        !jsonFormatUtils::existsCoNamed(configData, input, state, jsonFormatUtils::Staat)){
                    taskDescription->setText(QString::fromStdString("Falsch. Eine richtige Antwort wäre " + state));
                    taskDescription->setStyleSheet("QLabel { color : red; }");
                    currentQuestion = nullptr;
                }
            }
            if(currentQuestion != nullptr){
                allDone.push_back(currentQuestion);
                allPending.erase(std::find(allPending.begin(), allPending.end(), currentQuestion));
                updateProgress();
                generateQuestion();
            }
        }
        else if(type == jsonFormatUtils::Fluss){
            std::string flowsInto = getCurrentQuestion()["mündetIn"].template get<std::string>();
            if(flowsInto != input){
                if(!jsonFormatUtils::existsCoNamed(configData, input, flowsInto, jsonFormatUtils::Fluss) &&
                        !jsonFormatUtils::existsCoNamed(configData, input, flowsInto, jsonFormatUtils::See) &&
                        !jsonFormatUtils::existsCoNamed(configData, input, flowsInto, jsonFormatUtils::Meeresteil) &&
                        !jsonFormatUtils::existsCoNamed(configData, input, flowsInto, jsonFormatUtils::Ozean)){
                    taskDescription->setText(QString::fromStdString("Falsch. Eine richtige Antwort wäre " + flowsInto));
                    taskDescription->setStyleSheet("QLabel { color : red; }");
                    currentQuestion = nullptr;
                }
            }
            if(currentQuestion != nullptr){
                allDone.push_back(currentQuestion);
                allPending.erase(std::find(allPending.begin(), allPending.end(), currentQuestion));
                updateProgress();
                generateQuestion();
            }
        }
        else{
            qDebug() << "A nonsensical question was found...";
        }
    }
    else{
        std::vector<std::string> names = getCurrentQuestion()["Name"].template get<std::vector<std::string>>();
        if(std::find(names.begin(), names.end(), input) != names.end()){
            allDone.push_back(currentQuestion);
            allPending.erase(std::find(allPending.begin(), allPending.end(), currentQuestion));
            updateProgress();
            generateQuestion();
        }
        else{
            taskDescription->setText(QString::fromStdString("Falsch. Eine richtige Antwort wäre " + names[0]));
            taskDescription->setStyleSheet("QLabel { color : red; }");
            currentQuestion = nullptr;
        }
    }
}

void LearnerWidget::toggleType(std::vector<std::string> types, bool include)
{
    std::vector<json*> foundInstances;
    for(const std::string& type : types){
        std::vector<json*> newInstances = jsonFormatUtils::getAllOfType(configData, type);
        foundInstances.insert(foundInstances.end(), newInstances.begin(), newInstances.end());
    }
    
    for(json* instance : foundInstances){
        auto instanceIt = std::find(allUsed.begin(), allUsed.end(), instance);
        auto pendingIt = std::find(allPending.begin(), allPending.end(), instance);

        if(!include){
            if(instanceIt != allUsed.end()) allUsed.erase(instanceIt);
            if(pendingIt != allPending.end()) allPending.erase(pendingIt);
        }
        if(include){
            if((*instance)["istMetaelement"].template get<bool>()) continue;
            if(instanceIt == allUsed.end()) allUsed.push_back(instance);
            if(pendingIt == allPending.end()) allPending.push_back(instance);
        }
    }
    updateProgress();
    generateQuestion();
}

void LearnerWidget::toggleAdvancedType(const std::vector<std::string>& types, bool include)
{

    for(const std::string& type : types){
        std::vector<json*> newInstances = jsonFormatUtils::getAllOfType(configData, type);
        for(json* instance : newInstances){
            if(instance == nullptr || !(*instance)["istMetaelement"].template get<bool>() &&
                    (!instance->contains("liegtInStaat") && !instance->contains("mündetIn"))) continue;
            auto it = findAsTemporary(type, instance);
            json* found;
            if(include){
                if(it == temporary.end()){
                    found = new json(*instance);
                    temporary.push_back(found);
                }
                else found = *it;
                allUsed.push_back(found);
                if(std::find(allDone.begin(), allDone.end(), found) == allDone.end()) allPending.push_back(found);
            }
            else{
                if(it == temporary.end()) found = instance; //this should not be possible
                found = *it;
                allUsed.erase(std::find(allUsed.begin(), allUsed.end(), found));
                if(std::find(allDone.begin(), allDone.end(), found) == allDone.end())
                    allPending.erase(std::find(allPending.begin(), allPending.end(), found));

            }
        }
    }
    updateProgress();
    generateQuestion();

}


void LearnerWidget::generateQuestion()
{
    if(allPending.empty()){
        map->setSelectorColor(QColor(0, 0, 0, 0)); //make invisible
        taskDescription->setText("Genug gelernt? Ansonsten den \"Neustarten\"-Knopf drücken");
        taskDescription->setStyleSheet("QLabel { color : green; }");
        return;
    }

    std::uniform_int_distribution<size_t> Distribution(0, allPending.size() - 1);
    size_t chosenIndex = Distribution(RandomGenerator);
    currentQuestion = allPending[chosenIndex];

    uint8_t type = jsonFormatUtils::getSupportedTypesIndex(jsonFormatUtils::getType(configData, currentQuestion));
    if(std::find(temporary.begin(), temporary.end(), currentQuestion) != temporary.end()){
        taskDescription->setStyleSheet("QLabel { color : blue; }");
        answerLine->setText("");

        map->setSelectorColor(QColor(0, 0, 0, 0)); //make invisible
        if(type == jsonFormatUtils::Stadt || type == jsonFormatUtils::Bundesstaat){
            std::string staat = getCurrentQuestion()["liegtInStaat"].template get<std::string>();

            bool inBundesstaat = jsonFormatUtils::existsElementWithNameInTypeset(getCurrentQuestion(), staat,
                    {jsonFormatUtils::supportedTypes[jsonFormatUtils::Bundesstaat]});
            std::string name = "Staat";
            if(inBundesstaat) name = "Bundesstaat";
            taskDescription->setText(QString::fromStdString("In welchem " + name + " liegt " +
                        getCurrentQuestion()["Name"].template get<std::vector<std::string>>()[0] + "?"));   
        }
        else if(type == jsonFormatUtils::Fluss){
            std::string flowsInto = getCurrentQuestion()["mündetIn"].template get<std::string>();
            taskDescription->setText(QString::fromStdString("In welches Gewässer mündet der folgende Fluss: " +
                        getCurrentQuestion()["Name"].template get<std::vector<std::string>>()[0] + "?")); 
        }
        else{
            qDebug() << "A nonsensical question was found...";
        }
    }
    else{
        map->setSelectorColor(jsonFormatUtils::getAssociatedColor(type));
        taskDescription->setStyleSheet("QLabel { color : black; }");
        answerLine->setText("");


        taskDescription->setText(QString::fromStdString(std::string("Wie heisst ") + 
                    jsonFormatUtils::getWording(type) + " sich an der markierten Position befindet?"));
        std::vector<double> position = (*currentQuestion)["Position"].template get<std::vector<double>>();
        map->setSelectorPos(QPointF(position[0], position[1]));
    }
}



void LearnerWidget::updateProgress()
{
    std::stringstream outText;
    outText << "Fortschritt: " << allUsed.size() - allPending.size() << " / " << allUsed.size();
    progressInfo->setText(QString::fromStdString(outText.str()));
}

std::vector<json*>::const_iterator LearnerWidget::findAsTemporary(const std::string& type, const std::string& name) const
{
    for(auto it = temporary.begin(); it < temporary.end(); it++){
        json* temp = *it;
        if(type != jsonFormatUtils::getType(configData, temp)) continue;
        std::vector<std::string> names = (*temp)["Name"].template get<std::vector<std::string>>();
        if(std::find(names.begin(), names.end(), name) != names.end()) return it;
    }
    return temporary.end();
}

std::vector<json*>::const_iterator LearnerWidget::findAsTemporary(const std::string& type, json* data) const
{
    for(auto it = temporary.begin(); it < temporary.end(); it++){
        json* temp = *it;
        if(type != jsonFormatUtils::getType(configData, temp)) continue;
        if(*temp == *data) return it;
    }
    return temporary.end();
}
