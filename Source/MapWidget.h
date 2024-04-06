#ifndef MAP_WIDGET_H
#define MAP_WIDGET_H

#include <QLabel>

class MapWidget : public QFrame
{
    Q_OBJECT

public:
    explicit MapWidget(const QPixmap& mapImage, const QSize& selectorSize, QWidget* parent = nullptr);
    ~MapWidget();

    QPoint getSelectorPos() const{ return selector->pos(); }
private:
    QLabel* map;
    QLabel* selector;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
};

#endif //MAP_WIDGET_H
