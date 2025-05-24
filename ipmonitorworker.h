#ifndef IPMONITORWORKER_H
#define IPMONITORWORKER_H

#include<QObject>
#include<QRunnable>
#include<global.h>
#include<QProcess>
#include<qDebug>
#include <QRegularExpression>


const static QRegularExpression GATEWAY_REGEXP("^[0-9\\.]+$");

class IpMonitorWorker : public QObject, public QRunnable{
    Q_OBJECT

private:
    QString localAddr;
    QString defaultDns;

    void getActiveLocalIPAddress() {
        QProcess process;
        process.start("route", QStringList() << "print");
        process.waitForFinished();
        QString output = process.readAllStandardOutput();


        output.replace(QRegularExpression("\\x1b\\[[0-9;]*m"), "");
        output = QString::fromLocal8Bit(output.toLocal8Bit());        // 打印输出内容

        QRegularExpression reg("\\s+");

        // 从输出中提取默认网关地址（在路由表中找到"0.0.0.0"的网关）
        QStringList lines = output.split("\n", Qt::SkipEmptyParts);
        for (auto line : lines) {
            if (line.contains("0.0.0.0")) {
                //qDebug() << "0.0.0.0 line: " << line;

                line.replace(REMOVE_BLANK_REG, "");

                QStringList columns = line.split(reg);
                if (columns.size() > 3 && !columns[2].isEmpty() && !columns[3].isEmpty()) {
                    // 验证网关是否符合正则
                    auto matcher = GATEWAY_REGEXP.match(columns[2]);
                    auto matcher2 = GATEWAY_REGEXP.match(columns[3]);
                    if(matcher.hasMatch() && matcher2.hasMatch()){
                        // 默认网关通常在第三列, 第4列为ip地址
                        this->defaultDns = columns.at(2);
                        this->localAddr = columns.at(3);
                        return;
                    }
                }
            }
        }

        this->defaultDns = "";
        this->localAddr = "";
    }
signals:
    void appendLogSignal(QString log);
    void updateIpLabelSignal(QString newIp);
    void startServerSignal(bool isRestartMode, QString localAddr, QString defaultDns);


public:
    IpMonitorWorker(){}
    void run() override{
        QString oldLocalAddr, oldDefaultDns;
        while(getIsMonitorRunning()){
            //qDebug() << "本机ip: " << localAddr.data() << ", 默认网关: " << defaultDns.data();
            getActiveLocalIPAddress();

            if(localAddr.isEmpty() || defaultDns.isEmpty()){
                emit updateIpLabelSignal(localAddr);

                if(getIsRunning()){
                    emit appendLogSignal("检测不到本机ip或网关, 准备停止服务");
                    setIsRunning(false);
                }
            }
            // ip或网关发生变化
            else if(localAddr != oldLocalAddr || defaultDns != oldDefaultDns){
                // 更新ip 的label
                emit updateIpLabelSignal(localAddr);

                if(!oldLocalAddr.isEmpty()){
                    emit appendLogSignal("检测到本机IP发生变化, 即将重启服务, 新IP: " + localAddr);
                    emit startServerSignal(true, localAddr, defaultDns);
                }else{
                    emit appendLogSignal("检测到本机IP, 即将启动服务, IP: " + localAddr);
                    emit startServerSignal(false, localAddr, defaultDns);
                }
            }

            oldLocalAddr = localAddr;
            oldDefaultDns = defaultDns;

            QThread::msleep(100);
        }
    }
};

#endif // IPMONITORWORKER_H
