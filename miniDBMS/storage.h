#ifndef STORAGE_H
#define STORAGE_H

#include <QString>
#include <QVector>

class Storage {
public:
    Storage();

    // 表操作
    bool createTable(const QString& tableName, const QVector<QString>& columnNames, const QVector<QString>& columnTypes);
    bool insertValue(const QString& tableName, const QString& columnName, const QString& value);
    bool insertRow(const QString& tableName, const QVector<QString>& columnNames, const QVector<QString>& values);
    bool deleteTable(const QString& tableName);
    bool selectWhere(const QString& tableName, const QVector<QString>& selectedCols,
                     const QString& whereCol, const QString& whereVal, QVector<QVector<QString>>& result);
    bool updateRow(const QString& tableName, const QString& targetCol, const QString& newValue,
                   const QString& whereCol, const QString& whereVal);
    bool deleteRow(const QString& tableName, const QString& whereCol, const QString& whereVal);

    // 列操作
    bool addColumn(const QString& tableName, const QString& columnName, const QString& columnType);
    bool dropColumn(const QString& tableName, const QString& columnName);
    bool modifyColumn(const QString& tableName, const QString& columnName, const QString& newType);

    // 数据库操作
    bool createDatabase(const QString& dbName);
    bool useDatabase(const QString& dbName);
    bool deleteDatabase(const QString& dbName);

    // 辅助函数
    QString getTablePath(const QString& tableName) const;
    QString getCurrentDatabasePath() const;
    QString currentDatabasePath;


    bool readTable(const QString& filePath, QVector<QVector<QString>>& tableData) const;
    bool writeTable(const QString& filePath, const QVector<QVector<QString>>& tableData) const;
};

#endif // STORAGE_H
