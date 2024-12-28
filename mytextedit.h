#ifndef MYTEXTEDIT_H
#define MYTEXTEDIT_H

#include <QTextEdit>
#include <QDebug>

class MyTextEdit : public QTextEdit {
    Q_OBJECT

public:
    MyTextEdit(QWidget *parent = nullptr) : QTextEdit(parent) {}

protected:
    // 重载 focusOutEvent 方法
    void focusOutEvent(QFocusEvent *event) override {
        QTextEdit::focusOutEvent(event);  // 调用基类的 focusOutEvent 处理
        qDebug() << "QTextEdit lost focus!";
    }
};

#endif // MYTEXTEDIT_H
