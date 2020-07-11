#ifndef VIDEOCANVAS_H
#define VIDEOCANVAS_H

#include <QWidget>
#include <QImage>

class VideoCanvas : public QWidget
{
    Q_OBJECT
public:
    explicit VideoCanvas(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event) override final;
    QImage GetCurrentFrame() const {return frame_; }

public slots:
    void SetFrame(const QImage &frame);


private:
    QImage frame_;

};

#endif // VIDEOCANVAS_H
