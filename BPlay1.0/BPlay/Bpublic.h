/********************************
 * Bpublic.h
 * 功能：公共接口头文件
 * *****************************/

#ifndef BPUBLIC_H
#define BPUBLIC_H

#include <QDateTime>
#include <QString>


#define BLOG qDebug("[%s][%d][%s][%s]", __FILE__, __LINE__, QDateTime::currentDateTime().toString("yyyy-MM-dd").toLatin1().data(), QDateTime::currentDateTime().toString("hh:mm:ss.zzz ").toLatin1().data()); qDebug

class Bpublic
{
public:
    Bpublic();
};

#endif // BPUBLIC_H
