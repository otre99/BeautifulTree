#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPicture>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QLabel;
QT_END_NAMESPACE

class ImageViewer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pBDraw_clicked();
    void on_horizontalSlider_valueChanged(int value);
    void on_pushButton_clicked();
    void on_doubleSpinBoxSf_valueChanged(double arg1);
    void on_pushButtonSave_clicked();
    void on_pushButtonBColor_clicked();
    void on_pushButtonFColor_clicked();

private:
    void SelectColor(QLabel *);
    Ui::MainWindow *ui;
    void RecursiveDraw(QPainter *p, const QPointF &last_node, const QPointF &curr_node, const int level);
    void DirectDraw(QPainter *p, const QPointF &last_node, const QPointF &curr_node);


    int max_level_;
    int max_branches_;
    int angles1_, angles2_;
    double ball_radius_;
    double bsf_;
    double max_len_;
    bool irregular_levels_;
    bool growing_branches_;

    QColor fcolor_;
    QColor bcolor_;

    ImageViewer *canvas_;
    QImage image_;
    QPicture picture_drawer_;

};
#endif // MAINWINDOW_H
