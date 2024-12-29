#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QRegularExpression>
#include <QSystemTrayIcon>
#include <QCloseEvent>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

#define DNS_CONFIG_FILE "dns_config"

const static QRegularExpression REMOVE_BLANK_REG("(^\\s+)|(\\s+$)|(\\s*#.*$)");

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // 重写窗口关闭事件
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
    QMap<QString, QString> dnsMap;
    QString localAddr;
    QString defaultDns;
    QSystemTrayIcon *trayIcon;  // 系统托盘图标


    void startServer();
    void saveDnsConfig();
    void reloadDnsConfig();
    void getActiveLocalIPAddress();
    bool isStartupWithSystem();

signals:
    void stopServerSignal();
private slots:
    void appendLog(QString log);
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_checkBox_stateChanged(int arg1);
};
#endif // MAINWINDOW_H
