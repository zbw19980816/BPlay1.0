#include "Baudio.h"

QAtomicPointer<Baudio> Baudio::Instance= NULL;
QMutex Baudio::mtx;

Baudio::Baudio()
{

}

Baudio* Baudio::GetInstance()
{
    if (Instance.testAndSetOrdered(NULL, NULL)) {
        QMutexLocker mtxlocker(&mtx);
        Instance.testAndSetOrdered(NULL, new Baudio);
    }
    return Instance;

}

/********************************
 * void Baudio::OpenAudioOutput()
 * 功能：初始化设置音频设备参数
 * *****************************/
void Baudio::OpenAudioOutput()
{
    QAudioFormat  fmt;                            // 设置音频输出格式
    fmt.setSampleRate(44100);                     // 1秒的音频采样率
    fmt.setSampleSize(16);                        // 声音样本的大小
    fmt.setChannelCount(Bffmpeg::GetInstance()->GetFormatContext()->streams[Bffmpeg::GetInstance()->GetAudioIndex()]->codec->channels);                 // 声道
    fmt.setCodec("audio/pcm");                    // 解码格式
    fmt.setByteOrder(QAudioFormat::LittleEndian);
    fmt.setSampleType(QAudioFormat::UnSignedInt); // 设置音频类型
    output = new QAudioOutput(fmt);
    io = output->start();                         // 播放开始

    return;
}

/********************************
 * void Baudio::AudioReset()
 * 功能：清除音频相关配置
 * *****************************/
void Baudio::AudioReset()
{
    audiorun = false;
    if (output) {
        delete output;
        output = NULL;
    }
    
    io = NULL;

    Bffmpeg::GetInstance()->GetAudioQue().que.clear();
    return;
}

void Baudio::start()
{
    audiorun = true;
    QThread::start();
    return;
}

void Baudio::stop()
{
    audiorun = false;
    return;
}
/********************************
 * void BAudio::run()
 * 功能：音频解码/播放线程
 * *****************************/
void Baudio::run()
{
    char out[20000] = { 0 };

    while (audiorun)
    {
        AVPacket pkt;
        memset(&pkt , 0, sizeof(pkt));

        if ((Bffmpeg::GetInstance()->GetAudioQue().que.size() == 0) || (output->bytesFree() < output->periodSize())) {
            msleep(1);
            continue;
        } else {
            /* 音频流编码数据出队列 */
            QMutexLocker Locker(&Bffmpeg::GetInstance()->GetAudioQue().mtx);
            pkt = Bffmpeg::GetInstance()->GetAudioQue().que.front();
            Bffmpeg::GetInstance()->GetAudioQue().que.pop_front();

            /* 更新音频播放时间 */
            Bffmpeg::GetInstance()->SetAudioPts(pkt.pts *
                    av_q2d(Bffmpeg::GetInstance()->GetFormatContext()->streams[Bffmpeg::GetInstance()->GetAudioIndex()]->time_base));
           // BLOG("Bffmpeg::GetInstance()->GetAudioPts() %f", Bffmpeg::GetInstance()->GetAudioPts());
        }
        
        
        /* 解码 */
        AVFrame* frame = Bffmpeg::GetInstance()->Decode(&pkt);
        av_packet_unref(&pkt); 
        if (frame == NULL) {
            continue;
        }
        //continue;
        /* 音频重采样 */
        AVFormatContext *FormatContext = Bffmpeg::GetInstance()->GetFormatContext();
        AVCodecContext *CodecContext = FormatContext->streams[Bffmpeg::GetInstance()->GetAudioIndex()]->codec;

        uint8_t *data[1];
        data[0] = (uint8_t *)out;
        int len = swr_convert(Bffmpeg::GetInstance()->GetSwrContext(), data, sizeof(out), (const uint8_t **)frame->data, frame->nb_samples);

        if (len <= 0)
        {
            return;
        }

        // out_buffer_size = len * ctx->channels *
        // av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
        int datasize =  av_samples_get_buffer_size(NULL, CodecContext->channels, frame->nb_samples, AV_SAMPLE_FMT_S16, 0);

        /* 播放 */
        if (io) {
            io->write(out, datasize);
        }
    }
    
    return;
}

