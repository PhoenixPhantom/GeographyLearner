#ifndef EDITOR_WIDGET_H
#define EDITOR_WIDGET_H

#include <QWidget>
#include <QProcess>
#include <qlistwidget.h>
#include "../utils/json.hpp"

using json = nlohmann::json;

class QPushButton;
class QComboBox;
class QLabel;
class QGroupBox;
class QTextLine;
class QLineEdit;
class MapWidget;
class QListWidget;
class QListWidgetItem;
class QCheckBox;

using json_ptr = json::json_pointer;

struct elementRef{
    json_ptr ptr;
    json& root;
    uint8_t type;
    json& get(){ return root[ptr]; }
    inline void deleteObj(){
        json& parent = root.at(ptr.parent_pointer());
        if(parent.is_array()){
            parent.erase(std::stoi(ptr.back()));
        }
        else{
            assert(parent.is_object());
            parent.erase(ptr.back());
        }
    }
};

class JsonBoundListItem : public QListWidgetItem
{
public:
    JsonBoundListItem(const elementRef& entry);
    explicit JsonBoundListItem(const elementRef& entry, QListWidget *listview = nullptr, int type = Type);
    explicit JsonBoundListItem(const elementRef& entry, const QString &text, QListWidget *listview = nullptr, int type = Type);
    explicit JsonBoundListItem(const elementRef& entry, const QIcon &icon, const QString &text,
                             QListWidget *listview = nullptr, int type = Type);

    const elementRef& getAssociatedEntry() const { return associatedEntry; }
    json& getBoundObject(){ return associatedEntry.get(); }
    inline void removeEntry(){ associatedEntry.deleteObj(); }
private:
    elementRef associatedEntry;
};

struct FilterProperties{
    std::string nameFragment;
    uint8_t targetType = uint8_t(-1);
};

class EditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EditorWidget(QWidget* parent = nullptr);
    ~EditorWidget();

protected:
    void resizeEvent(QResizeEvent* event) override;
private slots:
    void onSearch();
    void onSelectedElementChanged();
    void onAddNewElement();
    void onElementTypeChanged(int index);
    void onElementNamesEdited();
    void onElementIsMetadataChanged(bool checked);
    void onSavePosition();
    void showPosition();
    void onSearchState();
    void onElementStateEdited();
    void onSearchFlowsInto();
    void onElementFlowsIntoEdited();
    void onElementDeleted();
    void onDocumentSaved();
private:
    MapWidget* map;

    //Search part
    QGroupBox* elementsSearch;
    QLineEdit* searchLine;
    QComboBox* typeSearchFilter;
    QListWidget* matchingElements;
    QPushButton* addNewElementButton;

    //edit properties part
    QGroupBox* elementSettings;
    QLineEdit* elementNames;
    QCheckBox* elementIsMetadata;
    QComboBox* elementType;
    QGroupBox* elementPosContainer;
    QPushButton* setElementSelectorPos; 
    QPushButton* getElementSelectorPos; 
    QGroupBox* flowsIntoGroup;
    QPushButton* searchFlowsInto;
    QLineEdit* elementFlowsInto;
    QGroupBox* stateGroup;
    QPushButton* searchState;
    QLineEdit* elementState;
    QPushButton* elementDelete;

    //general functionality
    QPushButton* confirmSettings; 

    json configData;
    JsonBoundListItem* currentListItem;
    FilterProperties listFilter;

    void resetFilter();
    void readJson();
    void rebuildViewFromJson(int row = 0, const json& target = json());
    void onEditDataInputSubmitted();
    void updateJson();

    void elementStateSet(const std::string& str, bool shouldCheckExists = true);
    void elementFlowsIntoSet(const std::string& str, bool shouldCheckExists = true);
    void setNeitherStateNorFlowsInto();

    static json restructureJson(const json& target);
    static std::string extractArgument(const std::string& container, const std::string& finder);
    static void extractArgumentList(const std::string& container, const std::string& finder,
            std::vector<std::string>& located); 
};

#endif //MAIN_WIDGET_H
