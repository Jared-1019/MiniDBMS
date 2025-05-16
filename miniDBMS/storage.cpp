#include "storage.h"
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDebug>

Storage::Storage() {
    currentDatabasePath = "/Users/hongzhe/mindata";
    QDir dir(currentDatabasePath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

QString Storage::getTablePath(const QString& tableName) const {
    return currentDatabasePath + "/" + tableName + ".csv";
}

QString Storage::getCurrentDatabasePath() const {
    return currentDatabasePath;
}

bool Storage::readTable(const QString& filePath, QVector<QVector<QString>>& tableData) const {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QTextStream in(&file);
    while (!in.atEnd()) {
        tableData.append(in.readLine().split(","));
    }
    file.close();
    return true;
}

bool Storage::writeTable(const QString& filePath, const QVector<QVector<QString>>& tableData) const {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) return false;

    QTextStream out(&file);
    for (const auto& row : tableData) {
        out << row.join(",") << "\n";
    }
    file.close();
    return true;
}

bool Storage::createTable(const QString& tableName, const QVector<QString>& columnNames, const QVector<QString>& columnTypes) {
    QString path = getTablePath(tableName);
    if (QFile::exists(path)) return false;

    QVector<QVector<QString>> tableData;
    tableData.append(columnNames);
    tableData.append(columnTypes);
    return writeTable(path, tableData);
}

bool Storage::insertValue(const QString& tableName, const QString& columnName, const QString& value) {
    QString path = getTablePath(tableName);
    QVector<QVector<QString>> tableData;

    if (!readTable(path, tableData)) return false;

    int colIndex = tableData[0].indexOf(columnName);
    if (colIndex == -1) return false;

    QVector<QString> newRow(tableData[0].size(), "");
    newRow[colIndex] = value;
    tableData.append(newRow);
    return writeTable(path, tableData);
}

bool Storage::insertRow(const QString& tableName, const QVector<QString>& columnNames, const QVector<QString>& values) {
    if (columnNames.size() != values.size()) return false;

    QString path = getTablePath(tableName);
    QVector<QVector<QString>> tableData;
    if (!readTable(path, tableData)) return false;

    QVector<QString> newRow(tableData[0].size(), "");
    for (int i = 0; i < columnNames.size(); ++i) {
        int idx = tableData[0].indexOf(columnNames[i]);
        if (idx == -1) return false;
        newRow[idx] = values[i];
    }

    tableData.append(newRow);
    return writeTable(path, tableData);
}

bool Storage::deleteTable(const QString& tableName) {
    QString path = getTablePath(tableName);
    QFile file(path);
    if (!file.exists()) return false;
    return file.remove();
}

bool Storage::selectWhere(const QString& tableName,
                          const QVector<QString>& selectedCols,
                          const QString& whereCol,
                          const QString& whereVal,
                          QVector<QVector<QString>>& result)
{
    QVector<QVector<QString>> table;
    if (!readTable(getTablePath(tableName), table) || table.size() < 2) return false;

    int whereIndex = table[0].indexOf(whereCol);
    if (whereIndex == -1) return false;

    QVector<int> selectedIndexes;
    if (selectedCols.isEmpty()) {
        for (int i = 0; i < table[0].size(); ++i) selectedIndexes.append(i);
    } else {
        for (const QString& col : selectedCols) {
            int idx = table[0].indexOf(col);
            if (idx == -1) return false;
            selectedIndexes.append(idx);
        }
    }

    result.clear();
    QVector<QString> header;
    for (int idx : selectedIndexes) header.append(table[0][idx]);
    result.append(header);

    QString whereType = (whereIndex < table[1].size()) ? table[1][whereIndex] : "TEXT";

    for (int i = 2; i < table.size(); ++i) {
        QString cellValue = table[i][whereIndex].trimmed();
        QString targetValue = whereVal.trimmed();
        bool match = false;

        if (whereType == "INT") {
            match = (cellValue.toInt() == targetValue.toInt());
        } else if (whereType == "FLOAT" || whereType == "REAL") {
            match = qFuzzyCompare(cellValue.toDouble() + 1, targetValue.toDouble() + 1);
        } else {
            match = (cellValue == targetValue);
        }

        if (match) {
            QVector<QString> row;
            for (int idx : selectedIndexes) row.append(table[i][idx]);
            result.append(row);
        }
    }

    return true;
}

bool Storage::updateRow(const QString& tableName,
                        const QString& targetCol,
                        const QString& newValue,
                        const QString& whereCol,
                        const QString& whereVal)
{
    QVector<QVector<QString>> table;
    if (!readTable(getTablePath(tableName), table) || table.size() < 2) return false;

    int whereIndex = table[0].indexOf(whereCol);
    int targetIndex = table[0].indexOf(targetCol);
    if (whereIndex == -1 || targetIndex == -1) return false;

    bool updated = false;
    for (int i = 2; i < table.size(); ++i) {
        if (table[i][whereIndex] == whereVal) {
            table[i][targetIndex] = newValue;
            updated = true;
        }
    }

    return updated && writeTable(getTablePath(tableName), table);
}

bool Storage::deleteRow(const QString& tableName, const QString& whereCol, const QString& whereVal) {
    QVector<QVector<QString>> table;
    if (!readTable(getTablePath(tableName), table)) return false;

    int colIndex = table[0].indexOf(whereCol);
    if (colIndex == -1) return false;

    bool deleted = false;
    for (int i = 2; i < table.size(); ) {
        if (table[i][colIndex] == whereVal) {
            table.remove(i);
            deleted = true;
        } else {
            ++i;
        }
    }

    return deleted && writeTable(getTablePath(tableName), table);
}

bool Storage::addColumn(const QString& tableName, const QString& columnName, const QString& columnType) {
    QVector<QVector<QString>> table;
    if (!readTable(getTablePath(tableName), table)) return false;

    table[0].append(columnName);
    table[1].append(columnType);
    for (int i = 2; i < table.size(); ++i)
        table[i].append("");

    return writeTable(getTablePath(tableName), table);
}

bool Storage::dropColumn(const QString& tableName, const QString& columnName) {
    QVector<QVector<QString>> table;
    if (!readTable(getTablePath(tableName), table)) return false;

    int colIndex = table[0].indexOf(columnName);
    if (colIndex == -1) return false;

    for (auto& row : table)
        row.remove(colIndex);

    return writeTable(getTablePath(tableName), table);
}

bool Storage::modifyColumn(const QString& tableName, const QString& columnName, const QString& newType) {
    QVector<QVector<QString>> table;
    if (!readTable(getTablePath(tableName), table)) return false;

    int colIndex = table[0].indexOf(columnName);
    if (colIndex == -1) return false;

    table[1][colIndex] = newType;
    return writeTable(getTablePath(tableName), table);
}

bool Storage::createDatabase(const QString& dbName) {
    QDir dir("/Users/hongzhe/mindata");
    if (!dir.exists()) return false;
    return dir.mkdir(dbName);
}

bool Storage::useDatabase(const QString& dbName) {
    QString path = "/Users/hongzhe/mindata/" + dbName;
    if (QDir(path).exists()) {
        currentDatabasePath = path;
        return true;
    }
    return false;
}

bool Storage::deleteDatabase(const QString& dbName) {
    return QDir("/Users/hongzhe/mindata/" + dbName).removeRecursively();
}
