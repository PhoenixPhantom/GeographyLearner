#include "MainWidget.h"
#include "MapWidget.h"
#include "json.hpp"
#include "qlogging.h"
#include "qnamespace.h"
#include <cstring>
#include <random>

#ifdef EDITDATA
#include <fstream>
#else
#include "qboxlayout.h"
#endif

#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <QtWidgets>
#include <QTextStream>

MainWidget::MainWidget(QWidget *parent) : QWidget(parent)
#ifndef EDITDATA
    , currentQuestion(nullptr)
#endif
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

#ifdef EDITDATA
    //NOTE: this usage of the path assumes the executable is located at the same location that the Data folder is
    map->setCanMoveSelector(true);
    std::ifstream inputStream(EDITDATA);
    inputStream >> configData;
    fileBrowser = new QTextBrowser();
    std::stringstream str;
    str << std::setw(4) << configData;
    fileBrowser->setText(QString::fromStdString(str.str()));
    mainLayout->addWidget(fileBrowser, 0, 2, 1, 3);
    fileBrowser->setMinimumSize(QSize(200, 400));
#else
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
#endif

    setLayout(mainLayout);
    setWindowTitle(tr("Geography-Learner"));

    connect(answerLine, SIGNAL(returnPressed()), this, SLOT(onInputSubmitted()));
    
    connect(acceptButton, SIGNAL(released()), this, SLOT(onInputSubmitted()));
}

MainWidget::~MainWidget() {
    for(json* temp : temporary){
        delete temp;
    }
    delete map;
    delete taskDescription;
    delete answerLine;
    delete acceptButton;
}


void MainWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

void MainWidget::onInputSubmitted()
{
#ifdef EDITDATA
   onEditDataInputSubmitted(); 
#else
    validateInput(); 
    answerLine->setText("");
#endif
}

std::string MainWidget::getType(const json& information, json const* entry)
{
    for(auto& [key, value] : information.items()){
        if(!value.is_array()){
            qDebug() << key << "doesn't specify a value. Consider running \"restructureJson\" first";
            continue;
        }

        for(auto& arrayElement : value){
            if(arrayElement == *entry) return key;
        }
    }
    return "";
}


json& MainWidget::getNamed(json& target, const std::string& name, const std::string& type){
    for(auto& [key, value] : target.items()){
        if(!type.empty() && key != type) continue;
        if(!value.is_array()){
            qDebug() << key << "doesn't specify a value. Consider running \"restructureJson\" first";
            continue;
        }
        
        for(auto& element : value){
            if(!element.contains("Name")) continue;
            std::vector<std::string> aliases = element["Name"].template get<std::vector<std::string>>();
            if(std::find(aliases.begin(), aliases.end(), name) != aliases.end()) return element;
        }
    }

    json& targetObject = target[type];
    if(!target.contains(type)) targetObject = json::array();
    json newObject = json::object();
    newObject["Name"] = json::array();
    newObject["Name"].push_back(name);
    targetObject.push_back(newObject);
    return targetObject.at(targetObject.size() - 1);
}


bool MainWidget::existsCoNamed(json& target, const std::string& name1, const std::string& name2,
        const std::string& type){
    for(auto& [key, value] : target.items()){
        if(!type.empty() && key != type) continue;
        if(!value.is_array()){
            qDebug() << key << "doesn't specify a value. Consider running \"restructureJson\" first";
            continue;
        }
        
        for(auto& element : value){
            if(!element.contains("Name")) continue;
            std::vector<std::string> aliases = element["Name"].template get<std::vector<std::string>>();
            if(std::find(aliases.begin(), aliases.end(), name1) != aliases.end() &&
                    std::find(aliases.begin(), aliases.end(), name2) != aliases.end()) return true;
        }
    }
    return false;
}

std::vector<json*> MainWidget::getAllOfType(json& target, const std::string& type)
{
    std::vector<json*> foundInstances;
    for(auto& [key, value] : target.items()){
        if(key != type) continue;

        if(!value.is_array()){
            qDebug() << key << "doesn't specify a value. Consider running \"restructureJson\" first";
            continue;
        }
        for(auto& arrayElement : value){
            foundInstances.push_back(&arrayElement);
        }
    }
    return foundInstances;
}


#ifdef EDITDATA
void MainWidget::onEditDataInputSubmitted()
{
    std::string input = answerLine->text().toStdString();
    if(input.empty()) return;

    if(input.find("RestructureJson") != std::string::npos){
        configData = restructureJson(configData);
        updateJson();
        return;
    }

    size_t firstArgEnd = input.find(";");
    if(firstArgEnd == std::string::npos) firstArgEnd = input.length() - 1;
    std::string objectName = input.substr(0, firstArgEnd);
    std::string additionalInformation = input.substr(firstArgEnd, input.length() - firstArgEnd);


    std::string type = extractArgument(additionalInformation, "Type=");
    json& objectConfig = getNamed(configData, objectName, type);

    if(additionalInformation.find("Delete") > firstArgEnd){
        std::vector<std::string> stateArguments;
        extractArgumentList(additionalInformation, "State=", stateArguments); 
        
        std::vector<std::string> aliasArguments;
        extractArgumentList(additionalInformation, "Alias=", aliasArguments); 

       if(additionalInformation.find("Position") != std::string::npos)
           objectConfig["Position"] = {map->getSelectorPos().x(), map->getSelectorPos().y()};
       if(stateArguments.size() > 0)
           objectConfig["State"] = stateArguments;
       if(aliasArguments.size() > 0)
           objectConfig["Name"] = aliasArguments;
 
       std::string extractedArgument = extractArgument(additionalInformation, "FlowsInto=");
       if(extractedArgument.length() > 0){
           if(extractedArgument.find("Delete") != std::string::npos) objectConfig.erase("FlowsInto");
           else objectConfig["FlowsInto"] = extractedArgument;
       }
    }
    else {
        for(auto& data : configData[type].items()){
            if(data.key() != objectName &&
                    (!data.value().contains("Alias") || data.value()["Alias"] != objectName)) continue;
            if(data.value().is_array()) continue;
            configData[type].erase(data.key());
            break;
        }
    }


    updateJson();
}

void MainWidget::updateJson(){
    std::ofstream outStream(EDITDATA);
    outStream << std::setw(4) << configData << std::endl; 
    answerLine->setText("");

    std::stringstream str;
    str << std::setw(4) << configData;
    fileBrowser->setText(QString::fromStdString(str.str()));
}

json MainWidget::restructureJson(const json& target)
{
    json restructuredData = json::object();
    for(const auto& [key, value] : target.items()){ 
        if(!value.contains("Type")){
            qDebug() << key << "doesn't contain a type";
            continue;
        }

        std::string type = value["Type"].template get<std::string>();
        //ensure the type exists and is an array
        if(!restructuredData.contains(type)){
            restructuredData[type] = json::array();
        }

        json dataToAdd = json::object();
        if(value.contains("InternalCoordinates")){
            dataToAdd["Position"] = value["InternalCoordinates"];
        }
        else qDebug() << key << "has no associated position";

        if(value.contains("State")) dataToAdd["State"] = value["State"];
        std::vector<std::string> names = {key};
        if(value.contains("Alias")){
            std::vector<std::string> aliases = value["Alias"].template get<std::vector<std::string>>();
            names.insert(names.end(), aliases.begin(), aliases.end());
        }
        dataToAdd["Name"] = names;
        if(value.contains("FlowsInto")) dataToAdd["FlowsInto"] = value["FlowsInto"];

        restructuredData[type].push_back(dataToAdd);
    }
    return restructuredData;
}

std::string MainWidget::extractArgument(const std::string& container, const std::string& finder)
{
    if(container.empty()) return "";

    size_t start = container.find(finder);
    if(start == std::string::npos) return "";
    start += finder.length();
    size_t end = container.find(";", start);
    if(end == std::string::npos) end = container.length();
    return container.substr(start, end - start);
}

void MainWidget::extractArgumentList(const std::string& container, const std::string& finder, std::vector<std::string>& located)
{

    std::string total = extractArgument(container, finder);
    if(total.empty()) return;
    size_t start = 0;
    size_t end = 0;

    while ((start = total.find_first_not_of(", ", end)) != std::string::npos) {
        end = total.find(",", start);
        located.push_back(total.substr(start, end - start));
    }
}
#else
void MainWidget::onRestart()
{
    allDone.clear();
    allPending = allUsed;
    updateProgress();
    generateQuestion();
}

void MainWidget::validateInput(){
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
        std::string type = getType(configData, currentQuestion);
        if(type == "City"){
            std::vector<std::string> states = getCurrentQuestion()["State"].template get<std::vector<std::string>>();
            if(states[0] != input){
                if(!existsCoNamed(configData, input, states[0], "Nation")){
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
                if(!existsCoNamed(configData, input, flowsInto, "River") &&
                        !existsCoNamed(configData, input, flowsInto, "Ocean") &&
                        !existsCoNamed(configData, input, flowsInto, "Sea")){
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

void MainWidget::toggleType(std::vector<std::string> types, bool include)
{
    std::vector<json*> foundInstances;
    if(std::find(types.begin(), types.end(), "Island") != types.end()) types.push_back("IslandRegion");
    for(const std::string& type : types){
        std::vector<json*> newInstances = getAllOfType(configData, type);
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

void MainWidget::toggleAdvancedType(const std::vector<std::string>& types, bool include)
{

    for(const std::string& type : types){
        std::vector<json*> newInstances = getAllOfType(configData, type);
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


void MainWidget::generateQuestion()
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
 
    std::string type = getType(configData, currentQuestion);
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



void MainWidget::updateProgress()
{
    std::stringstream outText;
    outText << "Fortschritt: " << allUsed.size() - allPending.size() << " / " << allUsed.size();
    progressInfo->setText(QString::fromStdString(outText.str()));
}

std::vector<json*>::const_iterator MainWidget::findAsTemporary(const std::string& type, const std::string& name) const
{
    for(auto it = temporary.begin(); it < temporary.end(); it++){
        json* temp = *it;
        if(type != getType(configData, temp)) continue;
        std::vector<std::string> names = (*temp)["Name"].template get<std::vector<std::string>>();
        if(std::find(names.begin(), names.end(), name) != names.end()) return it;
    }
    return temporary.end();
}

std::vector<json*>::const_iterator MainWidget::findAsTemporary(const std::string& type, json* data) const
{
    for(auto it = temporary.begin(); it < temporary.end(); it++){
        json* temp = *it;
        if(type != getType(configData, temp)) continue;
        if(*temp == *data) return it;
    }
    return temporary.end();
}
#endif
