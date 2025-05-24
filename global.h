#ifndef GLOBAL_H
#define GLOBAL_H
#include<QMutex>
#include<QThreadPool>
#include<QHostAddress>
#include<dnsResponseData.h>
#include<QRegularExpression>
#include<QQueue>

const static QRegularExpression REMOVE_BLANK_REG("(^\\s+)|(\\s+$)|(\\s*#.*$)");

extern bool isRunning;
bool getIsRunning();
void setIsRunning(bool val);

extern bool isMonitorRunning;
bool getIsMonitorRunning();
void setIsMonitorRunning(bool val);

// 全局变量, 线程池
extern QThreadPool *threadPool;
QThreadPool* getThreadPool();


// 全局变量, 待发送的DnsResponseData队列
extern QQueue<DnsResponseData*> dataQueue;
void pushToQueue(DnsResponseData *data);
DnsResponseData* popQueue();
int getQueueSize();

// 获取app存储的数据文件所在路径
QString getAppDataDir();

#endif // GLOBAL_H
