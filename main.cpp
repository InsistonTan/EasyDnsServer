#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>
#include <csignal>

// 系统错误信号处理
void signalHandler(int signal) {
    const char* errorMsg = nullptr;
    switch (signal) {
    case SIGSEGV: errorMsg = "Segmentation Fault (内存访问错误)\n\n例如空指针异常, 数组下标越界等"; break;
    case SIGFPE:  errorMsg = "Floating Point Exception (算术错误)"; break;
    case SIGILL:  errorMsg = "Illegal Instruction (非法指令)"; break;
    case SIGABRT: errorMsg = "Abort Signal (程序中止)"; break;
    default:      errorMsg = "Unknown Signal (未知错误信号)"; break;
    }

    QMessageBox::critical(nullptr, "程序错误", errorMsg);
    std::exit(EXIT_FAILURE);  // 安全退出
}

int main(int argc, char *argv[])
{
    // 注册错误信号处理函数
    std::signal(SIGSEGV, signalHandler);
    std::signal(SIGFPE,  signalHandler);
    std::signal(SIGILL,  signalHandler);
    std::signal(SIGABRT, signalHandler);


    QApplication a(argc, argv);
    MainWindow w;


    // 解析命令行参数
    QCommandLineParser parser;
    parser.setApplicationDescription("EasyDnsServer");
    parser.addHelpOption();
    parser.addOption(QCommandLineOption("hide", "Start the application background"));
    parser.process(a);

    // 检查运行参数是否带 --hide
    if(parser.isSet("hide")){
        w.hide();
    }else{
        w.show();
    }

    return a.exec();
}
