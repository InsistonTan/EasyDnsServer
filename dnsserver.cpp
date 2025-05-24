#include "dnsserver.h"
#include "global.h"
#include <QMessageBox>
#include <QDateTime>
#include <QThread>
#include <handleDefaultDnsDataWorker.h>
#include <QCoreApplication>

QString DnsServer::getIpFromDomain(QString domain){
    if(dnsMap.isEmpty()){
        return QString();
    }

    if(dnsMap.keys().contains(domain)){
        return dnsMap[domain];
    }

    for(const auto key : dnsMap.keys()){
        if(!domain.isEmpty() && domain.endsWith(key)){
            return dnsMap[key];
        }
    }

    return QString();
}

void DnsServer::doWork(){
    socket = new QUdpSocket(this);

    // 监听 53 端口
    if (!socket->bind(QHostAddress(ip), 53)) {
        QMetaObject::invokeMethod(QCoreApplication::instance(), [=](){
            QMessageBox::critical(nullptr, "错误", "监听 UDP 53 端口失败!");
        }, Qt::QueuedConnection);

        return;
    }

    emit appendLogSignal("<span style='color:green;'>DNS服务启动成功</span>");

    setIsRunning(true);

    // 添加一个专门发送socket数据的任务
    getThreadPool()->start([=]{
        while(getIsRunning()){
            int queueSize = getQueueSize();
            for (int i = 0; i < queueSize - 1; i++) {
                auto data = popQueue();
                if(data != nullptr){
                    // 发送响应
                    socket->writeDatagram(data->datagram, data->host, data->port);
                }
            }

            QThread::msleep(50);
        }
    });

    while(getIsRunning()){
        while (socket->hasPendingDatagrams()) {
            QByteArray datagram;
            QHostAddress sender;
            quint16 senderPort;

            // 读取 UDP 数据包
            datagram.resize(socket->pendingDatagramSize());
            socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

            auto queryDomain = parseRequestedDomain(datagram).toLower();

            if(queryDomain.contains("/")){
                continue;
            }

            auto ip = getIpFromDomain(queryDomain);

            QByteArray response;
            if(!ip.isEmpty() && ip.split(".").size() == 4){
                emit appendLogSignal(" - [" + queryDomain + "] 该域名已配置ip : " + ip);
                // 解析 DNS 查询并生成响应
                response = generateDnsResponse(datagram, ip.split("."));
            }else{
                emit appendLogSignal(" - [" + queryDomain + "] 该域名使用系统DNS解析");
                // 使用系统dns
                forwardToSystemDns(datagram, sender, senderPort);
            }


            // 发送响应
            socket->writeDatagram(response, sender, senderPort);
        }

        QThread::msleep(10);
    }

    emit appendLogSignal("等待线程结束...");

    // 等待线程池的线程全部结束
    //getThreadPool()->waitForDone();

    //if(socket->isOpen()){
    socket->close();
    socket->deleteLater();

    emit appendLogSignal("<span style='color:red;'>服务已停止</span>");

    qDebug("服务已停止");

    emit workFinished();
}

void DnsServer::stopServerSlot(){
    setIsRunning(false);
}

QByteArray DnsServer::generateDnsResponse(const QByteArray &query, QList<QString> ip){
    // 简单处理：返回固定的 A 记录
    if (query.size() < 12) { // DNS Header 长度至少为 12 字节
        qWarning() << "Invalid DNS query.";
        return QByteArray();
    }

    QByteArray response = query; // 开始构造响应，以查询数据为基础
    response[2] = 0x81;          // 标志字节：设置为响应，无错误
    response[3] = 0x80;

    // 设置回答数为 1
    response[6] = 0x00;
    response[7] = 0x01;

    // 设置授权（Authority）部分的资源记录数量为 0
    response[8] = 0x00;
    response[9] = 0x00;

    // 设置附加（Additional）部分的资源记录数量为 0
    response[10] = 0x00;
    response[11] = 0x00;

    // 构造回答部分
    QByteArray answer;
    answer.append(query.mid(12)); // 复制查询的域名部分
    //answer.append(static_cast<char>(0x00)); // 回答结束符

    // 设置类型为 A（0x0001）
    //answer.append(static_cast<char>(0x00));
    //answer.append(static_cast<char>(0x01));

    // 设置类为 IN（0x0001）
    //answer.append(static_cast<char>(0x00));
    //answer.append(static_cast<char>(0x01));

    // 设置 TTL 为 60 秒（0x0000003C）
    answer.append(static_cast<char>(0x00));
    answer.append(static_cast<char>(0x00));
    answer.append(static_cast<char>(0x00));
    answer.append(static_cast<char>(0x3C));

    // 设置数据长度为 4（IPv4 地址）
    answer.append(static_cast<char>(0x00));
    answer.append(static_cast<char>(0x04));

    // 设置 IPv4 地址
    answer.append(static_cast<char>(ip[0].toInt()));
    answer.append(static_cast<char>(ip[1].toInt()));
    answer.append(static_cast<char>(ip[2].toInt()));
    answer.append(static_cast<char>(ip[3].toInt()));

    response.append(answer);
    return response;
}

void DnsServer::forwardToSystemDns(const QByteArray& requestData, QHostAddress sender, quint16 senderPort) {
    getThreadPool()->start(new handleDefaultDnsDataWorker(defaultDns, requestData, sender, senderPort));
}

QString DnsServer::parseRequestedDomain(const QByteArray &requestData) {
    QString domain;
    int index = 12;  // DNS 请求头通常前 12 字节是固定的，跳过这些

    while (true) {
        // 获取当前标签的长度
        quint8 labelLength = static_cast<quint8>(requestData[index]);
        index++;

        // 结束符，长度为 0 时，域名结束
        if (labelLength == 0) {
            break;
        }

        // 提取标签内容
        domain.append(requestData.mid(index, labelLength));
        domain.append(".");

        // 移动到下一个标签
        index += labelLength;
    }

    // 去除最后一个多余的点
    if (!domain.isEmpty() && domain.endsWith(".")) {
        domain.chop(1);
    }

    return domain;
}

