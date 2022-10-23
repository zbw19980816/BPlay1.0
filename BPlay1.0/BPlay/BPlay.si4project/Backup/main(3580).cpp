/********************************
 * main.cpp
 * 功能：BPlay主程序入口
 * *****************************/

#include "mainwindow.h"
#include <QApplication>
#include "Bpublic.h"

/********************************
 * int main(int argc, char *argv[])
 * 功能：主函数
 * *****************************/
int main(int argc, char *argv[])
{
    BLOG("hello");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
