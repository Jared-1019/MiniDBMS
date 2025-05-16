#include "startupwindow.h"
#include <QPushButton>
#include <QVBoxLayout>

StartupWindow::StartupWindow(QWidget *parent) : QDialog(parent) {
    setWindowTitle("欢迎使用 miniDBMS");
    resize(300, 150);

    loginButton = new QPushButton("登录");
    registerButton = new QPushButton("注册");

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(loginButton);
    layout->addWidget(registerButton);
    setLayout(layout);

    connect(loginButton, &QPushButton::clicked, this, &StartupWindow::loginRequested);
    connect(registerButton, &QPushButton::clicked, this, &StartupWindow::registerRequested);
}
