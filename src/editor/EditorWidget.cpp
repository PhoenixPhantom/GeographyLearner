#include "EditorWidget.h"

#include <cstring>
#include <cstdlib>
#include <sstream>
#include <QtWidgets>
#include <QTextStream>
#include <fstream>
#include "../utils/jsonDataInterpreter.h"
#include "../utils/MapWidget.h"



EditorWidget::EditorWidget(QWidget *parent) : QWidget(parent)
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

    //NOTE: this usage of the path assumes the executable is located at the same location that the Data folder is
    map->setCanMoveSelector(true);
    std::ifstream inputStream("../../Data/configData.json");
    inputStream >> configData;
    fileBrowser = new QTextBrowser();
    std::stringstream str;
    str << std::setw(4) << configData;
    fileBrowser->setText(QString::fromStdString(str.str()));
    mainLayout->addWidget(fileBrowser, 0, 2, 1, 3);
    fileBrowser->setMinimumSize(QSize(200, 400));

    setLayout(mainLayout);
    setWindowTitle(tr("Geography-Learner"));

    connect(answerLine, SIGNAL(returnPressed()), this, SLOT(onInputSubmitted()));
    
    connect(acceptButton, SIGNAL(released()), this, SLOT(onInputSubmitted()));
}

EditorWidget::~EditorWidget() {
    delete map;
    delete taskDescription;
    delete answerLine;
    delete acceptButton;
}


void EditorWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

void EditorWidget::onInputSubmitted()
{
   onEditDataInputSubmitted(); 
}

void EditorWidget::onEditDataInputSubmitted()
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
    json& objectConfig = jsonDataInterpreter::getNamed(configData, objectName, type);

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

void EditorWidget::updateJson(){
    std::ofstream outStream("../../Data/configData.json");
    outStream << std::setw(4) << configData << std::endl; 
    answerLine->setText("");

    std::stringstream str;
    str << std::setw(4) << configData;
    fileBrowser->setText(QString::fromStdString(str.str()));
}

json EditorWidget::restructureJson(const json& target)
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

std::string EditorWidget::extractArgument(const std::string& container, const std::string& finder)
{
    if(container.empty()) return "";

    size_t start = container.find(finder);
    if(start == std::string::npos) return "";
    start += finder.length();
    size_t end = container.find(";", start);
    if(end == std::string::npos) end = container.length();
    return container.substr(start, end - start);
}

void EditorWidget::extractArgumentList(const std::string& container, const std::string& finder, std::vector<std::string>& located)
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
