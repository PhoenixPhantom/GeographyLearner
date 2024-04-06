#include "MapWidget.h"

#include <QtWidgets>

MapWidget::MapWidget(const QPixmap& mapImage, const QSize& selectorSize, QWidget* parent) :
    QFrame(parent)
{  
    setAcceptDrops(true);

    QGridLayout* gridLayout = new QGridLayout(this);
    map = new QLabel(this);
    map->setPixmap(mapImage);
    gridLayout->addWidget(map);
    setLayout(gridLayout);

    selector = new QLabel(this);
    QPixmap selectorImg(":/Data/DotSelector.png");
    selector->setPixmap(selectorImg.scaled(selectorSize, Qt::KeepAspectRatio));
    selector->setFixedSize(selectorSize);
    selector->move(100, 100);
    selector->show();
 
    setWindowTitle(tr("A simple geography learner"));

}

MapWidget::~MapWidget(){
    delete selector;
}

void MapWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

void MapWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

void MapWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
        QByteArray itemData = event->mimeData()->data("application/x-dnditemdata");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);

        QPixmap pixmap;
        QPoint offset;
        dataStream >> pixmap >> offset;
        
        if(pixmap.toImage() != selector->pixmap().toImage()) return;
        selector->move(event->position().toPoint() - offset);
        selector->setVisible(true);

        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

void MapWidget::mousePressEvent(QMouseEvent* event)
{
    QLabel *child = static_cast<QLabel*>(childAt(event->position().toPoint()));
    if (!child) return;
    if (child != selector) return;

    QPixmap pixmap = selector->pixmap();

    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << pixmap << QPoint(event->position().toPoint() - selector->pos());

    QMimeData *mimeData = new QMimeData();
    mimeData->setData("application/x-dnditemdata", itemData);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->setHotSpot(event->position().toPoint() - selector->pos());
    selector->setVisible(false);

    drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);

}
