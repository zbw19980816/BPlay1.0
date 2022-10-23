#include "Baudio.h"

Baudio::Baudio()
{

}

Baudio* Baudio::GetInstance()
{
  /*  if (Instance.testAndSetOrdered(NULL, NULL)) {
        QMutexLocker mtxlocker(&mtx);
        Instance.testAndSetOrdered(NULL, new Baudio);
    }*/
    return Instance;

}

void Baudio::start()
{
    audiorun = true;
    QThread::start();
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
        if (Bffmpeg::GetInstance()->GetAudioQue().que.size() == 0) {
            //BLOG("run %d", ++c);
            msleep(10);
            continue;
        } else {
            /* 音频流编码数据出队列 */
            QMutexLocker Locker(&Bffmpeg::GetInstance()->GetAudioQue().mtx);
            AVPacket pkt = Bffmpeg::GetInstance()->GetAudioQue().que.front();
            Bffmpeg::GetInstance()->GetAudioQue().que.pop_front();
        }

        /* 解码 */
        AVFrame* frame = Bffmpeg::GetInstance()->Decode(pkt);
        av_packet_unref(&pkt); 
        if (frame == NULL) {
            continue;
        }
        
        /* 音频重采样 */
        AVFormatContext *FormatContext = Bffmpeg::GetInstance()->GetFormatContext();
        AVCodecContext *CodecContext = FormatContext->streams[Bffmpeg::GetInstance()->GetAudioIndex()]->codec;

        uint8_t *data[1];
            data[0] = (uint8_t *)out;
    int len =
        swr_convert(Bffmpeg::GetInstance()->GetSwrContext(),
                    data,
                    44100,
                    (const uint8_t **)frame->data,
                    frame->nb_samples);

    if (len <= 0)
    {
        return;
    }

    // out_buffer_size = len * ctx->channels *
    // av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    int datasize =  av_samples_get_buffer_size(NULL,
                                      CodecContext->channels,
                                      frame->nb_samples,
                                      AV_SAMPLE_FMT_S16,
                                      0);

        /* 播放 */
        //io->write(out, datasize);
        msleep(1);
    }
    return;
}

