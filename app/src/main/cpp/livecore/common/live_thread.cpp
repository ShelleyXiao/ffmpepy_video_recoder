//
// Created by ShaudXiao on 2018/8/2.
//

#include "live_thread.h"

#define LOG_TAG "LiveThread"

LiveThread::LiveThread() {
    pthread_mutex_init(&mLock, NULL);
    pthread_cond_init(&mCondition, NULL);

}

LiveThread::~LiveThread() {

}

void LiveThread::start() {
    handleRun(NULL);
}

void LiveThread::startAsync() {
    pthread_create(&mThread, NULL, startThread, this);
}

int LiveThread::wait() {
    if (!mRunning) {
        return 0;
    }
    void *status;
    int ret = pthread_join(mThread, &status);
    LOGI("pthread_join thread return result is %d ", status);
    return (int) ret;
}

void LiveThread::waitOnNotify() {
    pthread_mutex_lock(&mLock);
    pthread_cond_wait(&mCondition, &mLock);
    pthread_mutex_unlock(&mLock);
}

void LiveThread::notify() {
    pthread_mutex_lock(&mLock);
    pthread_cond_signal(&mCondition);
    pthread_mutex_unlock(&mLock);
}

void LiveThread::handleRun(void *ptr) {

}

void *LiveThread::startThread(void *ptr) {
    LOGI("starting thread");
    LiveThread *thread = (LiveThread *) ptr;
    thread->mRunning = true;
    thread->handleRun(ptr);
    thread->mRunning = false;
    return NULL;
}
