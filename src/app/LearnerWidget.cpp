#include "LearnerWidget.h"

#include <cstring>
#include <cstdlib>
#include <sstream>
#include <QtWidgets>
#include <QTextStream>
#include "../utils/jsonDataInterpreter.h"
#include "../utils/MapWidget.h"

#include "qboxlayout.h"
#include <random>
#include <algorithm>

LearnerWidget::LearnerWidget(QWidget *parent) : QWidget(parent)
    , currentQuestion(nullptr)
{

    map = new MapWidget(":/Data/WorldMap.png", QSize(15, 15), this);
// QSizePolicy sizePolicy;
//    sizePolicy.setVerticalPolicy(QSizePolicy::Maximum);
 //   sizePolicy.setHorizontalPolicy(QSizePolicy::Maximum);
  //  map->setSizePolicy(sizePolicy);
    taskDescription = new QLabel(this);
    taskDescription->setText("Drücke Enter, oder den \"Weiter\"-Knopf um zu beginnen...");
//    sizePolicy.setVerticalPolicy(QSizePolicy::Minimum);
 //   sizePolicy.setHorizontalPolicy(QSizePolicy::Minimum);
  //  taskDescription->setSizePolicy(sizePolicy);
 
    answerLine = new QLineEdit(this);
    acceptButton = new QPushButton(tr("Weiter"));

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->addWidget(map, 0, 0, 1, 2, Qt::AlignCenter);
    mainLayout->addWidget(taskDescription, 1, 0, 1, 2);

    mainLayout->addWidget(answerLine, 2, 0);
    mainLayout->addWidget(acceptButton, 2, 1);

    QFile configFile(":/Data/configData.json");
    configFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream textStream(&configFile);
    std::stringstream dataStream;
    dataStream << textStream.readAll().toStdString();
    dataStream >> configData;

    QVBoxLayout* menuLayout = new QVBoxLayout(this);
    useCities = new QCheckBox(this);
    useCities->setText("Städte");
    useCities->setChecked(true);
    useSeaOceans = new QCheckBox(this);
    useSeaOceans->setText("Meere & Meeresteile");
    useSeaOceans->setChecked(true);
    useIslandPeninsulas = new QCheckBox(this);
    useIslandPeninsulas->setText("Inseln & Halbinseln");
    useIslandPeninsulas->setChecked(true);
    useMountainRanges = new QCheckBox(this);
    useMountainRanges->setText("Gebirge");
    useMountainRanges->setChecked(true);
    useRivers = new QCheckBox(this);
    useRivers->setText("Flüsse");
    useRivers->setChecked(true);
    useContinents = new QCheckBox(this);
    useContinents->setText("Kontinente");
    useContinents->setChecked(true);
    advancedRiverQuestions = new QCheckBox(this);
    advancedRiverQuestions->setText("Flüsse Advanced");
    advancedRiverQuestions->setChecked(false);
    advancedCityQuestions = new QCheckBox(this);
    advancedCityQuestions->setText("Städte Advanced");
    advancedCityQuestions->setChecked(false);



    restartButton = new QPushButton(this);
    restartButton->setText("Neustarten");

    progressInfo = new QLabel(this);

    menuLayout->addWidget(progressInfo);
    menuLayout->addSpacing(300);
    menuLayout->addWidget(restartButton);
    menuLayout->addWidget(useCities);
    menuLayout->addWidget(useSeaOceans);
    menuLayout->addWidget(useIslandPeninsulas);
    menuLayout->addWidget(useMountainRanges);
    menuLayout->addWidget(useRivers);
    menuLayout->addWidget(useContinents);
    menuLayout->addWidget(advancedCityQuestions);
    menuLayout->addWidget(advancedRiverQuestions);

    mainLayout->addLayout(menuLayout, 0, 2, 3, 1);

    toggleType({"Continent", "City", "Sea", "Ocean", "River", "MountainRange", "Island", "Peninsula"}, true);

    connect(restartButton, SIGNAL(released()), this, SLOT(onRestart()));

    connect(useCities, SIGNAL(clicked(bool)), this, SLOT(onCitiesToggled()));
    connect(useSeaOceans, SIGNAL(clicked(bool)), this, SLOT(onSeaOceansToggled()));
    connect(useIslandPeninsulas, SIGNAL(clicked(bool)), this, SLOT(onIslandPeninsulasToggled()));
    connect(useMountainRanges, SIGNAL(clicked(bool)), this, SLOT(onMountainRangesToggled()));
    connect(useRivers, SIGNAL(clicked(bool)), this, SLOT(onRiversToggled()));
    connect(useContinents, SIGNAL(clicked(bool)), this, SLOT(onContinentsToggled()));
    connect(advancedCityQuestions, SIGNAL(clicked(bool)), this, SLOT(onCityAdvancedToggled()));
    connect(advancedRiverQuestions, SIGNAL(clicked(bool)), this, SLOT(onRiverAdvancedToggled()));

    setLayout(mainLayout);
    setWindowTitle(tr("Geography-Learner"));

    connect(answerLine, SIGNAL(returnPressed()), this, SLOT(onInputSubmitted()));
    
    connect(acceptButton, SIGNAL(released()), this, SLOT(onInputSubmitted()));
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
        std::string type = jsonDataInterpreter::getType(configData, currentQuestion);
        if(type == "City"){
            std::vector<std::string> states = getCurrentQuestion()["State"].template get<std::vector<std::string>>();
            if(states[0] != input){
                if(!jsonDataInterpreter::existsCoNamed(configData, input, states[0], "Nation")){
                    taskDescription->setText(QString::fromStdString("Falsch. Eine richtige Antwort wäre " + states[0]));
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
        else if(type == "River"){
            std::string flowsInto = getCurrentQuestion()["FlowsInto"].template get<std::string>();
            if(flowsInto != input){
                if(!jsonDataInterpreter::existsCoNamed(configData, input, flowsInto, "River") &&
                        !jsonDataInterpreter::existsCoNamed(configData, input, flowsInto, "Ocean") &&
                        !jsonDataInterpreter::existsCoNamed(configData, input, flowsInto, "Sea")){
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
    if(std::find(types.begin(), types.end(), "Island") != types.end()) types.push_back("IslandRegion");
    for(const std::string& type : types){
        std::vector<json*> newInstances = jsonDataInterpreter::getAllOfType(configData, type);
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
        std::vector<json*> newInstances = jsonDataInterpreter::getAllOfType(configData, type);
        for(json* instance : newInstances){
            if(instance == nullptr || (!instance->contains("State") && !instance->contains("FlowsInto"))) continue;
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
 
    std::string type = jsonDataInterpreter::getType(configData, currentQuestion);
    if(std::find(temporary.begin(), temporary.end(), currentQuestion) != temporary.end()){
        taskDescription->setStyleSheet("QLabel { color : blue; }");
        answerLine->setText("");

        map->setSelectorColor(QColor(0, 0, 0, 0)); //make invisible
        if(type == "City"){
            std::vector<std::string> states = getCurrentQuestion()["State"].template get<std::vector<std::string>>();
            std::string name = "Staat";
            if(states.size() > 1) name = "Bundesstaat";
            taskDescription->setText(QString::fromStdString("In welchem " + name + " liegt " +
                        getCurrentQuestion()["Name"].template get<std::vector<std::string>>()[0] + "?"));   
        }
        else if(type == "River"){
            std::string flowsInto = getCurrentQuestion()["FlowsInto"].template get<std::string>();
            taskDescription->setText(QString::fromStdString("In welchen Fluss/welches Meer fliesst der folgende Fluss: " +
                        getCurrentQuestion()["Name"].template get<std::vector<std::string>>()[0] + "?")); 
        }
        else{
            qDebug() << "A nonsensical question was found...";
        }
    }
    else{
        std::string name;
        if(type == "City"){
            name = "die Stadt, die";
            map->setSelectorColor(QColor(255,20,147));
        }
        else if(type == "Continent"){
            name = "der Kontinent, der";
            map->setSelectorColor(QColor(255,215,0));
        }
        else if(type == "Peninsula"){
            name = "die Halbinsel, die";
            map->setSelectorColor(QColor(102,205,170));
        }
        else if(type == "Island"){
            name = "die Insel, die";
            map->setSelectorColor(QColor(138,43,226));
        }
        else if(type == "IslandRegion"){
            name = "die Inselregion, die";
            map->setSelectorColor(QColor(138,43,226));
        }
        else if(type == "Ocean"){
            name = "der Ozean, der";
            map->setSelectorColor(QColor(0,0,205));
        }
        else if(type == "Sea"){
            name = "der Meeresteil, der";
            map->setSelectorColor(QColor(30,144,255));
        }
        else if(type == "River"){
            name = "der Fluss, der";
            map->setSelectorColor(QColor(0,191,255));
        }
        else if(type == "MountainRange"){
            name = "das Gebirge, das";
            map->setSelectorColor(QColor(255,140,0));
        }
        taskDescription->setStyleSheet("QLabel { color : black; }");
        answerLine->setText("");


        taskDescription->setText(QString::fromStdString(std::string("Wie heisst ") + name + " sich an der markierten Position befindet?"));
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
        if(type != jsonDataInterpreter::getType(configData, temp)) continue;
        std::vector<std::string> names = (*temp)["Name"].template get<std::vector<std::string>>();
        if(std::find(names.begin(), names.end(), name) != names.end()) return it;
    }
    return temporary.end();
}

std::vector<json*>::const_iterator LearnerWidget::findAsTemporary(const std::string& type, json* data) const
{
    for(auto it = temporary.begin(); it < temporary.end(); it++){
        json* temp = *it;
        if(type != jsonDataInterpreter::getType(configData, temp)) continue;
        if(*temp == *data) return it;
    }
    return temporary.end();
}
