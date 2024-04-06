#include "MapWidget.h"

#include <QtWidgets>

MapWidget::MapWidget(const QString& pathToMap, const QSize& selectorSize, QWidget* parent) :
    QFrame(parent)
{

    setAcceptDrops(true);

    mapImage = new QPixmap(pathToMap);

    QVBoxLayout* gridLayout = new QVBoxLayout(this);
    map = new QLabel(this);
    map->setPixmap(*mapImage);
    gridLayout->addWidget(map);
    setLayout(gridLayout);

    selector = new QLabel(this);
    QPixmap selectorImg(":/Data/DotSelector.png");
    selector->setPixmap(selectorImg.scaled(selectorSize, Qt::KeepAspectRatio));
    selector->setFixedSize(selectorSize);

    localPos = QPoint(100, 100);
    selector->move(map->mapToParent(localPos).toPoint());
    selector->show();
}

MapWidget::~MapWidget(){
    delete selector;
}

void MapWidget::resizeEvent(QResizeEvent* event)
{
    //ATTENTION: this has the potential to scale the application towards infinity if QSize()
    const QPixmap& temp = mapImage->scaled(size()-QSize(20, 20), Qt::KeepAspectRatio);
    map->setPixmap(temp);
    scaledImgSize = temp.size();
    QFrame::resizeEvent(event);
    
    updateSelectorPos();
}

void MapWidget::updateSelectorPos(){
    selector->move((map->mapToParent(localPos*(double(scaledImgSize.width()/double(mapImage->width()))))
                + QPointF(double(map->width() - scaledImgSize.width())*0.5, double(map->height() - scaledImgSize.height())*0.5)).toPoint());
}


void MapWidget::setLocalPosFromLocal(const QPoint& newLocalPos){
    localPos = map->mapFromParent(QPointF(newLocalPos) -
            QPointF(double(map->width() - scaledImgSize.width())*0.5, double(map->height() - scaledImgSize.height())*0.5))*
        (double(mapImage->width())/double(scaledImgSize.width()));


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
        const QPoint& newLocalPos = event->position().toPoint() - offset;
        selector->move(newLocalPos);
        selector->setVisible(true);

        setLocalPosFromLocal(newLocalPos);


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

QPoint MapWidget::getReference(const QPixmap& temp) const
{
    return pos()+QPoint(temp.width()*0.5, temp.height()*0.5);
}
