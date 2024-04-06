#include "MainWidget.h"
#include "MapWidget.h"
#include "json.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <QtWidgets>

MainWidget::MainWidget(QWidget *parent) : QWidget(parent) {

    map = new MapWidget(":/Data/BlankMap-World-noborders.png",
            QSize(10, 10), this);
    QSizePolicy sizePolicy;
    sizePolicy.setVerticalPolicy(QSizePolicy::Expanding);
    sizePolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    map->setSizePolicy(sizePolicy);

    taskDescription = new QLabel(this);
    taskDescription->setText("Answer the following question: ");

    answerLine = new QLineEdit(this);
    acceptButton = new QPushButton(tr("Push this button"));

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->addWidget(map, 0, 0, 1, 2);
    mainLayout->addWidget(taskDescription, 1, 0, 1, 2);

    mainLayout->addWidget(answerLine, 2, 0);
    mainLayout->addWidget(acceptButton, 2, 1);

#ifdef EDITDATA
    //NOTE: this usage of the path assumes the executable is located at the same location that the Data folder is
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
    configFile >> configData;
#endif

    setLayout(mainLayout);
    setWindowTitle(tr("A simple geography learner"));

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
    taskDescription->setText(taskDescription->text() + answerLine->text() + " ?");
    answerLine->setText("");
#endif
}


#ifdef EDITDATA
void MainWidget::onEditDataInputSubmitted()
{
    std::string input = answerLine->text().toStdString();
    if(input.length() == 0) return;

    size_t firstArgEnd = input.find(";");
    if(firstArgEnd == std::string::npos) firstArgEnd = input.length() - 1;
    std::string objectName = input.substr(0, firstArgEnd);
    std::string additionalInformation = input.substr(firstArgEnd, input.length() - firstArgEnd);

    if(!configData.contains(objectName)){
        //if the name doesn't exist, check if an alias was used in place of the true object name
        for(auto& [key, value] : configData.items()){
            std::vector<std::string> aliases = value["Alias"].template get<std::vector<std::string>>();
            if(std::find(aliases.begin(), aliases.end(), objectName) == aliases.end()) continue;
            objectName = key;
        }
    }

    if(additionalInformation.find("Delete") == std::string::npos){
        std::vector<std::string> stateArguments;
        extractArgumentList(additionalInformation, "State=", stateArguments); 
        
        std::vector<std::string> aliasArguments;
        extractArgumentList(additionalInformation, "Alias=", aliasArguments); 

       if(!configData.contains(objectName)) configData[objectName] = json::object();  

       json& objectConfig = configData[objectName];

       if(additionalInformation.find("Position") != std::string::npos)
           objectConfig["InternalCoordinates"] = {map->getSelectorPos().x(), map->getSelectorPos().y()};
       if(stateArguments.size() > 0)
           objectConfig["State"] = stateArguments;
       if(aliasArguments.size() > 0)
           objectConfig["Alias"] = aliasArguments;

       std::string extractedArgument = extractArgument(additionalInformation, "Type=");
       if(extractedArgument.length() > 0){
           if(extractedArgument.find("Delete") != std::string::npos) objectConfig.erase("Type");
           else objectConfig["Type"] = extractedArgument;
       }
       extractedArgument = extractArgument(additionalInformation, "FlowsInto=");
       if(extractedArgument.length() > 0){
           if(extractedArgument.find("Delete") != std::string::npos) objectConfig.erase("FlowsInto");
           else objectConfig["FlowsInto"] = extractedArgument;
       }
    }
    else {
        configData.erase(objectName);
    }

    std::ofstream outStream(EDITDATA);
    outStream << std::setw(4) << configData << std::endl; 
    answerLine->setText("");

    std::stringstream str;
    str << std::setw(4) << configData;
    fileBrowser->setText(QString::fromStdString(str.str()));
}

std::string MainWidget::extractArgument(const std::string& container, const std::string& finder)
{
    if(container.length() <= 0) return "";

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
    if(total.length() <= 0) return;
    size_t start = 0;
    size_t end = 0;

    while ((start = total.find_first_not_of(", ", end)) != std::string::npos) {
        end = total.find(",", start);
        located.push_back(total.substr(start, end - start));
    }
}
#endif
