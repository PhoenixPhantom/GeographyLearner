#include "MainWidget.h"
#include "MapWidget.h"
#include "json.hpp"

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

    map = new MapWidget(":/Data/WorldMap.png", QSize(10, 10), this);
    QSizePolicy sizePolicy;
    sizePolicy.setVerticalPolicy(QSizePolicy::Expanding);
    sizePolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    map->setSizePolicy(sizePolicy);

    taskDescription = new QLabel(this);
    taskDescription->setText("Drücke Enter, oder den \"Weiter\"-Knopf um zu beginnen...");

    answerLine = new QLineEdit(this);
    acceptButton = new QPushButton(tr("Weiter"));

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->addWidget(map, 0, 0, 1, 2);
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
    useCities->toggled(true);
    useSeaOceans = new QCheckBox(this);
    useSeaOceans->setText("Meere & Meeresteile");
    useSeaOceans->toggled(true);
    useIslandPeninsulas = new QCheckBox(this);
    useIslandPeninsulas->setText("Inseln & Halbinseln");
    useIslandPeninsulas->toggled(true);
    useMountainRanges = new QCheckBox(this);
    useMountainRanges->setText("Gebirge");
    useMountainRanges->toggled(true);
    useRivers = new QCheckBox(this);
    useRivers->setText("Flüsse");
    useRivers->toggled(true);
    useContinents = new QCheckBox(this);
    useContinents->setText("Kontinente");
    useContinents->toggled(true);

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

    mainLayout->addLayout(menuLayout, 0, 2, 1, 3);

    toggleType({"Continent", "City", "Sea", "Ocean", "River", "MountainRange", "Island", "Peninsula"}, true);

    connect(restartButton, SIGNAL(released()), this, SLOT(onRestart()));

    connect(useCities, SIGNAL(checkStateChanged(Qt::CheckState)), this, SLOT(onCitiesToggled()));
    connect(useSeaOceans, SIGNAL(checkStateChanged(Qt::CheckState)), this, SLOT(onSeaOceansToggled()));
    connect(useIslandPeninsulas, SIGNAL(checkStateChanged(Qt::CheckState state)), this, SLOT(onIslandPeninsulasToggled()));
    connect(useMountainRanges, SIGNAL(checkStateChanged(Qt::CheckState state)), this, SLOT(onMountainRangesToggled()));
    connect(useRivers, SIGNAL(checkStateChanged(Qt::CheckState state)), this, SLOT(onRiversToggled()));
    connect(useContinents, SIGNAL(checkStateChanged(Qt::CheckState state)), this, SLOT(onContinentsToggled()));
#endif

    setLayout(mainLayout);
    setWindowTitle(tr("Ein einfacher Geographie-Learner"));

    connect(answerLine, SIGNAL(returnPressed()), this, SLOT(onInputSubmitted()));
    
    connect(acceptButton, SIGNAL(released()), this, SLOT(onInputSubmitted()));
}

MainWidget::~MainWidget() {
    delete map;
    delete taskDescription;
    delete answerLine;
    delete acceptButton;
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
            if(&arrayElement == entry) return key;
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

    std::string input = answerLine->text().toStdString();
    std::vector<std::string> names = getCurrentQuestion()["Name"].template get<std::vector<std::string>>();
    if(std::find(names.begin(), names.end(), input) != names.end()){
        allDone.push_back(currentQuestion);
        allPending.erase(std::find(allPending.begin(), allPending.end(), currentQuestion));
        updateProgress();
        generateQuestion();
    }
    else{
       taskDescription->setText(QString::fromStdString("Falsch. Eine richtige Antwort wäre " + names[0]));
       currentQuestion = nullptr;
    }
}

void MainWidget::toggleType(const std::vector<std::string>& types, bool include)
{
    std::vector<json*> foundInstances;
    for(const std::string& type : types){
        std::vector<json*> newInstances = getAllOfType(configData, type);
        foundInstances.insert(foundInstances.end(), newInstances.begin(), newInstances.end());
    }
    
    for(json* instance : foundInstances){
        auto instanceIt = std::find(allUsed.begin(), allUsed.end(), instance);
        if(!include && instanceIt != allUsed.end()){
            allUsed.erase(instanceIt);
            allPending.erase(std::find(allPending.begin(), allPending.end(), instance));
        }
        if(include && instanceIt == allUsed.end()){
            allUsed.push_back(instance);
            allPending.push_back(instance);
        }
    }
    updateProgress();
}

void MainWidget::generateQuestion()
{
    size_t chosenIndex = double(rand()) / RAND_MAX * double(allPending.size());
    currentQuestion = allPending[chosenIndex];

    std::string type = getType(configData, currentQuestion);
    std::string name;
    if(type == "City"){
        name = "die Stadt, die";
    }
    else if(type == "Continent"){
        name = "der Kontinent, der";
    }
    else if(type == "Peninsula"){
        name = "die Halbinsel, die";
    }
    else if(type == "Island"){
        name = "die Insel, die";
    }
    else if(type == "Ocean"){
        name = "der Ozean, der";
    }
    else if(type == "Sea"){
        name = "der Meeresteil, der";
    }
    else if(type == "River"){
        name = "der Fluss, der";
    }
    else if(type == "MountainRange"){
        name = "das Gebirge, das";
    }

    taskDescription->setText(QString::fromStdString(std::string("Wie heisst ") + name + " sich an der markierten Position befindet?"));
    answerLine->setText("");
    std::vector<double> position = (*currentQuestion)["Position"].template get<std::vector<double>>();
    map->setSelectorPos(QPointF(position[0], position[1]));
}



void MainWidget::updateProgress()
{
    std::stringstream outText;
    outText << "Vortschritt: " << allUsed.size() - allPending.size() << " / " << allUsed.size();
    progressInfo->setText(QString::fromStdString(outText.str()));
}


#endif
