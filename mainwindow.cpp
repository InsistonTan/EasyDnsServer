#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dnsserver.h"
#include <QNetworkInterface>
#include <QProcess>
#include <QRegularExpression>
#include <QFile>
#include <QMessageBox>
#include <QDateTime>
#include <QScrollBar>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QSettings>
#include "global.h"

QString getFormatLog(QString log){
    // 获取当前日期和时间
    QDateTime currentDateTime = QDateTime::currentDateTime();

    // 格式化输出当前日期和时间
    QString formattedDateTime = "[" + currentDateTime.toString("yyyy-MM-dd HH:mm:ss") + "] ";

    return formattedDateTime.append(log);
}

void MainWindow::appendLog(QString log){
    this->ui->textEdit_2->append(getFormatLog(log));

    // 获取垂直滚动条
    QScrollBar *verticalScrollBar = this->ui->textEdit_2->verticalScrollBar();
    // 自动滚动到底部
    verticalScrollBar->setValue(verticalScrollBar->maximum());
}

//重写窗口关闭事件
void MainWindow::closeEvent(QCloseEvent *event){
    // 如果服务还在运行, 关闭窗口则最小化到托盘
    if(getIsRunning()){
        event->ignore();  // 阻止关闭窗口
        this->hide();     // 隐藏窗口

        // 将应用程序最小化到系统托盘
        trayIcon->showMessage("程序后台运行", "程序已经最小化到后台运行。", QSystemTrayIcon::Information);
    }else{
        // 服务停止了, 关闭窗口则关闭程序
        event->accept();
    }

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle(QString("EasyDnsServer v") + CURRENT_VERSION);

    // 创建系统托盘图标
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/res/assets/icon-eds.png"));
    trayIcon->show();
    // 创建托盘菜单
    QMenu *trayMenu = new QMenu(this);
    QAction *quitAction = new QAction("退出", this);
    // 退出选项点击的信号和槽
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
    trayMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayMenu);
    // 点击托盘图标时的槽
    connect(trayIcon, &QSystemTrayIcon::activated, this, [=](QSystemTrayIcon::ActivationReason reason){
        if (reason == QSystemTrayIcon::Trigger) {
            // 如果窗口已经最小化，恢复显示
            this->show();
            this->raise(); // 提升窗口到最前
            this->activateWindow(); // 激活窗口
        }
    });


    // 设置最大线程数为cpu核心线程数减一
    int coreThreadSize = std::thread::hardware_concurrency();
    getThreadPool()->setMaxThreadCount(coreThreadSize > 6 ? coreThreadSize - 2 : 6);


    // 使用 QSignalBlocker 阻止信号触发
    {
        QSignalBlocker blocker(ui->checkBox);  // 阻止checkBox所有信号
        // 检查开机自启动
        if(isStartupWithSystem()){
            ui->checkBox->setChecked(true);
        }else{
            ui->checkBox->setChecked(false);
        }
    }  // blocker 被销毁后，信号会重新触发


    // 无效的数据, 用于占位
    dnsMap[",!();"] = "0";

    ui->lineEdit->setReadOnly(true);

    ui->textEdit_2->setReadOnly(true);

    QFile configFile = QFile(getAppDataDir() + "dns_config");

    appendLog("正在读取dns配置...");

    // 打开文件进行读取
    if (configFile.exists() && (configFile.open(QIODevice::ReadOnly | QIODevice::Text))) {
        QTextStream in(&configFile);

        QRegularExpression reg("\\s+");

        int lineCounter = 0;

        // 逐行读取文件
        while (!in.atEnd()) {
            lineCounter++;

            QString line = in.readLine(); // 读取一行
            ui->textEdit->append(line);

            if(line.startsWith("#")){
                continue;
            }

            line.replace(REMOVE_BLANK_REG, "");

            QStringList columns = line.split(reg);
            if(columns.size() == 2 && columns[0].split(".").size() == 4){
                dnsMap[columns[1].toLower()] = columns[0];
            }
        }

        appendLog("dns配置文件行数: " + QString(std::to_string(lineCounter).data()) + ", 有效配置条数: " + QString(std::to_string(dnsMap.size() - 1).data()));
    }else{
        appendLog("dns配置文件不存在或不可读");
    }

    if(ui->textEdit->toPlainText().isEmpty()){
        ui->textEdit->setText("# 配置格式:ip 域名, 示例:0.0.0.0 www.baidu.com");
    }

    startIPMonitor();
}

void MainWindow::startIPMonitor(){
    appendLog("<span style='color:green;'>已开启IP监测</span>");

    // 开启ip监测任务
    IpMonitorWorker *ipmonitor = new IpMonitorWorker();
    connect(ipmonitor, &IpMonitorWorker::appendLogSignal, this, &MainWindow::appendLog);
    connect(ipmonitor, &IpMonitorWorker::updateIpLabelSignal, this, &MainWindow::updateIpLabel);
    connect(ipmonitor, &IpMonitorWorker::startServerSignal, this, &MainWindow::monitorAskStart);

    setIsMonitorRunning(true);

    getThreadPool()->start(ipmonitor);
}

MainWindow::~MainWindow()
{
    setIsMonitorRunning(false);
    setIsRunning(false);
    delete ui;
}


void MainWindow::startServer(){
    DnsServer *dnsServer = new DnsServer(localAddr, defaultDns, dnsMap);

    QThread *thread = new QThread();

    // 将 worker 移到新线程
    dnsServer->moveToThread(thread);

    // 当线程开始时，启动工作任务
    QObject::connect(thread, &QThread::started, dnsServer, &DnsServer::doWork);

    // 任务完成后，退出线程并清理
    QObject::connect(dnsServer, &DnsServer::workFinished, thread, &QThread::quit);
    QObject::connect(dnsServer, &DnsServer::workFinished, dnsServer, &DnsServer::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    // 停止信号连接到dnsServer
    //QObject::connect(this, &MainWindow::stopServerSignal, dnsServer, &DnsServer::stopServerSlot);

    // dnsServer日志更新信号连接到this
    QObject::connect(dnsServer, &DnsServer::appendLogSignal, this, &MainWindow::appendLog);

    // 启动线程
    thread->start();

    setIsRunning(true);

    qDebug() << "服务已启动";
}


void MainWindow::on_pushButton_clicked()
{
    if(getIsRunning()){
        setIsRunning(false);
        setIsMonitorRunning(false);
        appendLog("<span style='color:red;'>已停止IP监测!</span>");

        emit stopServerSignal();

        qDebug() << "停止服务";
    }

}

void MainWindow::restartServer(){
    // 如果正在运行, 需要先停止再启动
    if(getIsRunning()){
        setIsRunning(false);

        QTimer* timer = new QTimer(this);
        timer->setSingleShot(true);
        connect(timer, &QTimer::timeout, this, [=]() {
            // 重新加载配置
            reloadDnsConfig();
            saveDnsConfig();

            // 启动服务
            startServer();

            qDebug() << "启动服务";
        });

        // 启动定时器
        timer->start(500);  // 500毫秒后触发
    }else{
        // 重新加载配置
        reloadDnsConfig();
        saveDnsConfig();
        // 启动服务
        startServer();

        qDebug() << "启动服务";
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    // 重启dns服务
    restartServer();
}

void MainWindow::reloadDnsConfig(){
    dnsMap.clear();
    // 无效的数据, 用于占位
    dnsMap[",!();"] = "0";

    auto text = ui->textEdit->toPlainText();
    if(!text.isEmpty()){
        auto dnsList = text.split("\n");

        QRegularExpression reg("\\s+");
        if(dnsList.size() > 0){
            for(auto dnsItem : dnsList){
                if(dnsItem.startsWith("#")){
                    continue;
                }

                dnsItem.replace(REMOVE_BLANK_REG, "");

                auto dnsItemSplits = dnsItem.split(reg);
                if(dnsItemSplits.size() == 2 && dnsItemSplits[0].split(".").size() == 4){
                    dnsMap[dnsItemSplits[1].toLower()] = dnsItemSplits[0];
                }
            }
        }

        appendLog("dns配置文件行数: " + QString(std::to_string(dnsList.size()).data()) + ", 有效配置条数: " + QString(std::to_string(dnsMap.size() - 1).data()));
    }
}

void MainWindow::saveDnsConfig(){
    // 创建一个 QFile 对象，并打开文件进行写入
    QFile file(getAppDataDir() + DNS_CONFIG_FILE);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);  // 创建一个文本流对象
        out << ui->textEdit->toPlainText(); // 写入文本

        // 关闭文件
        file.close();
        appendLog("DNS配置保存成功");
    } else {
        appendLog("DNS配置保存失败!");
    }
}


void MainWindow::on_pushButton_3_clicked()
{
    // 保存dns配置
    saveDnsConfig();
}


void MainWindow::on_pushButton_4_clicked()
{
    QMainWindow *window = new QMainWindow();
    window->setWindowTitle("请作者喝杯奶茶, 感谢您的支持和鼓励");
    window->setFixedSize(400, 300);

    // 创建一个 QLabel 来显示图片
    QLabel* label = new QLabel();
    QPixmap pixmap(":/res/assets/shoukuanma.jpg");  // 替换为你的图片路径
    pixmap = pixmap.scaled(400, 300, Qt::KeepAspectRatio);
    label->setPixmap(pixmap);
    label->setAlignment(Qt::AlignCenter);  // 设置图片居中显示

    // 创建一个 QWidget 作为主窗口的中央部件
    QWidget* centralWidget = new QWidget();

    // 创建一个 QVBoxLayout 布局管理器
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    // 将 QLabel 添加到布局中
    layout->addWidget(label);

    // 设置布局为中央部件的布局
    centralWidget->setLayout(layout);

    // 设置中央部件
    window->setCentralWidget(centralWidget);

    window->show();
}


void MainWindow::on_checkBox_stateChanged(int arg1)
{

    // 获取应用程序的路径
    QString appPath = QFileInfo(QCoreApplication::applicationFilePath()).absoluteFilePath();

    // 获取程序名称
    QString appName = QFileInfo(appPath).fileName();

    // 创建 QSettings 对象，操作 Windows 注册表
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);

    // 勾选了开机自启动
    if(arg1 > 0){
        // 在注册表中添加应用程序的启动项, 并且添加 --hide运行参数, 在main.cpp中检查运行参数, 带有--hide的隐藏主窗口运行
        settings.setValue(appName, "\"" + appPath.replace("/", "\\") + "\" --hide");

        ui->checkBox->setChecked(true);

        appendLog("已设置开机自启动");
    }else{
        // 取消开机自启动
        // 从注册表中移除应用程序的启动项
        settings.remove(appName);

        ui->checkBox->setChecked(false);

        appendLog("取消开机自启动成功!");
    }
}

bool MainWindow::isStartupWithSystem(){
    // 获取应用程序的路径
    QString appPath = QFileInfo(QCoreApplication::applicationFilePath()).absoluteFilePath();

    // 获取程序名称
    QString appName = QFileInfo(appPath).fileName();

    // 创建 QSettings 对象，操作 Windows 注册表
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);

    return settings.contains(appName);
}

void MainWindow::updateIpLabel(QString newIp){
    ui->lineEdit->setText(newIp);
}

void MainWindow::monitorAskStart(bool isRestartMode, QString localAddr, QString defaultDns){
    this->localAddr = localAddr;
    this->defaultDns = defaultDns;

    if(isRestartMode){
        restartServer();
    }else{
        startServer();
    }
}

