#include "global.h"

bool isRunning = false;
QMutex mutex;
bool getIsRunning(){
    QMutexLocker locker(&mutex);
    return isRunning;
}
void setIsRunning(bool val){
    QMutexLocker locker(&mutex);
    isRunning = val;
}


// 全局变量, 线程池
QThreadPool *threadPool = QThreadPool::globalInstance();
QThreadPool* getThreadPool(){
    return threadPool;
}

// 全局变量, 待发送的DnsResponseData队列
QQueue<DnsResponseData*> dataQueue;
QMutex queueMutex;
void pushToQueue(DnsResponseData* data){
    QMutexLocker locker(&queueMutex);
    dataQueue.enqueue(data);
}
DnsResponseData* popQueue(){
    QMutexLocker locker(&queueMutex);
    if(dataQueue.size() > 0){
        return dataQueue.dequeue();
    }
    return nullptr;
}
int getQueueSize(){
    QMutexLocker locker(&queueMutex);
    return dataQueue.size();
}
