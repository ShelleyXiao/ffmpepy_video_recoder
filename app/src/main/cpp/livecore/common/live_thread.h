//
// Created by ShaudXiao on 2018/8/2.
//

#ifndef ANDROIDFFMPEGRECODER_LIVE_THREAD_H
#define ANDROIDFFMPEGRECODER_LIVE_THREAD_H

#include <pthread.h>
#include "livecore/platform_dependent/platform_4_live_common.h"

class LiveThread {
public:

    LiveThread();
    ~LiveThread();

    void start();
    void startAsync();
    int wait();

    void waitOnNotify();
    void notify();

protected:
    bool mRunning;

    virtual void handleRun(void *ptr);

private:
    pthread_t mThread;
    pthread_mutex_t mLock;
    pthread_cond_t mCondition;

    static void *startThread(void *ptr);
};


#endif //ANDROIDFFMPEGRECODER_LIVE_THREAD_H
