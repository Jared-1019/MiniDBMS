#include <QApplication>
#include "mainwindow.h"
#include "startupwindow.h"
#include "registerdialog.h"
#include "logindialog.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    StartupWindow startup;
    MainWindow w;

    QObject::connect(&startup, &StartupWindow::registerRequested, [&]() {
        RegisterDialog regDialog;
        regDialog.exec();  // 弹出注册窗口
    });

    QObject::connect(&startup, &StartupWindow::loginRequested, [&]() {
        LoginDialog loginDialog;
        QObject::connect(&loginDialog, &LoginDialog::loginSuccess, [&]() {
            w.show();  // 登录成功后显示主界面
            startup.close();
        });
        loginDialog.exec();
    });

    startup.exec();  // 显示启动界面（模式窗口）

    return a.exec();
}
