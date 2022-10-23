/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.12
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QWidget>
#include <bwidget.h>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    Bwidget *BPlayopenGLWidget;
    QPushButton *Bopenfile_btn;
    QSlider *BPlaySlider;
    QLabel *MediaPlayTime;
    QLabel *MediaAllTime;
    QLabel *label;
    QMenuBar *menubar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        BPlayopenGLWidget = new Bwidget(centralwidget);
        BPlayopenGLWidget->setObjectName(QString::fromUtf8("BPlayopenGLWidget"));
        BPlayopenGLWidget->setGeometry(QRect(10, 15, 780, 500));
        Bopenfile_btn = new QPushButton(centralwidget);
        Bopenfile_btn->setObjectName(QString::fromUtf8("Bopenfile_btn"));
        Bopenfile_btn->setGeometry(QRect(530, 540, 81, 31));
        BPlaySlider = new QSlider(centralwidget);
        BPlaySlider->setObjectName(QString::fromUtf8("BPlaySlider"));
        BPlaySlider->setGeometry(QRect(180, 545, 281, 20));
        BPlaySlider->setOrientation(Qt::Horizontal);
        MediaPlayTime = new QLabel(centralwidget);
        MediaPlayTime->setObjectName(QString::fromUtf8("MediaPlayTime"));
        MediaPlayTime->setGeometry(QRect(120, 550, 54, 12));
        MediaAllTime = new QLabel(centralwidget);
        MediaAllTime->setObjectName(QString::fromUtf8("MediaAllTime"));
        MediaAllTime->setGeometry(QRect(470, 550, 54, 12));
        label = new QLabel(centralwidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(650, 530, 141, 51));
        label->setPixmap(QPixmap(QString::fromUtf8(":/BPlay/Image/BPlay.png")));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 22));
        MainWindow->setMenuBar(menubar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
        Bopenfile_btn->setText(QApplication::translate("MainWindow", "\346\211\223\345\274\200\346\226\207\344\273\266", nullptr));
        MediaPlayTime->setText(QApplication::translate("MainWindow", "TextLabel", nullptr));
        MediaAllTime->setText(QApplication::translate("MainWindow", "TextLabel", nullptr));
        label->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
