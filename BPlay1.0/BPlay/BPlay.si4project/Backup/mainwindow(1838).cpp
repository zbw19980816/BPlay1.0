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

    /* 开始/停止播放按钮 */
    OnOffBbutton = new Bbutton(":/Image/on.png", ":/Image/off.png");
    OnOffBbutton->setParent(this);
    OnOffBbutton->move(50, 530);
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

    /* 初始化画布 */
    ui->BPlayopenGLWidget->InitMedia();

    /* 成功打开一个媒体文件,开始播放 */
    Bffmpeg::GetInstance()->StartPlay();

    return;
}
