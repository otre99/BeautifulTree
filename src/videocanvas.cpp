#include "videocanvas.h"
#include <QPainter>
#include <QDebug>

VideoCanvas::VideoCanvas(QWidget *parent) : QWidget(parent)
{

}

void VideoCanvas::paintEvent(QPaintEvent *event)
{
    if ( !frame_.isNull() ){
        QPainter painter(this);
        painter.drawImage(painter.window(), frame_);
    } else {
        QWidget::paintEvent(event);
    }
}

void VideoCanvas::SetFrame(const QImage &frame)
{
    frame_ = frame;//.copy();
    update();
}
