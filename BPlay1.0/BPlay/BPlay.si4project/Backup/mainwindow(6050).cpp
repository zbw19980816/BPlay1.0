/********************************
 * mainwindow.cpp
 * 功能：主窗口cpp
 * *****************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Bpublic.h"
#include "Bffmpeg.h"
#include <QFileDialog>
#include <QDebug>

/********************************
 * MainWindow::MainWindow(QWidget *parent)
 * 功能：主窗口构造函数
 * *****************************/
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* BPlay Logo */
    this->setWindowIcon(QIcon(":/BPlay/Image/WindowIcon.png"));
    this->setWindowTitle(QString("BPlay1.0"));
    this->

    /* 开始/停止播放按钮 */
    OnOffBbutton = new Bbutton(":/BPlay/Image/on.png", ":/BPlay/Image/off.png");
    OnOffBbutton->setParent(this);
    OnOffBbutton->move(50, 530);

    /* 进度条和播放时长 */
    ui->BPlaySlider->setValue(0);
    ui->MediaAllTime->setText("00:00");
    ui->MediaPlayTime->setText("00:00");

    /* 进度条刷新 */
    connect(&UpdateTimer, &QTimer::timeout, [=](){
        int TimeAll = (int)Bffmpeg::GetInstance()->GetTimeAll();
        int Pts = (int)Bffmpeg::GetInstance()->GetAudioPts();

        ui->MediaAllTime->setText(QString("%1:%2:%3").arg(TimeAll / 3600).arg((TimeAll % 3600) / 60).arg(TimeAll % 60));
        ui->MediaPlayTime->setText(QString("%1:%2:%3").arg(Pts / 3600).arg((Pts % 3600) / 60).arg(Pts % 60));

        if (!BPlaySliderPress) {
            ui->BPlaySlider->setMaximum(TimeAll);
            ui->BPlaySlider->setValue(Pts);
        }
    });

    UpdateTimer.start(100);  //定视频没有播放时时器可以关闭，后期优化

    /* 为进度条注册事件过滤器(鼠标点击进度条任意处可准确修改滑块位置) */
    ui->BPlaySlider->installEventFilter(this);
}

/********************************
 * MainWindow::~MainWindow()
 * 功能：主窗口析构函数
 * *****************************/
MainWindow::~MainWindow()
{
    delete ui;
}

/********************************
 * bool MainWindow::eventFilter(QObject *obj, QEvent *event)
 * 功能：事件过滤器
 * *****************************/
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->BPlaySlider) {
        /* 滑动条事件过滤 */
        if (event->type() == QEvent::MouseButtonPress) {
            //BPlaySliderPress = true;
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            int value = QStyle::sliderValueFromPosition(ui->BPlaySlider->minimum(), ui->BPlaySlider->maximum(), mouseEvent->pos().x(), ui->BPlaySlider->width());
            ui->BPlaySlider->setValue(value);
            //BPlaySliderPress = false;
            BLOG("sssssssss222on_BPlaySlider_sliderReleased  %d", ui->BPlaySlider->value());
            Bffmpeg::GetInstance()->ResetTime(ui->BPlaySlider->value());
        }
    }
    return QObject::eventFilter(obj,event);
}

/********************************
 * void MainWindow::on_BPlaySlider_valueChanged()
 * 功能：滑块的值发生改变回调函数
 * *****************************/
void MainWindow::on_BPlaySlider_valueChanged()
{
    //BPlaySliderPress = true;
    BLOG("on_BPlaySlider_valueChangeddddddddddddddddddddddd");

    return;
}

/********************************
 * void MainWindow::on_BPlaySlider_sliderPressed()
 * 功能：滑动条点击回调函数
 * *****************************/
void MainWindow::on_BPlaySlider_sliderPressed()
{
    BPlaySliderPress = true;
    BLOG("111on_BPlaySlider_sliderPressed");

    return;
}

/********************************
 * void MainWindow::on_BPlaySlider_sliderPressed()
 * 功能：滑动条释放回调函数
 * *****************************/
void MainWindow::on_BPlaySlider_sliderReleased()
{
    BPlaySliderPress = false;
    BLOG("222on_BPlaySlider_sliderReleased  %d", ui->BPlaySlider->value());
    Bffmpeg::GetInstance()->ResetTime(ui->BPlaySlider->value());
    return;
}

/********************************
 * void MainWindow::on_Bopenfile_btn_clicked()
 * 功能：打开文件按钮点击回调函数
 * *****************************/
void MainWindow::on_Bopenfile_btn_clicked()
{
    QString FilePath = QFileDialog::getOpenFileName(this, QString("媒体文件"), QString("."), QString("视频文件(*.mp4 *.flv *.avi);;所有文件(*.*)"));
    if (FilePath.isEmpty()) {
        BLOG("Media File empty");
        return;
    }

    if (0 != Bffmpeg::GetInstance()->BLoadMediaFile(FilePath)) {
        /* 媒体文件获取失败 */
        BLOG("Media File illegal");
        QMessageBox::information(this, QString("错误"), QString("媒体文件非法！"));
        return;
    }
    BLOG("Media File load");

    /* 初始化画布 */
    ui->BPlayopenGLWidget->InitMedia();
    BLOG("InitMedia");

    /* 更新按钮状态(开启) */
    OnOffBbutton->SetButtonStatus(true);
    BLOG("SetButtonStatus");
    return;
}
