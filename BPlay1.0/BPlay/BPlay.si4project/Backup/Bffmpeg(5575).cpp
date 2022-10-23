/********************************
 * Bffmpeg.cpp
 * 功能：音视频主控cpp
 * *****************************/

#include "Bffmpeg.h"

QAtomicPointer<Bffmpeg> Bffmpeg::Instance = NULL;
QMutex Bffmpeg::ffmpegMutex;

Bffmpeg::Bffmpeg()
{
    ;
}

/********************************
 * Bffmpeg* Bffmpeg::GetInstance()
 * 功能：获取单例实例对象
 *      返回：单例实例对象
 * *****************************/
Bffmpeg* Bffmpeg::GetInstance()
{
    /*
     testAndSetOrdered实现逻辑:
        testAndSetOrdered(expectedValue, newValue)
        currentValue = _instance;   //currentValue为QAtomicPointer<Bffmpeg>模板定义的指针变量
        if (currentValue == expectedValue) {
            currentValue = newValue;
            return true;
        }
        return false;
    */
    if (Instance.testAndSetOrdered(NULL, NULL)) {
        QMutexLocker lock(&ffmpegMutex);
        Instance.testAndSetOrdered(NULL, new Bffmpeg);
    }

    return Instance;
}

/********************************
 * int Bffmpeg::BLoadMediaFile(QString FilePath)
 * 功能：媒体文件加载
 *      成功返回：0
 *      失败返回：errcode
 * *****************************/
int Bffmpeg::BLoadMediaFile(QString FilePath)
{
    int Ret = 0;
    char errbuf[128] = {0};


    /* 释放上一段视频所有资源 */
    Reset();

    if (NULL != FormatContext) {
        avformat_close_input(&FormatContext);
    }

    FormatContext = avformat_alloc_context();

    /* 打开多媒体数据并且获得一些相关的信息 */
    Ret = avformat_open_input(&FormatContext, FilePath.toLocal8Bit().data(), NULL, NULL);
    if (Ret != 0) {
        av_strerror(Ret, errbuf, sizeof(errbuf));
        BLOG("avformat_open_input fail, Ret[%d], Err[%s]", Ret, errbuf);
        return Ret;
    }

    /* 读取一部分音视频数据并且获得一些相关的信息 */
    Ret = avformat_find_stream_info(FormatContext, NULL);
    if (Ret < 0) {
        av_strerror(Ret, errbuf, sizeof(errbuf));
        BLOG("avformat_open_input fail, Ret[%d], Err[%s]", Ret, errbuf);
        avformat_close_input(&FormatContext);
        return Ret;
    }

    /* 查找视频流索引 */
    Video_index  = av_find_best_stream(FormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if ((AVERROR_STREAM_NOT_FOUND == Video_index) || (AVERROR_DECODER_NOT_FOUND== Video_index)) {
        BLOG("av_find_best_stream fail, Video_index[%d]", Video_index);
        avformat_close_input(&FormatContext);
        return -1;
    }

    /* 找视频解码器 */
    AVCodec *Codec = avcodec_find_decoder(FormatContext->streams[Video_index]->codecpar->codec_id);
    if (NULL == Codec) {
        BLOG("avcodec_find_decoder fail");
        avformat_close_input(&FormatContext);
        return -1;
    }

    /* 打开解码器,初始化FormatContext内部解码器上下文,FormatContext->streams[Video_index]->codec内存在avformat_open_input调用时已分配 */
    Ret = avcodec_open2(FormatContext->streams[Video_index]->codec, Codec, NULL);
    if (Ret < 0) {
        av_strerror(Ret, errbuf, sizeof(errbuf));
        BLOG("avcodec_open2 fail, Ret[%d], Err[%s]", Ret, errbuf);
        avformat_close_input(&FormatContext);
        return Ret;
    }

    /* 查找音频流索引 */
    Audio_index  = av_find_best_stream(FormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if ((AVERROR_STREAM_NOT_FOUND == Audio_index) || (AVERROR_DECODER_NOT_FOUND== Audio_index)) {
        BLOG("av_find_best_stream fail, Audio_index[%d]", Audio_index);
        avformat_close_input(&FormatContext);
        return -1;
    }

    /* 找音频解码器 */
    Codec = avcodec_find_decoder(FormatContext->streams[Audio_index]->codecpar->codec_id);
    if (NULL == Codec) {
        BLOG("avcodec_find_decoder fail");
        avformat_close_input(&FormatContext);
        return -1;
    }

    /* 打开解码器,初始化FormatContext内部解码器上下文,FormatContext->streams[Audio_index]->codec内存在avformat_open_input调用时已分配 */
    Ret = avcodec_open2(FormatContext->streams[Audio_index]->codec, Codec, NULL);
    if (Ret < 0) {
        av_strerror(Ret, errbuf, sizeof(errbuf));
        BLOG("avcodec_open2 fail, Ret[%d], Err[%s]", Ret, errbuf);
        avformat_close_input(&FormatContext);
        return Ret;
    }

    TimeAll = (float)FormatContext->duration / (AV_TIME_BASE);
    BLOG("open codec success, width=%d,height=%d,TimeAll=%f\n",
            FormatContext->streams[Video_index]->codec->width, FormatContext->streams[Video_index]->codec->height, TimeAll);

    Swr = swr_alloc();   //未释放？？？swr_free(&aCtx);
    AVCodecContext *AudioCodec = FormatContext->streams[Audio_index]->codec;
    swr_alloc_set_opts(Swr, AudioCodec->channel_layout, AV_SAMPLE_FMT_S16, 44100,                                 /* 目标格式 */
                           AudioCodec->channel_layout, AudioCodec->sample_fmt, AudioCodec->sample_rate, 0, 0);    /* 源格式 */
    swr_init(Swr);

    /* 开启音频外设 */
    Baudio::GetInstance()->OpenAudioOutput();

    return 0;
}

/********************************
 * void Bffmpeg::Reset()
 * 功能：释放上一个媒体的所有资源
 * *****************************/
void Bffmpeg::Reset()
{
    BLOG("111111");
    StopPlay();
    BLOG("111111");
    if (FormatContext) {
        avformat_close_input(&FormatContext);  /* 关闭媒体流上下文 */
    }
    BLOG("111111");
    if (Swr) {
        swr_free(&Swr);
    }
    BLOG("111111");
    Baudio::GetInstance()->AudioReset();
    BLOG("111111");
    Bvideo::GetInstance()->VideoReset();

   
    
    
    return;
}

/********************************
 * double Bffmpeg::GetTimeAll()
 * 功能：重设视频播放时间(进度)
 * *****************************/
void Bffmpeg::ResetTime(int Time)
{
    StopPlay();
    Bffmpeg::GetInstance()->GetAudioQue().que.clear();
    Bffmpeg::GetInstance()->GetVideoQue().que.clear();

   // QMutexLocker locker(&ffmpegMutex);

    if (!FormatContext)
    {
        BLOG("seek error");
        return;
    }

    /* 1、找到前面的关键帧 */
    av_seek_frame(FormatContext, -1, (int64_t)Time * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);

    /* 2、向后找播放点 */
    AVPacket pkt;
    while (1) {
        //msleep(1);
        av_read_frame(FormatContext, &pkt);
        if (pkt.stream_index == Video_index) {
            BLOG("--------%d < %f   ", Time, pkt.pts * av_q2d(FormatContext->streams[Video_index]->time_base));
            if (Time < pkt.pts * av_q2d(FormatContext->streams[Video_index]->time_base)) {
                break;
            }

            /* 解码 */

            AVFrame* frame = Decode(&pkt);

            av_packet_unref(&pkt);

            if (frame == NULL) {
                continue;
            }

            //continue;
            /* 解码帧数据入RGB队列 */
            //QMutexLocker FrameLocker(&Bvideo::GetInstance()->GetFrameque().mtx);
            Bvideo::GetInstance()->GetFrameque().frame.push_back(frame);
        }
    }

    StartPlay();
    return;
}

/********************************
 * double Bffmpeg::GetTimeAll()
 * 功能：获取视频总时长
 * *****************************/
double Bffmpeg::GetTimeAll()
{
    return TimeAll;
}

/********************************
 * void Bffmpeg::StartPlay()
 * 功能：开始播放媒体文件
 * *****************************/
void Bffmpeg::StartPlay()
{
    this->start();
    Bvideo::GetInstance()->start();
    Baudio::GetInstance()->start();
    return;
}

/********************************
 * void Bffmpeg::StopPlay()
 * 功能：暂停播放媒体文件
 * *****************************/
void Bffmpeg::StopPlay()
{
    ffmpegrun = false;
    Bvideo::GetInstance()->stop();
    Baudio::GetInstance()->stop();
    return;
}

/********************************
 * void Bffmpeg::start()
 * 功能：启动音视频主控线程
 * *****************************/
void Bffmpeg::start()
{
    ffmpegrun = true;
    QThread::start();
    return;
}

/********************************
 * void Bffmpeg::run()
 * 功能：音视频主控线程
 * *****************************/
void Bffmpeg::run()
{
    int Ret = 0;
    AVPacket pkt;
    int c = 0;
    BLOG("Bffmpeg::run >>>>>>>>>>>>");
    while (ffmpegrun) {
        //BLOG("run AudioQue.que.size() %d    VideoQue.que.size() %d", AudioQue.que.size(), VideoQue.que.size());
        if (AudioQue.que.size() > 10 || VideoQue.que.size() > 10) {
            //BLOG("run %d", ++c);
            msleep(1);
            continue;
        }

        memset(&pkt , 0, sizeof(pkt));
        Ret = av_read_frame(FormatContext, &pkt);
        //BLOG("av_read_frame Ret:%d  pkt.stream_index:%d", Ret, pkt.stream_index);
        if (0 != Ret) {
            BLOG("av_read_frame fail, Ret[%d]", Ret);
            return;
        }

        if (pkt.stream_index == Audio_index) {
            /* 音频流入队列 */
            //BLOG("音频流 %d  pts %d  timebase[%f]", c, pkt.pts, av_q2d(FormatContext->streams[Audio_index]->time_base));
            QMutexLocker locker(&AudioQue.mtx);
            AudioQue.que.push_back(pkt);
        } else if (pkt.stream_index == Video_index) {
            /* 视频流入队列 */
            //BLOG("视频流 %d  pts %d  timebase[%f]", c, pkt.pts, av_q2d(FormatContext->streams[Video_index]->time_base));
            QMutexLocker locker(&VideoQue.mtx);
            VideoQue.que.push_back(pkt);
        }

        msleep(1);
    }

    
    BLOG("Bffmpeg::run <<<<<<<<<<<<<<");
    return;
}

/********************************
 * BQueue Bffmpeg::GetVideoQue()
 * 功能：获取视频流队列
 * *****************************/
BQueue& Bffmpeg::GetVideoQue()
{
    return this->VideoQue;
}

/********************************
 * BQueue Bffmpeg::GetAudioQue()
 * 功能：获取音频流队列
 * *****************************/
BQueue& Bffmpeg::GetAudioQue()
{
    return this->AudioQue;
}

/********************************
 * AVFormatContext* Bffmpeg::GetFormatContext()
 * 功能：获取媒体流上下文
 * *****************************/
AVFormatContext* Bffmpeg::GetFormatContext()
{
    return this->FormatContext;
}

/********************************
 * int Bffmpeg::GetVideoIndex()
 * 功能：获取视频流索引
 * *****************************/
int Bffmpeg::GetVideoIndex()
{
    return this->Video_index;
}

/********************************
 * int Bffmpeg::GetAudioIndex()
 * 功能：获取音频流索引
 * *****************************/
int Bffmpeg::GetAudioIndex()
{
    return this->Audio_index;
}

/********************************
 * SwrContext* Bffmpeg::GetSwrContext()
 * 功能：获取音频重采样上下文
 * *****************************/
SwrContext* Bffmpeg::GetSwrContext()
{
    return this->Swr;
}

/********************************
 * double Bffmpeg::GetAudioPts()
 * 功能：获取音频Pts
 * *****************************/
double Bffmpeg::GetAudioPts()
{
    return this->AudioPts;
}

/********************************
 * double Bffmpeg::SetAudioPts()
 * 功能：更新音频Pts
 * *****************************/
void Bffmpeg::SetAudioPts(double NewAudioPts)
{
    this->AudioPts = NewAudioPts;
}

/********************************
 * AVFrame* Bffmpeg::Decode(AVPacket &pkt)
 * 功能：音视频解码
 * *****************************/
AVFrame* Bffmpeg::Decode(AVPacket *pkt)
{
    QMutexLocker lock(&ffmpegMutex);

    int Ret = 0;
    char errbuf[128] = {0};
    AVFrame *frame = av_frame_alloc();   /* 存在反复申请释放（转RGB后释放），后续优化 */

    /* 发送数据到ffmepg，放到解码队列中 */
    Ret = avcodec_send_packet(FormatContext->streams[pkt->stream_index]->codec, pkt);
    if (0 != Ret) {
        av_strerror(Ret, errbuf, sizeof(errbuf));
        BLOG("avcodec_send_packet fail, Ret[%d], Err[%s]", Ret, errbuf);
        ///sleep(50);
        return NULL;
    }

    /* 从解码队列中取出frame */
    Ret = avcodec_receive_frame(FormatContext->streams[pkt->stream_index]->codec, frame);
    if (0 != Ret) {
        av_strerror(Ret, errbuf, sizeof(errbuf));
        BLOG("avcodec_receive_frame fail, Ret[%d], Err[%s]", Ret, errbuf);
        return NULL;
    }

    return frame;
}

