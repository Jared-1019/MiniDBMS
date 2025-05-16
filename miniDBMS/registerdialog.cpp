#include "registerdialog.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

RegisterDialog::RegisterDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("注册账户");
    resize(300, 150);

    usernameEdit = new QLineEdit;
    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);
    confirmButton = new QPushButton("确认注册");

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(usernameEdit);
    layout->addWidget(passwordEdit);
    layout->addWidget(confirmButton);
    setLayout(layout);

    usernameEdit->setPlaceholderText("用户名");
    passwordEdit->setPlaceholderText("密码");

    connect(confirmButton, &QPushButton::clicked, this, &RegisterDialog::handleRegister);
}

void RegisterDialog::handleRegister() {
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "错误", "用户名和密码不能为空！");
        return;
    }

    QFile file("/Users/hongzhe/mindata/users.txt");
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法写入用户文件！");
        return;
    }

    QTextStream out(&file);
    out << username << "," << password << "\n";
    file.close();

    QMessageBox::information(this, "成功", "注册成功！");
    accept();  // 关闭注册窗口
}
