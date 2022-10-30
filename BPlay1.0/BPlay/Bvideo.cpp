/********************************
 * Bvideo.cpp
 * 功能：视频处理cpp
 * *****************************/

#include "Bvideo.h"
#include "Bffmpeg.h"

QAtomicPointer<Bvideo> Bvideo::Instance = NULL;
QMutex Bvideo::mtx;

/********************************
 * Bvideo::Bvideo()
 * 功能：Bvideo构造 
 * *****************************/
Bvideo::Bvideo()
{
}

/********************************
 * Bvideo* Bvideo::GetInstance()
 * 功能：获取单例实例对象
 *      返回：单例实例对象
 * *****************************/
Bvideo* Bvideo::GetInstance()
{
    if (Instance.testAndSetOrdered(NULL, NULL)) {
        QMutexLocker mtxlocker(&mtx);
        Instance.testAndSetOrdered(NULL, new Bvideo);
    }
    
    return Instance;
}

/********************************
 * void Bvideo::run()
 * 功能：视频解码线程
 * *****************************/
void Bvideo::run()
{
    while (videorun)
    {
        AVFrame* frame;
        
        {
            QMutexLocker Locker(&Bffmpeg::GetInstance()->GetVideoQue().mtx);
            if (Bffmpeg::GetInstance()->GetVideoQue().que.size() == 0) {
                msleep(1);
                continue;
            }

            AVPacket *pkt = Bffmpeg::GetInstance()->GetVideoQue().que.front();
            if (pkt->pts * av_q2d(Bffmpeg::GetInstance()->GetFormatContext()->streams[Bffmpeg::GetInstance()->GetVideoIndex()]->time_base) > Bffmpeg::GetInstance()->GetAudioPts()) {
                msleep(1);
                continue;
            } else {
                /* 视频流编码数据出队列+解码 */
                frame = Bffmpeg::GetInstance()->Decode(pkt);
                av_packet_unref(pkt); 
                av_packet_free(&pkt);
                av_free(pkt);
                Bffmpeg::GetInstance()->GetVideoQue().que.pop_front();
                if (frame == NULL) {
                    continue;
                }
            }
        }

        /* 解码帧数据入RGB队列 */
        QMutexLocker FrameLocker(&frameque.mtx);
        frameque.frame.push_back(frame);
        
        msleep(1);
    }
    
    return;
}

/********************************
 * void Bvideo::start()
 * 功能：开始播放视频
 * *****************************/
void Bvideo::start()
{
    videorun = true;
    QThread::start();
    return;
}

/********************************
 * void Bvideo::stop()
 * 功能：停止播放视频
 * *****************************/
void Bvideo::stop()
{
    videorun = false;
    return;
}

/********************************
 * FrameQueue& Bvideo::GetFrameque()
 * 功能：获取解码数据队列
 * *****************************/
FrameQueue& Bvideo::GetFrameque()
{
    return this->frameque;
}

/********************************
 * void Baudio::AudioReset()
 * 功能：清除视频相关配置
 * *****************************/
void Bvideo::VideoReset()
{
    videorun = false;
    Bffmpeg::GetInstance()->GetVideoQue().que.clear();
    return;
}

