#ifndef STARTUPWINDOW_H
#define STARTUPWINDOW_H

#include <QDialog>

class QPushButton;

class StartupWindow : public QDialog {
    Q_OBJECT
public:
    explicit StartupWindow(QWidget *parent = nullptr);

signals:
    void loginRequested();
    void registerRequested();

private:
    QPushButton *loginButton;
    QPushButton *registerButton;
};

#endif // STARTUPWINDOW_H
