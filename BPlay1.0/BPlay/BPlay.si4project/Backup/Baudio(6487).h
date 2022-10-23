#ifndef BAUDIO_H
#define BAUDIO_H

#include <QThread>
#include <QMutex>
#include "Bffmpeg.h"
class Baudio : public QThread
{
public:
    static Baudio* GetInstance();
    void run();
    void start();
private:
    Baudio();
    static QAtomicPointer<Baudio> Instance;
    static QMutex mtx;

    bool audiorun;                              /* 音频解码线程是否启动 */
};


#endif // BAUDIO_H
