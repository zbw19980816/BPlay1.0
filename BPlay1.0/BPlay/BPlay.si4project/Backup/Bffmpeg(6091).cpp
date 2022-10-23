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

    BLOG("open codec success, width=%d,height=%d!\n",
            FormatContext->streams[Video_index]->codec->width, FormatContext->streams[Video_index]->codec->height);

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
    while (ffmpegrun) {
        //BLOG("run %d", ++c);
        if (AudioQue.que.size() > 5 && VideoQue.que.size() > 5) {
            //BLOG("run %d", ++c);
            msleep(10);
            continue;
        }
        
        memset(&pkt , 0, sizeof(pkt));
        Ret = av_read_frame(FormatContext, &pkt);
        if (0 != Ret) {
            BLOG("av_read_frame fail, Ret[%d]", Ret);
            return;
        }

        if (pkt.stream_index == Audio_index) {
            /* 音频流入队列 */
           // BLOG("音频流 %d", c);

                    BLOG("音频流 %d  pts %d", c, pkt.pts);
            QMutexLocker locker(&AudioQue.mtx);
            AudioQue.que.push_back(pkt);
        } else if (pkt.stream_index == Video_index) {
            /* 视频流入队列 */
            //BLOG("视频流 %d", c);
            BLOG("视频流 %d  pts %d", c, pkt.pts);
            QMutexLocker locker(&VideoQue.mtx);
            VideoQue.que.push_back(pkt);
        }

        msleep(1);
    }
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

