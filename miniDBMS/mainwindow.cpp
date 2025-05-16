#include "mainwindow.h"
#include <QWidget>
#include <QMessageBox>
#include <QSplitter>
#include <QRegularExpression>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <numeric>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // ---------- SQL 编辑框 ----------
    sqlEdit = new QTextEdit(this);
    sqlEdit->setPlaceholderText("请输入 SQL 语句，例如：\nCREATE TABLE student (id INT, name TEXT);\nINSERT INTO student (id, name) VALUES (1, 'Tom');");
    QFont sqlFont("Courier New");
    sqlFont.setPointSize(11);
    sqlEdit->setFont(sqlFont);
    sqlEdit->setMinimumHeight(150);

    // ---------- 操作按钮 ----------
    executeButton = new QPushButton(QIcon(":/icons/run.png"), "执行 SQL", this);
    QPushButton* clearButton = new QPushButton(QIcon(":/icons/clear.png"), "清空 SQL", this); // 新按钮

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(executeButton);
    buttonLayout->addWidget(clearButton);

    // ---------- 表格控件 ----------
    tableWidget = new QTableWidget(this);
    tableWidget->setAlternatingRowColors(true);
    tableWidget->setStyleSheet("QTableWidget { background-color: #fdfdfd; }");
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    // ---------- 上下分隔布局 ----------
    QWidget* upperPanel = new QWidget(this);
    QVBoxLayout* upperLayout = new QVBoxLayout(upperPanel);
    upperLayout->addWidget(sqlEdit);
    upperLayout->addLayout(buttonLayout);
    upperLayout->setContentsMargins(0, 0, 0, 0);

    QSplitter* splitter = new QSplitter(Qt::Vertical, this);
    splitter->addWidget(upperPanel);
    splitter->addWidget(tableWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(splitter);
    centralWidget->setLayout(mainLayout);

    // ---------- 连接信号 ----------
    connect(executeButton, &QPushButton::clicked, this, &MainWindow::executeSQL);
    connect(clearButton, &QPushButton::clicked, sqlEdit, &QTextEdit::clear);

    // ---------- 窗口属性 ----------
    setWindowTitle("Mini DBMS");
    resize(900, 600);
}
MainWindow::~MainWindow()
{
    // 所有控件都有父组件，不需要手动释放
}

void MainWindow::executeSQL()
{
    QString sql = sqlEdit->toPlainText().trimmed();

    if (sql.startsWith("CREATE TABLE", Qt::CaseInsensitive)) {
        QRegularExpression re(R"(CREATE TABLE (\w+)\s*\((.+)\))", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = re.match(sql);
        if (match.hasMatch()) {
            QString tableName = match.captured(1);
            QStringList columnsAndTypes = match.captured(2).split(",", Qt::SkipEmptyParts);
            QVector<QString> colNames, colTypes;
            for (const QString& entry : columnsAndTypes) {
                QStringList parts = entry.trimmed().split(" ", Qt::SkipEmptyParts);
                if (parts.size() == 2) {
                    colNames.append(parts[0].trimmed());
                    colTypes.append(parts[1].trimmed());
                }
            }
            if (storage.createTable(tableName, colNames, colTypes)) {
                QMessageBox::information(this, "提示", "创建表成功！");
                displayTable(tableName);
            } else {
                QMessageBox::warning(this, "错误", "创建表失败！");
            }
        }

    }if (sql.startsWith("CREATE DATABASE", Qt::CaseInsensitive)) {
        QRegularExpression re(R"(CREATE DATABASE (\w+))", QRegularExpression::CaseInsensitiveOption);
        auto match = re.match(sql);
        if (match.hasMatch()) {
            QString dbName = match.captured(1);
            if (storage.createDatabase(dbName)) {
                QMessageBox::information(this, "提示", "数据库创建成功！");
            } else {
                QMessageBox::warning(this, "错误", "数据库创建失败！");
            }
        }
    } else if (sql.startsWith("USE", Qt::CaseInsensitive)) {
        QRegularExpression re(R"(USE (\w+))", QRegularExpression::CaseInsensitiveOption);
        auto match = re.match(sql);
        if (match.hasMatch()) {
            QString dbName = match.captured(1);
            if (storage.useDatabase(dbName)) {
                QMessageBox::information(this, "提示", "切换数据库成功！");
            } else {
                QMessageBox::warning(this, "错误", "数据库不存在！");
            }
        }
    } else if (sql.startsWith("DROP DATABASE", Qt::CaseInsensitive)) {
        QRegularExpression re(R"(DROP DATABASE (\w+))", QRegularExpression::CaseInsensitiveOption);
        auto match = re.match(sql);
        if (match.hasMatch()) {
            QString dbName = match.captured(1);
            if (storage.deleteDatabase(dbName)) {
                QMessageBox::information(this, "提示", "数据库删除成功！");
            } else {
                QMessageBox::warning(this, "错误", "数据库删除失败！");
            }
        }
    } else if (sql.startsWith("INSERT INTO", Qt::CaseInsensitive)) {
        QRegularExpression re(R"(INSERT INTO (\w+)\s*\(([^)]+)\)\s*VALUES\s*\(([^)]+)\))", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = re.match(sql);
        if (match.hasMatch()) {
            QString tableName = match.captured(1).trimmed();
            QStringList colList = match.captured(2).split(",", Qt::SkipEmptyParts);
            QStringList valList = match.captured(3).split(",", Qt::SkipEmptyParts);

            QVector<QString> columnNames, values;
            for (const QString& c : colList) columnNames.append(c.trimmed());
            for (QString v : valList) {
                v = v.trimmed();
                if (v.startsWith("'") && v.endsWith("'")) {
                    v = v.mid(1, v.length() - 2); // remove quotes
                }
                values.append(v);
            }

            if (storage.insertRow(tableName, columnNames, values)) {
                QMessageBox::information(this, "提示", "插入成功！");
                displayTable(tableName);
            } else {
                QMessageBox::warning(this, "错误", "插入失败！");
            }
        } else {
            QMessageBox::warning(this, "错误", "INSERT 语句格式错误，应为 INSERT INTO 表名 (列1, 列2) VALUES (值1, 值2)");
        }
    } else if (sql.startsWith("DELETE FROM", Qt::CaseInsensitive)) {
        QRegularExpression re(R"(DELETE FROM\s+(\w+)\s+WHERE\s+(\w+)\s*=\s*('?[^']+'?))", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = re.match(sql);
        if (match.hasMatch()) {
            QString tableName = match.captured(1).trimmed();
            QString whereCol = match.captured(2).trimmed();
            QString whereVal = match.captured(3).trimmed().remove("'");

            if (storage.deleteRow(tableName, whereCol, whereVal)) {
                QMessageBox::information(this, "提示", "删除成功！");
                displayTable(tableName);
            } else {
                QMessageBox::warning(this, "错误", "删除失败，可能没有匹配的记录！");
            }
        } else {
            QMessageBox::warning(this, "错误", "DELETE 语句格式错误，应为 DELETE FROM 表名 WHERE 列=值");
        }
    } else if (sql.startsWith("DROP TABLE", Qt::CaseInsensitive)) {
        QRegularExpression re(R"(DROP TABLE (\w+))", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = re.match(sql);
        if (match.hasMatch()) {
            QString tableName = match.captured(1);
            if (storage.deleteTable(tableName)) {
                QMessageBox::information(this, "提示", "删除表成功！");
                tableWidget->clear();
            } else {
                QMessageBox::warning(this, "错误", "删除表失败！");
            }
        }

    } else if (sql.startsWith("ALTER TABLE", Qt::CaseInsensitive)) {
        QRegularExpression addRe(R"(ALTER TABLE (\w+)\s+ADD COLUMN\s+(\w+)\s+(\w+))", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression dropRe(R"(ALTER TABLE (\w+)\s+DROP COLUMN\s+(\w+))", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression modRe(R"(ALTER TABLE (\w+)\s+MODIFY COLUMN\s+(\w+)\s+(\w+))", QRegularExpression::CaseInsensitiveOption);

        QRegularExpressionMatch match;
        if ((match = addRe.match(sql)).hasMatch()) {
            QString table = match.captured(1);
            QString col = match.captured(2);
            QString type = match.captured(3);
            if (storage.addColumn(table, col, type)) {
                QMessageBox::information(this, "成功", "添加列成功！");
                displayTable(table);
            } else {
                QMessageBox::warning(this, "失败", "添加列失败！");
            }
        } else if ((match = dropRe.match(sql)).hasMatch()) {
            QString table = match.captured(1);
            QString col = match.captured(2);
            if (storage.dropColumn(table, col)) {
                QMessageBox::information(this, "成功", "删除列成功！");
                displayTable(table);
            } else {
                QMessageBox::warning(this, "失败", "删除列失败！");
            }
        } else if ((match = modRe.match(sql)).hasMatch()) {
            QString table = match.captured(1);
            QString col = match.captured(2);
            QString newType = match.captured(3);
            if (storage.modifyColumn(table, col, newType)) {
                QMessageBox::information(this, "成功", "修改列类型成功！");
                displayTable(table);
            } else {
                QMessageBox::warning(this, "失败", "修改列类型失败！");
            }
        } else {
            QMessageBox::warning(this, "错误", "ALTER TABLE 语法不正确！");
        }
    } else if (sql.startsWith("UPDATE", Qt::CaseInsensitive)) {
        QRegularExpression re(R"(UPDATE\s+(\w+)\s+SET\s+(\w+)\s*=\s*('?[^']+'?)\s+WHERE\s+(\w+)\s*=\s*('?[^']+'?))",
                              QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = re.match(sql);
        if (match.hasMatch()) {
            QString tableName = match.captured(1).trimmed();
            QString setCol = match.captured(2).trimmed();
            QString setVal = match.captured(3).trimmed().remove("'");
            QString whereCol = match.captured(4).trimmed();
            QString whereVal = match.captured(5).trimmed().remove("'");

            if (storage.updateRow(tableName, setCol, setVal, whereCol, whereVal)) {
                QMessageBox::information(this, "提示", "更新成功！");
                displayTable(tableName);  // 显示更新后的表
            } else {
                QMessageBox::warning(this, "错误", "更新失败，可能没有匹配的记录！");
            }
        } else {
            QMessageBox::warning(this, "错误", "UPDATE 语句格式错误，应为 UPDATE 表名 SET 列=值 WHERE 列=值");
        }
    } else if (sql.startsWith("SELECT", Qt::CaseInsensitive)) {
        QRegularExpression reWithWhere(R"(SELECT\s+(.+)\s+FROM\s+(\w+)\s+WHERE\s+(\w+)\s*=\s*('?[^']+'?))", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = reWithWhere.match(sql);

        if (match.hasMatch()) {
            QString colPart = match.captured(1).trimmed();
            QString tableName = match.captured(2).trimmed();
            QString whereCol = match.captured(3).trimmed();
            QString whereVal = match.captured(4).trimmed().remove('\'');

            QVector<QString> selectedCols;
            if (colPart != "*") {
                for (QString c : colPart.split(",", Qt::SkipEmptyParts))
                    selectedCols.append(c.trimmed());
            }

            QVector<QVector<QString>> result;
            if (storage.selectWhere(tableName, selectedCols, whereCol, whereVal, result)) {
                tableWidget->clear();

                if (result.size() >= 2) {
                    QVector<QString> headers = result[0];
                    tableWidget->setColumnCount(headers.size());
                    tableWidget->setRowCount(result.size() - 1);  // ✅ 修复为 -1

                    for (int i = 0; i < headers.size(); ++i)
                        tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(headers[i]));

                    for (int row = 1; row < result.size(); ++row) {  // ✅ 从第1行开始读取数据
                        for (int col = 0; col < headers.size(); ++col) {
                            tableWidget->setItem(row - 1, col, new QTableWidgetItem(result[row][col]));
                        }
                    }
                } else {
                    QMessageBox::information(this, "结果", "没有符合条件的数据");
                }
            } else {
                QMessageBox::warning(this, "错误", "读取数据失败！");
            }

        } else {
            // 原始 SELECT 不带 WHERE 的处理逻辑
            QRegularExpression re(R"(SELECT\s+(.+)\s+FROM\s+(\w+))", QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatch match = re.match(sql);

            if (match.hasMatch()) {
                QString colPart = match.captured(1).trimmed();
                QString tableName = match.captured(2).trimmed();

                QVector<QString> cols;
                if (colPart == "*") {
                    displayTable(tableName);
                } else {
                    QStringList colList = colPart.split(",", Qt::SkipEmptyParts);
                    for (const QString& c : colList)
                        cols.append(c.trimmed());
                    displayTable(tableName, cols);
                }
            } else {
                QMessageBox::warning(this, "错误", "SELECT 语句格式不正确，应为 SELECT 列1,列2 FROM 表名 [WHERE 列=值]");
            }
        }
    } else {
        QMessageBox::warning(this, "错误", "无法识别的SQL语句！");
    }
}

void MainWindow::displayTable(const QString& tableName, const QVector<QString>& selectedCols) {
    QVector<QVector<QString>> tableData;
    QString path = storage.getTablePath(tableName);

    if (!storage.readTable(path, tableData)) {
        QMessageBox::warning(this, "错误", "无法读取表：" + tableName);
        return;
    }

    if (tableData.size() < 2) {
        QMessageBox::warning(this, "错误", "表结构不完整！");
        return;
    }

    QVector<QString> allHeaders = tableData[0]; // 第一行是列名
    QVector<int> selectedIndexes;

    // 确定要显示的列索引
    if (selectedCols.isEmpty()) {
        selectedIndexes.resize(allHeaders.size());
        std::iota(selectedIndexes.begin(), selectedIndexes.end(), 0); // 全部列
    } else {
        for (const QString& col : selectedCols) {
            int idx = allHeaders.indexOf(col);
            if (idx == -1) {
                QMessageBox::warning(this, "错误", "列不存在：" + col);
                return;
            }
            selectedIndexes.append(idx);
        }
    }

    // 设置表格列和行
    tableWidget->clear();  // 清除旧内容
    tableWidget->setColumnCount(selectedIndexes.size());
    tableWidget->setRowCount(tableData.size() - 2); // 去除前两行（列名和类型）

    // 设置表头
    for (int col = 0; col < selectedIndexes.size(); ++col) {
        tableWidget->setHorizontalHeaderItem(col, new QTableWidgetItem(allHeaders[selectedIndexes[col]]));
    }

    // 填充数据
    for (int row = 2; row < tableData.size(); ++row) {
        for (int col = 0; col < selectedIndexes.size(); ++col) {
            QString cell = tableData[row][selectedIndexes[col]];
            tableWidget->setItem(row - 2, col, new QTableWidgetItem(cell));
        }
    }

    tableWidget->resizeColumnsToContents();
}
