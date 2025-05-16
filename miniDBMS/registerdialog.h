#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>

class QLineEdit;
class QPushButton;

class RegisterDialog : public QDialog {
    Q_OBJECT
public:
    explicit RegisterDialog(QWidget *parent = nullptr);

private slots:
    void handleRegister();

private:
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *confirmButton;
};

#endif // REGISTERDIALOG_H
