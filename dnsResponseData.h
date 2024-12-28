#ifndef DNSRESPONSEDATA_H
#define DNSRESPONSEDATA_H
#include<QByteArray>
#include<QHostAddress>

class DnsResponseData : public QObject{
public:
    QByteArray datagram;
    QHostAddress host;
    quint16 port;

    DnsResponseData(){}
    DnsResponseData(QByteArray datagram, QHostAddress host, quint16 port){
        this->datagram = datagram;
        this->host = host;
        this->port = port;
    }


    // 拷贝构造函数
    DnsResponseData(const DnsResponseData &other)
        : datagram(other.datagram), host(other.host), port(other.port) {}

    // 拷贝赋值操作符
    DnsResponseData& operator=(const DnsResponseData &other) {
        if (this != &other) {
            datagram = other.datagram;
            host = other.host;
            port = other.port;
        }
        return *this;
    }

    // 移动构造函数
    DnsResponseData(DnsResponseData &&other) noexcept
        : datagram(std::move(other.datagram)), host(std::move(other.host)), port(other.port) {
        other.port = 0; // 将被移动对象的 port 设置为一个默认值，防止悬空
    }

    // 移动赋值操作符
    DnsResponseData& operator=(DnsResponseData &&other) noexcept {
        if (this != &other) {
            datagram = std::move(other.datagram);
            host = std::move(other.host);
            port = other.port;
            other.port = 0; // 清空被移动对象的 port
        }
        return *this;
    }
};

#endif // DNSRESPONSEDATA_H
