/********************************
 * Bbutton.cpp
 * 功能：开始/暂停自定义按钮cpp
 * *****************************/

#include "Bbutton.h"

/********************************
 * Bbutton::Bbutton(QString Image1, QString Image2)
 * 功能：开始/暂停自定义按钮构造
 *      Image1:开启
 *      Image2:暂停
 * *****************************/
Bbutton::Bbutton(QString Image1, QString Image2)
{
    BbuttonImage1 = Image1;
    BbuttonImage2 = Image2;

    QPixmap Pix;
    bool ret = Pix.load(Image2);  /* 默认暂停 */
    if (!ret) {
        BLOG( "Load Image1 fail");
        return;
    }

    this->setFixedSize(Pix.width(), Pix.height());  /* 将按钮的大小设定成图片的大小 */
    this->setStyleSheet("QPushButton{border:0px;}");  /* 设置不规则图片样式: 边界0像素 */
    this->setIcon(Pix);   /* 设置图标 */
    this->setIconSize(QSize(Pix.width(), Pix.height()));
}

/********************************
 * void Bbutton::mouseReleaseEvent(QMouseEvent *e)
 * 功能：鼠标释放事件回调
 * *****************************/
void Bbutton::mouseReleaseEvent(QMouseEvent *e)
{
    QPixmap Pix;
    QString Image;

    Status = !Status;
    Image = Status ? BbuttonImage1 : BbuttonImage2;

    bool ret = Pix.load(Image);
    if (!ret) {
        BLOG( "Load Image1 fail");
        return;
    }
    
    /* 设置图标 */
    this->setIcon(Pix);

    /* 联动视频开启/暂停 */
    LinkFfmpeg();
    
    return;
}

/********************************
 * void Bbutton::LinkFfmpeg()
 * 功能：开启/暂停视频
 * *****************************/
void Bbutton::LinkFfmpeg()
{
    if (Status) {
        Bffmpeg::GetInstance()->StartPlay();
    } else {
        Bffmpeg::GetInstance()->StopPlay();
    }
    
    return;
}

/********************************
 * void Bbutton::SetButtonStatus(bool status)
 * 功能：设置按钮状态(媒体加载成功后设置状态为开始)
 * *****************************/
void Bbutton::SetButtonStatus(bool status)
{
    QPixmap Pix;
    QString Image;

    Status = status;
    Image = Status ? BbuttonImage1 : BbuttonImage2;

    bool ret = Pix.load(Image);
    if (!ret) {
        BLOG( "Load Image1 fail");
        return;
    }

    this->setIcon(Pix);

    /* 联动视频开启/暂停 */
    LinkFfmpeg();

    return;
}

