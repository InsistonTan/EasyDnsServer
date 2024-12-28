#ifndef DNSSERVER_H
#define DNSSERVER_H

#include<QObject>
#include<QMap>
#include <QUdpSocket>

class DnsServer : public QObject {
    Q_OBJECT

private:
    //bool running = false;
    QUdpSocket *socket;
    QString ip;
    QString defaultDns;
    QMap<QString, QString> dnsMap;

    QString getIpFromDomain(QString domain);
    QByteArray generateDnsResponse(const QByteArray &query, QList<QString> ip);
    void forwardToSystemDns(const QByteArray& requestData, QHostAddress sender, quint16 senderPort);
    QString parseRequestedDomain(const QByteArray &requestData);

public:
    DnsServer(QString ip, QString defaultDns,QMap<QString, QString> dnsMap){
        this->ip = ip;
        this->defaultDns = defaultDns;
        this->dnsMap = dnsMap;
    }
public slots:
    // 工作任务
    void doWork();
    // 停止服务
    void stopServerSlot();

signals:
    // 任务结束信号
    void workFinished();
    // 追加日志信号
    void appendLogSignal(QString log);

};

#endif // DNSSERVER_H
