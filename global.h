#ifndef GLOBAL_H
#define GLOBAL_H
#include<QMutex>
#include<QThreadPool>
#include<QHostAddress>
#include<dnsResponseData.h>
#include<QQueue>

extern bool isRunning;
bool getIsRunning();
void setIsRunning(bool val);

// 全局变量, 线程池
extern QThreadPool *threadPool;
QThreadPool* getThreadPool();


// 全局变量, 待发送的DnsResponseData队列
extern QQueue<DnsResponseData*> dataQueue;
void pushToQueue(DnsResponseData *data);
DnsResponseData* popQueue();
int getQueueSize();

#endif // GLOBAL_H
