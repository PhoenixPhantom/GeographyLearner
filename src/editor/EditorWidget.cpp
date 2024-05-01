#include "EditorWidget.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <qboxlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qlistwidget.h>
#include <qnamespace.h>
#include <qpushbutton.h>
#include <sstream>
#include <QtWidgets>
#include <fstream>
#include <vector>
#include "../utils/jsonFormatUtils.h"
#include "../utils/MapWidget.h"
#define deleteSafe(ptr) delete ptr; ptr = nullptr;

JsonBoundListItem::JsonBoundListItem(const elementRef& entry) : QListWidgetItem(), associatedEntry(entry){

}

JsonBoundListItem::JsonBoundListItem(const elementRef& entry, QListWidget *listview, int type) :
    QListWidgetItem(listview, type), associatedEntry(entry){

}

JsonBoundListItem::JsonBoundListItem(const elementRef& entry, const QString &text, QListWidget *listview, int type) :
    QListWidgetItem(text, listview, type), associatedEntry(entry){

}

JsonBoundListItem::JsonBoundListItem(const elementRef& entry, const QIcon &icon, const QString &text,
        QListWidget *listview, int type) : QListWidgetItem(icon, text, listview, type),
        associatedEntry(entry){

}


EditorWidget::EditorWidget(QWidget *parent) : QWidget(parent), currentListItem(nullptr)
{
    map = new MapWidget(":/Data/WorldMap.png", QSize(15, 15), false, this);
    map->setCanMoveSelector(true);
    QSizePolicy sizePolicy;
    sizePolicy.setVerticalPolicy(QSizePolicy::Expanding);
    sizePolicy.setHorizontalPolicy(QSizePolicy::Expanding);
    map->setSizePolicy(sizePolicy);

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->addWidget(map, 0, 0, 1, 2, Qt::AlignCenter);

    QVBoxLayout* menuLayout = new QVBoxLayout(this);

    //Search related things
    elementsSearch = new QGroupBox(tr("Element-Suche"), this);
    QVBoxLayout* elementsSearchLayout = new QVBoxLayout(elementsSearch);
    QHBoxLayout* searchBarLayout = new QHBoxLayout(elementsSearch);

    searchLine = new QLineEdit(elementsSearch);
    connect(searchLine, &QLineEdit::returnPressed, this, &EditorWidget::onSearch);
    searchBarLayout->addWidget(searchLine);

    typeSearchFilter = new QComboBox(elementsSearch);
    typeSearchFilter->setEditable(false);
    typeSearchFilter->addItems({"Kein Filter", SUPPORTED_TYPES}); 
    connect(typeSearchFilter, &QComboBox::activated, this, &EditorWidget::onSearch);
    searchBarLayout->addWidget(typeSearchFilter);
    elementsSearchLayout->addLayout(searchBarLayout);

    matchingElements = new QListWidget(elementsSearch);
    matchingElements->focusWidget();
    connect(matchingElements, &QListWidget::itemSelectionChanged, this, &EditorWidget::onSelectedElementChanged);
    elementsSearchLayout->addWidget(matchingElements);

    elementsSearch->setLayout(elementsSearchLayout);
    menuLayout->addWidget(elementsSearch);


    addNewElementButton = new QPushButton(tr("Neues Element einfügen"), elementsSearch);
    connect(addNewElementButton, &QPushButton::released, this, &EditorWidget::onAddNewElement);
    menuLayout->addWidget(addNewElementButton);


    //edit properties related things
    elementSettings = new QGroupBox(tr("Element-Eigenschaften"), this); 
    QGridLayout* elementSettingsLayout = new QGridLayout(elementSettings);

    elementNames = new QLineEdit(elementSettings);
    connect(elementNames, &QLineEdit::editingFinished, this, &EditorWidget::onElementNamesEdited);
    elementSettingsLayout->addWidget(new QLabel(tr("Name(n)"), elementSettings), 0, 0);
    elementSettingsLayout->addWidget(elementNames, 0, 1);
    
    elementType = new QComboBox(elementSettings);
    elementType->setEditable(false);
    elementType->addItems({SUPPORTED_TYPES});
    elementSettingsLayout->addWidget(new QLabel(tr("Typ"), elementSettings), 1, 0);
    connect(elementType, &QComboBox::activated, this, &EditorWidget::onElementTypeChanged);
    elementSettingsLayout->addWidget(elementType, 1, 1);

    elementPosContainer = new QGroupBox(tr("Lage"), this);
    QHBoxLayout* posContainerLayout = new QHBoxLayout(elementPosContainer);
    setElementSelectorPos = new QPushButton(elementPosContainer); 
    setElementSelectorPos->setText(tr("Markierung von Karte übernehmen"));
    connect(setElementSelectorPos, &QPushButton::released, this, &EditorWidget::onSavePosition);
    posContainerLayout->addWidget(setElementSelectorPos);
/*    getElementSelectorPos = new QPushButton(elementPosContainer);
    getElementSelectorPos->setText(tr("Anzeigen"));
    connect(getElementSelectorPos, &QPushButton::released, this, &EditorWidget::showPosition);
    posContainerLayout->addWidget(getElementSelectorPos);*/
    elementPosContainer->setLayout(posContainerLayout);
    elementSettingsLayout->addWidget(elementPosContainer, 2, 0, 1, 2);

    stateGroup = new QGroupBox(tr("(Bundes)staat"), elementSettings);
    QVBoxLayout* stateGroupLayout = new QVBoxLayout(stateGroup);
    elementState = new QLineEdit(stateGroup);
    connect(elementState, &QLineEdit::editingFinished, this, &EditorWidget::onElementStateEdited);
    stateGroupLayout->addWidget(elementState);
    searchState = new QPushButton(tr("Zum Staat"), stateGroup);
    connect(searchState, &QPushButton::released, this, &EditorWidget::onSearchState);
    stateGroupLayout->addWidget(searchState);
    stateGroup->setLayout(stateGroupLayout);
    elementSettingsLayout->addWidget(stateGroup, 4, 0, 1, 2);


    flowsIntoGroup = new QGroupBox(tr("Mündet in"), elementSettings);
    QVBoxLayout* flowsIntoGroupLayout = new QVBoxLayout(flowsIntoGroup);
    elementFlowsInto = new QLineEdit(stateGroup);
    connect(elementFlowsInto, &QLineEdit::editingFinished, this, &EditorWidget::onElementFlowsIntoEdited);
    flowsIntoGroupLayout->addWidget(elementFlowsInto);
    searchFlowsInto = new QPushButton(tr("Zum Gewässer"), flowsIntoGroup);
    connect(searchFlowsInto, &QPushButton::released, this, &EditorWidget::onSearchFlowsInto);
    flowsIntoGroupLayout->addWidget(searchFlowsInto);
    flowsIntoGroup->setLayout(flowsIntoGroupLayout);
    elementSettingsLayout->addWidget(flowsIntoGroup, 5, 0, 1, 2);


    elementIsMetadata = new QCheckBox(elementSettings);
    connect(elementIsMetadata, &QCheckBox::clicked, this, &EditorWidget::onElementIsMetadataChanged);
    elementSettingsLayout->addWidget(new QLabel(tr("Metaelement"), elementSettings), 6, 0);
    elementSettingsLayout->addWidget(elementIsMetadata, 6, 1, Qt::AlignRight);

    elementDelete = new QPushButton(tr("Eintrag löschen"), elementSettings);
    connect(elementDelete, &QPushButton::released, this, &EditorWidget::onElementDeleted);
    elementSettingsLayout->addWidget(elementDelete, 7, 0, 1, 2, Qt::AlignCenter);

    elementSettings->setLayout(elementSettingsLayout);
    menuLayout->addWidget(elementSettings);

    menuLayout->addSpacing(25);

    confirmSettings = new QPushButton(this);
    confirmSettings->setText(tr("Alle Einträge speichern"));
    connect(confirmSettings, &QPushButton::released, this, &EditorWidget::onDocumentSaved);
    menuLayout->addWidget(confirmSettings);
    mainLayout->addLayout(menuLayout, 0, 2);
 
    setLayout(mainLayout);
    setWindowTitle(tr("GeographyLearner: Editor"));
    readJson();
}

EditorWidget::~EditorWidget() {
    delete map;

    delete elementsSearch;
    delete searchLine;
    delete typeSearchFilter;
    delete matchingElements;

    delete addNewElementButton;

    delete elementSettings;
    delete elementNames;
    delete elementIsMetadata;
    delete elementType;
    delete elementPosContainer;
    delete setElementSelectorPos; 
    delete getElementSelectorPos; 
    delete flowsIntoGroup;
    delete searchFlowsInto;
    delete elementFlowsInto;
    delete searchState;
    delete elementState;

    delete confirmSettings; 
}


void EditorWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

void EditorWidget::onSearch(){
    listFilter.nameFragment = searchLine->text().toLower().toStdString();
    listFilter.targetType = uint8_t(typeSearchFilter->currentIndex() - 1);

    if(matchingElements->selectedItems().empty()) rebuildViewFromJson();
    else rebuildViewFromJson(-1, currentListItem->getBoundObject());
}

void EditorWidget::onSelectedElementChanged(){
    if(matchingElements->selectedItems().empty()) return;
    
    QListWidgetItem* item = matchingElements->selectedItems()[0];

    currentListItem = static_cast<JsonBoundListItem*>(item);
    json& currentElement = currentListItem->getBoundObject();

    //load relevant information
    if (!currentElement.contains("Name")) return;
        //assertM(false, "the known error occured");
    std::vector<std::string> names = currentElement["Name"].template get<std::vector<std::string>>();
    std::string namesInOne = names[0];
    for(int i = 1; i < names.size(); i++){
        namesInOne += "; " + names[i];
    }
    elementNames->setText(QString::fromStdString(namesInOne));

    uint8_t typeIndex = currentListItem->getAssociatedEntry().type;
    elementType->setCurrentIndex(typeIndex);

    elementIsMetadata->setChecked(currentElement["istMetaelement"].template get<bool>());

    if(typeIndex == jsonFormatUtils::Stadt || typeIndex == jsonFormatUtils::Bundesstaat){
        if(currentElement.contains("liegtInStaat")){
            std::string state = currentElement["liegtInStaat"].template get<std::string>();
            elementStateSet(state);
        }
        else elementStateSet("", false);
    } else if(typeIndex == jsonFormatUtils::Fluss){
        if(currentElement.contains("mündetIn")){
            std::string flowsInto = currentElement["mündetIn"].template get<std::string>();
            elementFlowsIntoSet(flowsInto);

        }
        else elementFlowsIntoSet("", false);
    }
    else{
        setNeitherStateNorFlowsInto();
    }

    showPosition();
   
}

void EditorWidget::onAddNewElement(){
    std::string targetName = listFilter.nameFragment.empty() ? "_NEUES_ELEMENT_" : listFilter.nameFragment;
    uint8_t targetType = listFilter.targetType == uint8_t(-1) ? 0 : listFilter.targetType;
    json& newObject = jsonFormatUtils::createUnique(configData, targetName, 
            jsonFormatUtils::supportedTypes[targetType]);

    newObject["istMetaelement"] = false;
    rebuildViewFromJson(-1, newObject);
}

void EditorWidget::onElementTypeChanged(int index){
    QList<QListWidgetItem*> list = matchingElements->selectedItems();
    if(list.isEmpty() || currentListItem == nullptr) return;
    json& currentEntry = currentListItem->getBoundObject();
    json elementCopy = currentEntry;
    std::string name = elementCopy["Name"].template get<std::vector<std::string>>()[0];
    json& newEntry = jsonFormatUtils::getNamed(configData, name, elementType->currentText().toStdString());

    if(newEntry == currentEntry) return;
    if(newEntry["Name"].template get<std::vector<std::string>>()[0] != name){
        QMessageBox::critical(this, windowTitle(),
                tr("Elementtyp konnte nicht geändert werden. Es existiert bereits ein gleichnamiges Element des Zieltyps"), 
                QMessageBox::Close, QMessageBox::Close);
        elementType->setCurrentIndex(currentListItem->getAssociatedEntry().type);
        json& atType = configData[jsonFormatUtils::supportedTypes[currentListItem->getAssociatedEntry().type]];
        for(int i = 0; i < atType.size(); i++){
            if(atType[i] != newEntry) continue;
            atType.erase(i);
            break;
        }
        return;
    }

    newEntry = elementCopy;

    if(index != jsonFormatUtils::Staat && index != jsonFormatUtils::Bundesstaat){
        if(newEntry.contains("liegtInStaat")) newEntry.erase("liegtInStaat");
    }
    else if (index != jsonFormatUtils::getSupportedTypesIndex("Fluss")){
        if(newEntry.contains("mündetIn")) newEntry.erase("mündetIn");
    }

    JsonBoundListItem* temp = currentListItem;
    currentListItem = nullptr;
    temp->removeEntry();
    deleteSafe(temp);
    
    rebuildViewFromJson(-1, newEntry);
}

void EditorWidget::onElementNamesEdited(){
    QList<QListWidgetItem*> list = matchingElements->selectedItems();
    if(list.isEmpty() || currentListItem == nullptr) return;
    std::vector<std::string> allNames;

    QStringList names = elementNames->text().split("; ");
    for(QString str : names){
        QStringList local_names = str.split(";");
        for(QString localStr : local_names){
            allNames.push_back(localStr.toStdString());
        }
    }

    currentListItem->getBoundObject()["Name"] = allNames;
    currentListItem->setText(QString::fromStdString(allNames[0]));
}

void EditorWidget::onElementIsMetadataChanged(bool checked){
    QList<QListWidgetItem*> list = matchingElements->selectedItems();
    if(list.isEmpty() || currentListItem == nullptr) return;

    currentListItem->getBoundObject()["istMetaelement"] = checked;

    if(checked) currentListItem->setIcon(QIcon(":/Data/World.ico"));
    else currentListItem->setIcon(QIcon(":/Data/Position.ico"));
}

void EditorWidget::onSavePosition(){
    QList<QListWidgetItem*> list = matchingElements->selectedItems();
    if(list.isEmpty() || currentListItem == nullptr) return;

    currentListItem->getBoundObject()["Position"] = {map->getSelectorPos().x(), map->getSelectorPos().y()};

}

void EditorWidget::showPosition(){
    QList<QListWidgetItem*> list = matchingElements->selectedItems();
    if(list.isEmpty() || currentListItem == nullptr) return;

    if(currentListItem->getBoundObject().contains("Position")){
        std::vector<double> position =
            currentListItem->getBoundObject()["Position"].template get<std::vector<double>>();
        assertM(position.size() == 2, "Position must be a 2D array");
        map->setSelectorPos(QPointF(position[0], position[1]));
    }
    else{
        map->setSelectorPos(QPointF(0.0, 0.0));
    }

    map->setSelectorColor(jsonFormatUtils::getAssociatedColor(currentListItem->getAssociatedEntry().type));
}


void EditorWidget::onSearchState(){
    QList<QListWidgetItem*> list = matchingElements->selectedItems();
    if(list.isEmpty() || currentListItem == nullptr) return;

    json& currentEntry = currentListItem->getBoundObject();
    if(currentEntry.contains("liegtInStaat")){
        std::string stateSpecifier = currentEntry["liegtInStaat"].template get<std::string>();
        
        resetFilter();
        bool hasFound = false;
        for(int i = 0; i < matchingElements->count(); i++){
            JsonBoundListItem* item = static_cast<JsonBoundListItem*>(matchingElements->item(i));
            int type = item->getAssociatedEntry().type;
            if(type != jsonFormatUtils::Staat) continue;
            std::vector<std::string> stateNames = 
                item->getBoundObject()["Name"].template get<std::vector<std::string>>();
            if(std::find(stateNames.begin(), stateNames.end(), stateSpecifier) == stateNames.end()) continue;

            matchingElements->scrollToItem(item);
            matchingElements->setCurrentItem(item);
            hasFound = true;
            break;
        }

        if(!hasFound){
            elementState->setVisible(false);
           //elementState->setStyleSheet("QPushButton { color : gray; }");
        }
    }
}

void EditorWidget::onElementStateEdited(){
    QList<QListWidgetItem*> list = matchingElements->selectedItems();
    if(list.isEmpty() || currentListItem == nullptr) return;
    json& currentEntry = currentListItem->getBoundObject();

    const QString& newName = elementState->text();
    if(newName.contains(";")){
        QMessageBox::warning(this, windowTitle(),
                tr("Das Feld \"(Bundes)Staat\" unterstützt nur die Nennung eines einzelnen Eintrages. \
                    Alternative Namen des (Bundes)Staates können in einem separaten Eintrag mit Typ \"Staat\" definiert werden."), 
                QMessageBox::Close, QMessageBox::Close); 
        if(currentEntry.contains("liegtInStaat")){
            std::string state = currentEntry["liegtInStaat"].template get<std::string>();
            elementStateSet(state);
        }
        else elementStateSet("", false);
        return;
    }

    if(newName.isEmpty()){
        if(currentEntry.contains("liegtInStaat")) currentEntry.erase("gehörtZuStaat");
        elementStateSet("", false);
    }
    else {
        currentEntry["liegtInStaat"] = newName.toStdString();
        elementStateSet(newName.toStdString());
    }
}

void EditorWidget::onSearchFlowsInto(){
    QList<QListWidgetItem*> list = matchingElements->selectedItems();
    if(list.isEmpty() || currentListItem == nullptr) return;
    json& currentEntry = currentListItem->getBoundObject();

    if(currentEntry.contains("mündetIn")){
        std::string flowsIntoSpecifier = currentEntry["mündetIn"].template get<std::string>();

        resetFilter();
        for(int i = 0; i < matchingElements->count(); i++){
            JsonBoundListItem* item = static_cast<JsonBoundListItem*>(matchingElements->item(i));
            int type = item->getAssociatedEntry().type;
            if(type != jsonFormatUtils::Fluss && type != jsonFormatUtils::See &&
                    type != jsonFormatUtils::Ozean && type != jsonFormatUtils::Meeresteil) continue;

            std::vector<std::string> flowsIntoNames = 
                item->getBoundObject()["Name"].template get<std::vector<std::string>>();
            if(std::find(flowsIntoNames.begin(), flowsIntoNames.end(), flowsIntoSpecifier) == flowsIntoNames.end()) continue;
            matchingElements->scrollToItem(item);
            matchingElements->setCurrentItem(item);
            break;
        }
    }
}

void EditorWidget::onElementFlowsIntoEdited(){
    QList<QListWidgetItem*> list = matchingElements->selectedItems();
    if(list.isEmpty() || currentListItem == nullptr) return;
    json& currentEntry = currentListItem->getBoundObject();

    const QString& newName = elementFlowsInto->text();
    if(newName.contains(";")){
        QMessageBox::warning(this, windowTitle(),
                tr("Das Feld \"Mündet in\" unterstützt nur die Nennung eines einzelnen Eintrages. \
                    Alternative Namen des Mündungsgewässers können in einem separaten Eintrag mit Typ \
                    \"Ozean\"/\"Meeresteil\"/\"See\"/\"Fluss\" definiert werden."), 
                QMessageBox::Close, QMessageBox::Close); 
        if(currentEntry.contains("mündetIn")){
            std::string flowsInto = currentEntry["mündetIn"].template get<std::string>();
            elementFlowsIntoSet(flowsInto);
        }
        else elementFlowsIntoSet("", false);
        return;
    }

    if(newName.isEmpty()){
        if(currentEntry.contains("mündetIn")) currentEntry.erase("mündetIn");
        elementFlowsIntoSet("", false);
    }
    else {
        currentEntry["mündetIn"] = newName.toStdString();
        elementFlowsIntoSet(newName.toStdString());
    }
}

void EditorWidget::onElementDeleted(){
    QList<QListWidgetItem*> toDelete;
    bool singleSelect = true;
    if(matchingElements->selectedItems().length() >= 2){
        QMessageBox::StandardButton retVal = QMessageBox::warning(this, windowTitle(),
                tr("Sollen alle Markierten Einträge tatsächlich gelöscht werden?"), 
                QMessageBox::YesAll | QMessageBox::NoAll | QMessageBox::Yes | QMessageBox::No, QMessageBox::NoAll);
        if(retVal == QMessageBox::YesAll){
            toDelete = matchingElements->selectedItems();
            singleSelect = false;
        } else if (retVal == QMessageBox::NoAll){
            singleSelect = false;
        }
    }


    if(singleSelect){
        for(QListWidgetItem* selectedItem : matchingElements->selectedItems()) {
            QMessageBox::StandardButton retVal = QMessageBox::warning(this, windowTitle(),
                    tr("Soll ") + selectedItem->text() + tr(" tatsächlich gelöscht werden?"),
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
            if(retVal == QMessageBox::Yes){
                toDelete.push_back(selectedItem);
            }
        }
    }

    for(QListWidgetItem* item : toDelete){
        if(item == nullptr) continue;
        currentListItem = nullptr;
        static_cast<JsonBoundListItem*>(item)->removeEntry();
        deleteSafe(item);
    }

    rebuildViewFromJson(matchingElements->currentRow());
}

void EditorWidget::onDocumentSaved(){
    for(int i = 0; i < matchingElements->count(); i++){
        JsonBoundListItem* item = static_cast<JsonBoundListItem*>(matchingElements->item(i));
        if(!item->getBoundObject().contains("Position")){
            QMessageBox::critical(this, windowTitle(),
                    tr("Das Element ") + item->text() + tr(" hat keine zugeordnete Position. Ordne dem Element eine Position zu, um fortzufahren."), 
                    QMessageBox::Close, QMessageBox::Close);
            matchingElements->setCurrentItem(item);
            matchingElements->scrollToItem(item);
            return;
        }
    }

    std::ofstream outStream("F:/DevelopmentProjects/GeographyLearner/Data/configData.json");
    
    if(outStream.is_open()) outStream << std::setw(4) << configData << std::endl; 
    
    if(outStream.fail() || !outStream.is_open()){
        QMessageBox::critical(this, windowTitle(),
                tr("Datei konnte nicht gespeichert werden."), QMessageBox::Close, QMessageBox::Close);
    }
}

void EditorWidget::resetFilter(){
    listFilter = FilterProperties();
    searchLine->setText("");
    typeSearchFilter->setCurrentIndex(0);
    rebuildViewFromJson(-1, currentListItem->getBoundObject());
}

void EditorWidget::readJson(){
    //NOTE: this usage of the path assumes the executable is located at a certain location
    std::ifstream inputStream("F:/DevelopmentProjects/GeographyLearner/Data/configData.json");
    if(!inputStream.is_open()) {
        QMessageBox::StandardButton retVal = QMessageBox::critical(this, windowTitle(),
                tr("Konfigurationsdatei kann nicht geöffnet werden."), QMessageBox::Abort);
        return;
    }

    inputStream >> configData;

    rebuildViewFromJson();
}

void EditorWidget::rebuildViewFromJson(int row, const json& target){
    if(matchingElements->count() > 0){
        matchingElements->clear();
    }
    

    bool foundTarget = false;
    if(configData["Version"] != "0.1.0"){
        std::cerr << "Has to have matching version";
        std::terminate();
    }

    for(auto& [type, ofType] : configData.items()){
        uint8_t index = jsonFormatUtils::getSupportedTypesIndex(type); 
        if(index == uint8_t(-1)){
            if(type == "Version"){}
            continue;
        }
        uint8_t typeIndex = index & ~jsonFormatUtils::legacyFlag;


        assertM(ofType.is_array(), "Can only build view on array data");

        if(listFilter.targetType != uint8_t(-1) && listFilter.targetType != typeIndex) continue;

        for(int i = 0; i < ofType.size(); i++){
            if (!ofType[i].contains("Name")) continue;
            const json& object = ofType[i];
            std::vector<std::string> names = object["Name"].template get<std::vector<std::string>>();
            if(!listFilter.nameFragment.empty()){
                bool foundMatch = false;
                for(std::string name : names){
                    std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return std::tolower(c);});
                    if(name.find(listFilter.nameFragment) == std::string::npos) continue;
                    foundMatch = true;
                    break;
                }
                if(!foundMatch) continue;
            }

            std::string defaultName = names[0];
            json::json_pointer ptr;
            ptr.push_back(type);
            ptr.push_back(std::to_string(i));
            JsonBoundListItem *newItem = new JsonBoundListItem({ptr, configData, typeIndex}, QString::fromStdString(names[0]), matchingElements);

            if(object["istMetaelement"].template get<bool>()) newItem->setIcon(QIcon(":/Data/World.ico"));
            else newItem->setIcon(QIcon(":/Data/Position.ico"));


            matchingElements->addItem(newItem);
            if(!foundTarget && row < 0){
                if(object != target) continue;
                foundTarget = true;
                matchingElements->setCurrentItem(newItem);
                matchingElements->scrollToItem(newItem);
            }
        }
    }

    if(matchingElements->count() > 0 && row >= 0){
        if(matchingElements->count() > row){
            matchingElements->setCurrentRow(row);
        }
        else matchingElements->setCurrentRow(matchingElements->count() - 1);
    }
}

void EditorWidget::onEditDataInputSubmitted()
{
    //std::string input = answerLine->text().toStdString();
    std::string input;
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
    json& objectConfig = jsonFormatUtils::getNamed(configData, objectName, type);

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
    std::ofstream outStream("F:/DevelopmentProjects/GeographyLearner/Data/configData.json");
    outStream << std::setw(4) << configData << std::endl; 
    //answerLine->setText("");

    //std::stringstream str;
    //str << std::setw(4) << configData;
    
    //fileBrowser->setText(QString::fromStdString(str.str()));
}

void EditorWidget::elementStateSet(const std::string& str, bool shouldCheckExists){
    QList<QListWidgetItem*> list = matchingElements->selectedItems();
    if(list.isEmpty() || currentListItem == nullptr) return;
    JsonBoundListItem* currentItem = static_cast<JsonBoundListItem*>(list[0]);
    assertM(currentItem->getAssociatedEntry().type == jsonFormatUtils::Stadt ||
            currentItem->getAssociatedEntry().type == jsonFormatUtils::Bundesstaat,
            "Can only set state on compatible element");
    elementState->setText(QString::fromStdString(str));

    bool exists = false;
    if(shouldCheckExists){
        for(auto& [typeString, ofType] : configData.items()){
            int localType = jsonFormatUtils::getSupportedTypesIndex(typeString) & ~jsonFormatUtils::legacyFlag;
            if(localType != jsonFormatUtils::Staat &&
                localType != jsonFormatUtils::Bundesstaat) continue;
            for(json& element : ofType){
                if (!element.contains("Name")) continue;
                std::vector<std::string> names = element["Name"].template get<std::vector<std::string>>();
                
                if(std::find(names.begin(), names.end(), str) == names.end()) continue;
                exists = true;
                break;
            }
        }
    }

    if(!exists){
        QSizePolicy retainSize = searchState->sizePolicy();
        retainSize.setRetainSizeWhenHidden(true);
        searchState->setSizePolicy(retainSize);
        searchState->setVisible(false);
        stateGroup->setVisible(true);
    }
    else{
        elementState->setVisible(true);
        searchState->setVisible(true);
        stateGroup->setVisible(true);
    }

    QSizePolicy retainSize = searchFlowsInto->sizePolicy();
    retainSize.setRetainSizeWhenHidden(false);
    searchFlowsInto->setSizePolicy(retainSize);
    retainSize = flowsIntoGroup->sizePolicy();
    retainSize.setRetainSizeWhenHidden(false);
    flowsIntoGroup->setSizePolicy(retainSize);

    flowsIntoGroup->setVisible(false);

}

void EditorWidget::elementFlowsIntoSet(const std::string& str, bool shouldCheck){
    QList<QListWidgetItem*> list = matchingElements->selectedItems();
    if(list.isEmpty() || currentListItem == nullptr) return;
    JsonBoundListItem* currentItem = static_cast<JsonBoundListItem*>(list[0]);
    assertM(currentItem->getAssociatedEntry().type == jsonFormatUtils::Fluss, "Setting FlowsInto is only allowed on rivers");
    elementFlowsInto->setText(QString::fromStdString(str));

    bool exists = false;
    if(shouldCheck){
        for(auto& [typeString, ofType] : configData.items()){
            int type = jsonFormatUtils::getSupportedTypesIndex(typeString) & ~jsonFormatUtils::legacyFlag;
            if(type != jsonFormatUtils::Fluss && type != jsonFormatUtils::See &&
                    type != jsonFormatUtils::Ozean && type != jsonFormatUtils::Meeresteil) continue;

           for(json& element : ofType){
                std::vector<std::string> names = element["Name"].template get<std::vector<std::string>>();
                
                if(std::find(names.begin(), names.end(), str) == names.end()) continue;
                exists = true;
                break;
            }
        }
    }
    
    if(!exists){
        QSizePolicy retainSize = searchFlowsInto->sizePolicy();
        retainSize.setRetainSizeWhenHidden(true);
        searchFlowsInto->setSizePolicy(retainSize);
        searchFlowsInto->setVisible(false);
        flowsIntoGroup->setVisible(true);
    }
    else{
        elementFlowsInto->setVisible(true);
        searchFlowsInto->setVisible(true);
        flowsIntoGroup->setVisible(true);
    }

    QSizePolicy retainSize = searchState->sizePolicy();
    retainSize.setRetainSizeWhenHidden(false);
    searchState->setSizePolicy(retainSize);
    retainSize = stateGroup->sizePolicy();
    retainSize.setRetainSizeWhenHidden(false);
    stateGroup->setSizePolicy(retainSize);

    stateGroup->setVisible(false);
}

void EditorWidget::setNeitherStateNorFlowsInto(){
    QSizePolicy retainSize = searchState->sizePolicy();
    retainSize.setRetainSizeWhenHidden(true);
    searchState->setSizePolicy(retainSize);
    retainSize = stateGroup->sizePolicy();
    retainSize.setRetainSizeWhenHidden(true);
    stateGroup->setSizePolicy(retainSize);

    searchState->setVisible(false);
    stateGroup->setVisible(false);


    retainSize = searchFlowsInto->sizePolicy();
    retainSize.setRetainSizeWhenHidden(false);
    searchFlowsInto->setSizePolicy(retainSize);
    retainSize = flowsIntoGroup->sizePolicy();
    retainSize.setRetainSizeWhenHidden(false);
    flowsIntoGroup->setSizePolicy(retainSize);

    searchFlowsInto->setVisible(false);
    flowsIntoGroup->setVisible(false);
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
