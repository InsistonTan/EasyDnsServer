#include "global.h"
#include <QDir>

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


bool isMonitorRunning = false;
QMutex mutexMonitor;
bool getIsMonitorRunning(){
    QMutexLocker locker(&mutexMonitor);
    return isMonitorRunning;
}
void setIsMonitorRunning(bool val){
    QMutexLocker locker(&mutexMonitor);
    isMonitorRunning = val;
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

// 获取app存储的数据文件所在路径
QString getAppDataDir(){
    QDir dir = QDir(QDir::homePath() + "/AppData/Local/");
    // 使用 QDir 创建不存在的目录
    if (!dir.exists("EasyDnsServer")) {
        if (dir.mkdir("EasyDnsServer")) {
            return QDir::homePath() + "/AppData/Local/EasyDnsServer/";
        }
    }else{
        return QDir::homePath() + "/AppData/Local/EasyDnsServer/";
    }

    return "";
}
