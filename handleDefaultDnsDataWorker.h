#ifndef HANDLEDEFAULTDNSDATAWORKER_H
#define HANDLEDEFAULTDNSDATAWORKER_H
#include <QObject>
#include <QRunnable>
#include <QHostAddress>
#include <QUdpSocket>
#include <dnsResponseData.h>
#include <global.h>

class handleDefaultDnsDataWorker : public QObject, public QRunnable {
    Q_OBJECT

private:
    QString defaultDns;
    QByteArray datagram;
    QHostAddress sender;
    quint16 senderPort;
public:
    handleDefaultDnsDataWorker(QString defaultDns, QByteArray datagram, QHostAddress sender, quint16 senderPort){
        this->datagram = datagram;
        this->senderPort = senderPort;
        this->sender = sender;
        this->defaultDns = defaultDns;
    }

    void run() override{
        // 配置系统 DNS 服务器的地址
        QHostAddress systemDnsServer(defaultDns);
        quint16 systemDnsPort = 53;

        // 使用 UDP 向系统 DNS 服务器发送请求
        QUdpSocket socket;
        socket.writeDatagram(datagram, systemDnsServer, systemDnsPort);
        socket.waitForReadyRead();

        QByteArray responseData;
        while (socket.hasPendingDatagrams()) {
            responseData.resize(socket.pendingDatagramSize());
            socket.readDatagram(responseData.data(), responseData.size());
        }

        if(socket.isValid() && socket.isOpen()){
            socket.close();
            socket.deleteLater();
        }

        // 将结果加入队列
        pushToQueue(new DnsResponseData(responseData, sender, senderPort));
    }
};

#endif // HANDLEDEFAULTDNSDATAWORKER_H
