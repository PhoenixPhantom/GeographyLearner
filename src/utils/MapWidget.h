#ifndef MAP_WIDGET_H
#define MAP_WIDGET_H

#include "qpoint.h"
#include <QLabel>

class MapWidget : public QFrame
{
    Q_OBJECT

public:
    explicit MapWidget(const QString& pathToMap, const QSize& selectorSize, bool compatibilityMode = true, QWidget* parent = nullptr);
    ~MapWidget();

    QPointF getSelectorPos() const{ return localPos; }
    void setCanMoveSelector(bool canMove){ bCanMove = canMove; }

    void setSelectorColor(const QColor& color, bool compatibilityMode = true);
    void setSelectorPos(const QPointF& newPos){ setLocalPos(newPos); updateSelectorPos(); }
protected:
    void resizeEvent(QResizeEvent* event) override;
    void setLocalPos(const QPointF& newL){ localPos = newL; };
    void updateSelectorPos();
    void setLocalPosFromLocal(const QPoint& newlocalPos);
private:
    int8_t bCanMove:1;
    double mapAspectRatio;
    QPointF localPos;
    QSize scaledImgSize;
    QPixmap* mapImage;
    QLabel* map;
    QLabel* selector;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    QPoint getReference(const QPixmap& temp) const;
};

#endif //MAP_WIDGET_H
