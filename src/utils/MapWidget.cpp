#include "MapWidget.h"
#include <QtWidgets>

MapWidget::MapWidget(const QString& pathToMap, const QSize& selectorSize, bool compatibilityMode, QWidget* parent) :
    QFrame(parent), bCanMove(false)
{

    setAcceptDrops(true);

    mapImage = new QPixmap(pathToMap);
    mapAspectRatio = double(mapImage->width())/double(mapImage->height());

    QVBoxLayout* gridLayout = new QVBoxLayout(this);
    map = new QLabel(this);
    map->setPixmap(*mapImage);
    gridLayout->addWidget(map);
    setLayout(gridLayout);

    selector = new QLabel(this);
    selector->setFixedSize(selectorSize);
    setSelectorColor(QColor(255, 0, 0), compatibilityMode);


    localPos = QPoint(100, 100);
    selector->move(map->mapToParent(localPos).toPoint());
    selector->show();
}

MapWidget::~MapWidget(){
    delete selector;
}

void MapWidget::setSelectorColor(const QColor& color, bool compatibilityMode)
{
    const QSize localSize = compatibilityMode ? selector->size()*51.2 : selector->size();
    QPixmap selectorImg(localSize);
    selectorImg.fill(QColor(0, 0, 0, 0));
    QPainter p(&selectorImg);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QPen(color));
    p.setBrush(QBrush(color));
    p.drawEllipse(localSize.width()*0.1, localSize.height()*0.1, localSize.width()*0.8, localSize.height()*0.8);
    if(compatibilityMode) selector->setPixmap(selectorImg.scaled(selector->size(), Qt::KeepAspectRatio)); 
    else selector->setPixmap(selectorImg);
}


void MapWidget::resizeEvent(QResizeEvent* event)
{
    //ATTENTION: this has the potential to scale the application towards +infinity or -infinity
    
    const QPixmap& temp = mapImage->scaled(map->size(), Qt::KeepAspectRatio);
    map->setPixmap(temp);
    scaledImgSize = temp.size();
    QFrame::resizeEvent(event);
    
    updateSelectorPos();
}

void MapWidget::updateSelectorPos(){
    selector->move((map->mapToParent(localPos*(double(map->width()/double(mapImage->width()))))
                + QPointF(qMax(double(map->width() - scaledImgSize.width())*0.5, 0.0),
                    qMax(double(map->height() - scaledImgSize.height())*0.5, 0.0))).toPoint());
}


void MapWidget::setLocalPosFromLocal(const QPoint& newLocalPos){
    localPos = map->mapFromParent(QPointF(newLocalPos) -
            QPointF(qMax(double(map->width() - scaledImgSize.width())*0.5, 0.0),
                qMax(double(map->height() - scaledImgSize.height())*0.5, 0.0)))*
       (double(mapImage->width())/double(map->width()));


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
    if (!bCanMove || !child || child != selector) return;

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
