#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "imageviewer.h"
#include <cmath>
#include <QPainter>
#include <QDebug>
#include <QFileDialog>
#include <QColorDialog>
#include <QElapsedTimer>
#include <QThread>

const double kPI = std::acos(-1.0);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setCentralWidget(canvas_ = new ImageViewer(this));

    // other setup
    ui->labelBColor->setAutoFillBackground(true);
    ui->labelFColor->setAutoFillBackground(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pBDraw_clicked()
{
    max_level_    = ui->spinBoxLevels->value();
    angles1_      = ui->spinBoxAng1->value();
    angles2_      = ui->spinBoxAng2->value();
    max_branches_ = ui->spinBoxBranches->value();
    max_len_      = ui->spinBoxLen->value();
    ball_radius_  = ui->doubleSpinBoxBallSize->value();
    bsf_          = ui->doubleSpinBoxBranchSf->value();
    irregular_levels_ = ui->checkBoxIrregularLevels->isChecked();
    growing_branches_ = ui->checkBoxGrowingBrances->isChecked();



    QSize ss(ui->spinBoxW->value(), ui->spinBoxH->value() );
    if (ss.width() <= 0 ){
        int len = canvas_->viewport()->width();
        ss.setWidth(len);
        ui->spinBoxW->setValue(len);
    }
    if (ss.height() <= 0 ){
        int len = canvas_->viewport()->height();
        ss.setHeight(len);
        ui->spinBoxH->setValue(len);
    }
    on_doubleSpinBoxSf_valueChanged(ui->doubleSpinBoxSf->value());
    bcolor_ = ui->labelBColor->palette().color(QPalette::Window);
    fcolor_ = ui->labelFColor->palette().color(QPalette::Window);

    qDebug() << "####################################################";
    qDebug() << "Max. Levels:" <<   max_level_;
    qDebug() << "Angles:" << angles1_ << " to " << angles2_;
    qDebug() << "Len:" << max_len_ ;
    qDebug() << "Ball radius:" << ball_radius_ ;
    qDebug() << "Branch Scale factor:" << bsf_ ;
    qDebug() << "Irregular levels:" << irregular_levels_ ;
    qDebug() << "Growing Branches: " << growing_branches_ ;
    qDebug() << "####################################################";


    image_ = QImage(ss, QImage::Format_ARGB32);
    image_.fill(Qt::white);

    picture_drawer_ = QPicture();

    QPainter painter(&picture_drawer_);
    painter.setRenderHints(QPainter::Antialiasing);

    const double cx = ui->doubleSpinBoxCx->value()*ss.width();
    const double cy = ui->doubleSpinBoxCy->value()*ss.height();
    QPointF center(cx, cy);
    QPointF last(cx, cy+max_len_);

    auto pen = painter.pen();
    pen.setWidthF(ui->doubleSpinBoxBranchWidth->value());
    pen.setColor(bcolor_);
    painter.setPen(pen);


    QElapsedTimer timer;
    timer.start();
    if (ui->checkBoxRecursive->isChecked()){
        RecursiveDraw(&painter, last, center,0);
    } else {
        DirectDraw(&painter, last, center);
    }
    ui->statusbar->showMessage(QString("Elapsed time %1 ms").arg(timer.elapsed()));

    painter.end();


    painter.begin(&image_);
    painter.drawPicture(0, 0, picture_drawer_);    // draw the picture at (0,0)
    painter.end();                                 // painting done

    canvas_->AttachImagePtr(&image_);
    on_horizontalSlider_valueChanged(ui->horizontalSlider->value());
}

void MainWindow::RecursiveDraw(QPainter *p, const QPointF &last_node, const QPointF &curr_node, const int level)
{
    const int last_level = irregular_levels_ ? (rand()%max_level_+1) : max_level_;
    if (level>=last_level){
        p->save();
        p->setBrush(QBrush(fcolor_, Qt::SolidPattern));
        p->setPen(Qt::NoPen);
        p->drawEllipse(curr_node, ball_radius_,ball_radius_);
        p->restore();
        return;
    }

    const auto l = curr_node-last_node;
    const double base_len = growing_branches_ ? std::sqrt( l.x()*l.x()+l.y()*l.y() ) : max_len_;
    const int nb = std::rand()%max_branches_+1;
    const double base_angle = std::atan2(l.y(), l.x());
    int N=10000;
    for (int i=0; i<nb; ++i){

        const double ang = base_angle + ( angles1_ + rand()%(angles2_-angles1_))*kPI/180.0;

        const double f  = (1+rand()%N)/static_cast<double>(N);
        const QPointF new_node = curr_node +(0.5*base_len + f*base_len)*QPointF(std::cos(ang), std::sin(ang));
        p->save();
        auto pen = p->pen();
        pen.setWidthF(pen.widthF()*bsf_);

        p->setPen(pen);
        p->drawLine(curr_node, new_node);
        RecursiveDraw(p, curr_node, new_node, level+1);
        p->restore();
    }
}

void MainWindow::DirectDraw(QPainter *p, const QPointF &last_node, const QPointF &curr_node)
{
    std::vector<QPointF> nodes; nodes.reserve(max_level_+1);
    std::vector<int> nbranches; nbranches.reserve(max_level_+1);
    std::vector<int> finish_branches_; finish_branches_.reserve(max_len_+1);
    int level = 0;

    finish_branches_.push_back(0);
    nbranches.push_back(std::rand()%max_branches_+1);
    nodes.push_back(curr_node);

    const int N=100000;
    while (true){
        const int last_level = irregular_levels_ ? (rand()%max_level_+1) : max_level_;
        if (level>=last_level){
            p->save();
            p->setBrush(QBrush(fcolor_, Qt::SolidPattern));
            p->setPen(Qt::NoPen);
            p->drawEllipse(nodes[level], ball_radius_,ball_radius_);
            p->restore();

            nodes.pop_back();
            nbranches.pop_back();
            finish_branches_.pop_back();
            level-=1;
            continue;
        }

        if ( finish_branches_[level] < nbranches[level] ){

            const auto l = level ? nodes[level] - nodes[level-1] : curr_node - last_node;
            const double base_len = growing_branches_ ? std::sqrt( l.x()*l.x()+l.y()*l.y() ) : max_len_;
            const int nb = std::rand()%max_branches_+1;
            const double base_angle = std::atan2(l.y(), l.x());
            const double ang = base_angle + ( angles1_ + rand()%(angles2_-angles1_))*kPI/180.0;
            const double f  = (1+rand()%N)/static_cast<double>(N);
            const QPointF new_node = nodes[level] +(0.5*base_len + f*base_len)*QPointF(std::cos(ang), std::sin(ang));

            p->save();
            auto pen = p->pen();
            pen.setWidthF(pen.widthF()*pow(bsf_, level));

            p->setPen(pen);
            p->drawLine(nodes[level], new_node);
            p->restore();

            finish_branches_[level] += 1;
            finish_branches_.push_back(0);
            nbranches.push_back(nb);
            nodes.push_back(new_node);
            level +=1 ;

        } else {
            nodes.pop_back();
            nbranches.pop_back();
            finish_branches_.pop_back();
            level-=1;
        }

        if (nodes.empty()){
            break;
        }
    }

}





void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    canvas_->SetScf(value/500.0);
}


void MainWindow::on_pushButton_clicked()
{
    ui->horizontalSlider->setValue(500);
}

void MainWindow::on_doubleSpinBoxSf_valueChanged(double arg1)
{
    ui->labelCanvasW->setNum( static_cast<int>( ui->spinBoxW->value()*arg1  )  );
    ui->labelCanvasH->setNum( static_cast<int>( ui->spinBoxH->value()*arg1  )  );
}

void MainWindow::on_pushButtonSave_clicked()
{
    QString file_path = QFileDialog::getSaveFileName(this);
    if (file_path.isEmpty()) return;
    QSize ss;
    ss.setWidth(ui->labelCanvasW->text().toInt());
    ss.setHeight(ui->labelCanvasH->text().toInt());
    QImage canvas_image(ss, QImage::Format_ARGB32);
    canvas_image.fill(Qt::white);
    QPainter painter(&canvas_image);

    const double scf = ui->doubleSpinBoxSf->value();
    painter.scale(scf, scf);
    painter.drawPicture(0,0,picture_drawer_);
    painter.end();

    canvas_image.save(file_path);

}

void MainWindow::on_pushButtonBColor_clicked()
{
    SelectColor(ui->labelBColor);
}

void MainWindow::SelectColor(QLabel *lb)
{
    QColorDialog dlg(this);
    QColor selected_color = dlg.getColor(lb->palette().color(QPalette::Window));
    if (selected_color.isValid()){
        QPalette pal(lb->palette());
        pal.setColor(QPalette::Window, selected_color);
        lb->setPalette(pal);
    }
}

void MainWindow::on_pushButtonFColor_clicked()
{
    SelectColor(ui->labelFColor);
}


