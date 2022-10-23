#ifndef BBUTTON_H
#define BBUTTON_H

#include <QPushButton>
#include <QString>
#include <Bpublic.h>
#include "Bffmpeg.h"

class Bbutton : public QPushButton
{
public:
    Bbutton(QString Image1, QString Image2);
    void SetButtonStatus(bool status);          /* 设置按钮状态 */
    void LinkFfmpeg();
private:
    QString BbuttonImage1;                      /* 开启位图 */
    QString BbuttonImage2;                      /* 关闭位图 */
    bool Status = false;                        /* 初始化为暂停状态 */
    void mouseReleaseEvent(QMouseEvent *e);     /* 鼠标释放事件 */
};

#endif // BBUTTON_H
