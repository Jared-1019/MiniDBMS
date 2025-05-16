#include "logindialog.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("登录");
    resize(300, 150);

    usernameEdit = new QLineEdit;
    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);
    loginButton = new QPushButton("登录");

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(usernameEdit);
    layout->addWidget(passwordEdit);
    layout->addWidget(loginButton);
    setLayout(layout);

    usernameEdit->setPlaceholderText("用户名");
    passwordEdit->setPlaceholderText("密码");

    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::handleLogin);
}

void LoginDialog::handleLogin() {
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text().trimmed();

    QFile file("/Users/hongzhe/mindata/users.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法读取用户文件！");
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(",");
        if (parts.size() == 2 && parts[0] == username && parts[1] == password) {
            file.close();
            emit loginSuccess();
            accept();  // 登录成功，关闭窗口
            return;
        }
    }

    file.close();
    QMessageBox::warning(this, "错误", "用户名不存在或密码错误！");
}
