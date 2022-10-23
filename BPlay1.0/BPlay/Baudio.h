/********************************
 * Baudio.h
 * 功能：音频处理头文件
 * *****************************/

#ifndef BAUDIO_H
#define BAUDIO_H

#include <QAudioOutput>
#include <QThread>
#include <QMutex>
#include "Bffmpeg.h"


class Baudio : public QThread
{
public:
    static Baudio* GetInstance();
    void run();
    void start();
    void stop();
    void OpenAudioOutput();
    void AudioReset();
private:
    Baudio();
    static QAtomicPointer<Baudio> Instance;
    static QMutex mtx;

    bool audiorun;                              /* 音频解码线程是否启动 */
    QIODevice *io = NULL;                       /* 音频设备 */
    QAudioOutput *output = NULL;                /* 音频输出 */
};


#endif // BAUDIO_H
