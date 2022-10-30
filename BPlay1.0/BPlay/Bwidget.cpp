/********************************
 * Bwidget.cpp
 * 功能：视频显示控件cpp
 * *****************************/

#include "Bwidget.h"

/********************************
 * Bwidget::Bwidget(QWidget *parent) : QOpenGLWidget(parent)
 * 功能：Bwidget构造 
 * *****************************/
Bwidget::Bwidget(QWidget *parent) : QOpenGLWidget(parent)
{
    TimerID = startTimer(10);
}

/********************************
 * void Bwidget::timerEvent(QTimerEvent *event)
 * 功能：刷新widget
 * *****************************/
void Bwidget::timerEvent(QTimerEvent *event)
{
    /* update会触发paintEvent */
    this->update();
}

/********************************
 * void Bwidget::paintEvent(QPaintEvent *event)
 * 功能：绘制一帧图像
 * *****************************/
void Bwidget::paintEvent(QPaintEvent *event)
{
    if (Bvideo::GetInstance()->GetFrameque().frame.size() == 0) {
        return;
    }

    /* 解码数据出队列 */
    QMutexLocker Locker(&Bvideo::GetInstance()->GetFrameque().mtx);
    AVFrame* frame = Bvideo::GetInstance()->GetFrameque().frame.front();

    SwsContext *context = NULL;
    AVCodecContext *codec = Bffmpeg::GetInstance()->GetFormatContext()->streams[Bffmpeg::GetInstance()->GetVideoIndex()]->codec;

    /* 解码数据转RGB32 */
    //AV_PIX_FMT_BGRA ?
    context = sws_getCachedContext(context,                                         /* 转化上下文结构体 */
                                   codec->width, codec->height, codec->pix_fmt,     /* 源图像格式和宽高 */
                                   Width, Height, AV_PIX_FMT_BGRA,                  /* 目标图像格式和宽高 */
                                   SWS_BICUBIC, NULL, NULL, NULL);

    if (NULL == context) {
        BLOG("sws_getCachedContext fail");
        return;
    }

    uint8_t *data[AV_NUM_DATA_POINTERS] = { 0 };
    data[0] = (uint8_t *)Image->bits();
    int linesize[AV_NUM_DATA_POINTERS] = { 0 };
    linesize[0] = Width * 4;                /* 实际显示一行的宽度，32位4个字节 */
    sws_scale(context, frame->data,         /* 源视频解码数据 */
              frame->linesize,              /* 每行大小 */
              0,                            /* 用不到 */
              frame->height,                /* 图像高度 */
              data,                         /* 输出的每个通道数据指针 */
              linesize);                    /* 每个通道行字节数 */

    /* 释放解码数据，出队列 */
    sws_freeContext(context);
    av_frame_unref(frame);
    av_frame_free(&frame);
    Bvideo::GetInstance()->GetFrameque().frame.pop_front();

    /* 绘制图像 */
    QPainter painter;
    painter.begin(this);
    painter.drawImage(QPoint(X, Y), *Image);
    painter.end();

    return;
}

/********************************
 * void Bwidget::InitMedia()
 * 功能：初始化媒体画布相关信息
 * *****************************/
void Bwidget::InitMedia()
{
    int WidgetWidth = width();          /* Bwidget宽 */
    int WidgetHeight = height();        /* Bwidget高 */
    int VideoIndex = Bffmpeg::GetInstance()->GetVideoIndex();
    int MediaWidth = Bffmpeg::GetInstance()->GetFormatContext()->streams[VideoIndex]->codec->width;     /* 视频宽 */
    int MediaHeight = Bffmpeg::GetInstance()->GetFormatContext()->streams[VideoIndex]->codec->height;   /* 视频高 */
    
    BLOG("WidgetWidth[%d],WidgetHeight[%d], MediaWidth[%d], MediaHeight[%d]", WidgetWidth, WidgetHeight, MediaWidth, MediaHeight);

    if (((float)WidgetWidth / (float)WidgetHeight) > ((float)MediaWidth / (float)MediaHeight)) {
        /* Bwidget宽高比大于视频,左右需要有黑边 */
        Width = (int)((float)WidgetHeight * (float)MediaWidth / (float)MediaHeight);
        Height = WidgetHeight;
        X = (WidgetWidth - Width) / 2;
        Y = 0;
    } else {
        /* Bwidget宽高比小于视频,上下需要有黑边 */
        Width = WidgetWidth;
        Height = ((float)WidgetWidth * (float)MediaHeight / (float)MediaWidth);
        X = 0;
        Y = (WidgetHeight - Height) / 2;;
    }
    
    BLOG("Width[%d], Height[%d], X[%d], Y[%d]", Width, Height, X, Y);
    if (ImageData) {
        delete ImageData;
    }
    
    if (Image) {
        delete Image;
    }

    /* 初始化视频区域 */
    ImageData = new uchar[Width * Height * 4];
    Image = new QImage(ImageData, Width, Height, QImage::Format_RGB32);
    
    /* 初始背景为黑色 */
    QPainter pt;
    pt.begin(this);
    QColor color(0, 0, 0);
    pt.fillRect(0, 0, WidgetWidth, WidgetHeight, color);
    pt.end();

    return;
}
