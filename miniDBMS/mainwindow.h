#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include "storage.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void executeSQL();

private:
    QTextEdit* sqlEdit;
    QPushButton* executeButton;
    QPushButton* createTableButton;
    QTableWidget* tableWidget;
    Storage storage;

    void displayTable(const QString& tableName, const QVector<QString>& selectedCols = {});
};

#endif // MAINWINDOW_H
