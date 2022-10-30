/********************************
 * Bffmpeg.h
 * 功能：音视频主控头文件
 * *****************************/

#ifndef BFFMPEG_H
#define BFFMPEG_H

#include <QThread>
#include <QAtomicPointer>
#include <QMutex>
#include <QMutexLocker>
#include "Bpublic.h"
#include "Bvideo.h"
#include "Baudio.h"
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

typedef struct {
    QQueue<AVPacket *> que;
    QMutex mtx;
} BQueue;

class Bffmpeg : QThread
{
public:
    static Bffmpeg* GetInstance();              /* 获取单例实例对象 */
    int BLoadMediaFile(QString FilePath);       /* 媒体文件加载 */
    void StartPlay();                           /* 开始播放媒体文件 */
    void StopPlay();                            /* 暂停播放媒体文件 */
    BQueue& GetVideoQue();                      /* 获取视频流队列 */
    BQueue& GetAudioQue();                      /* 获取音频流队列 */
    AVFrame* Decode(AVPacket *pkt);             /* 音视频数据解码 */
    AVFormatContext* GetFormatContext();        /* 获取媒体流上下文 */
    int GetVideoIndex();                        /* 获取视频流索引 */
    int GetAudioIndex();                        /* 获取音频流索引 */
    SwrContext* GetSwrContext();                /* 获取音频重采样上下文 */
    double GetAudioPts();                       /* 获取音频Pts */
    void SetAudioPts(double NewAudioPts);       /* 更新音频Pts */
    void Reset();                               /* 释放上一个媒体的所有资源 */
    double GetTimeAll();                        /* 获取视频总时长 */
    void ResetTime(int Time);                   /* 重设视频播放时间(进度) */
    void ClearAudioQue();                       /* 清空音频流队列 */
    void ClearVideoQue();                       /* 清空视频流队列 */

private:
    Bffmpeg();                                  /* Bffmpeg构造 */
    void run();                                 /* 音视频主控线程 */
    void start();                               /* 启动音视频主控线程 */

    static QAtomicPointer<Bffmpeg> Instance;    /* 单例实例对象 */
    static QMutex ffmpegMutex;                  /* Bffmpeg对象锁 */
    AVFormatContext *FormatContext = NULL;      /* 媒体流上下文(解码器：FormatContext->streams[index].codec) */
    int Video_index;                            /* 视频流索引 */
    int Audio_index;                            /* 音频流索引 */
    bool ffmpegrun;                             /* 音视频主控线程是否启动 */

    BQueue VideoQue;                            /* 视频流队列 */
    BQueue AudioQue;                            /* 音频流队列 */

    SwrContext *Swr = NULL;                     /* 音频重采样上下文 */

    double AudioPts = 0;                        /* 音频Pts */
    double TimeAll = 0;                         /* 总时长(单位: 秒) */
};

#endif // BFFMPEG_H
