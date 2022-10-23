/********************************
 * Bvideo.h
 * 功能：视频处理头文件
 * *****************************/

#ifndef BVIDEO_H
#define BVIDEO_H

#include "Bpublic.h"
#include "Bffmpeg.h"
#include <QAtomicPointer>
#include <QMutex>
#include <QThread>
#include <QQueue>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavformat/version.h>
    #include <libavdevice/avdevice.h>
    #include <libavutil/time.h>
    #include <libavutil/mathematics.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
}

typedef struct{
    QQueue<AVFrame*> frame;
    QMutex mtx;
} FrameQueue;

class Bvideo : public QThread
{
public:
    static Bvideo* GetInstance();
    void run();
    void start();
    void stop();
    FrameQueue& GetFrameque();
    void VideoReset();
private:
    Bvideo();
    static QAtomicPointer<Bvideo> Instance;
    static QMutex mtx;

    FrameQueue frameque;                        /* 解码数据队列 */
    bool videorun;                              /* 视频解码线程是否启动 */
};

#endif // BVIDEO_H
