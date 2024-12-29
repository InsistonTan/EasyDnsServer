#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
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
