#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QClipboard>
#include <QInputDialog>
#include <QDateTime>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_model(nullptr)
    , m_proxyModel(nullptr)
{
    ui->setupUi(this);

    // 设置窗口标题和图标
    setWindowTitle("SQLite 数据库查看器");

    // 连接信号槽
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onOpenDatabase);
    connect(ui->actionRefresh, &QAction::triggered, this, &MainWindow::onRefreshData);
    connect(ui->actionExit, &QAction::triggered, this, &::QMainWindow::close);
    connect(ui->actionExecute, &QAction::triggered, this, &MainWindow::onExecuteSQL);
    connect(ui->actionExport, &QAction::triggered, this, &MainWindow::onExportData);
    connect(ui->actionStatistics, &QAction::triggered, this, &MainWindow::onShowStats);
    connect(ui->actionBackup, &QAction::triggered, this, &MainWindow::onBackupDatabase);

    connect(ui->btnExecute, &QPushButton::clicked, this, &MainWindow::onExecuteSQL);
    connect(ui->btnRefresh, &QPushButton::clicked, this, &MainWindow::onRefreshData);
    connect(ui->btnAddUser, &QPushButton::clicked, this, &MainWindow::onAddUser);
    connect(ui->btnDeleteUser, &QPushButton::clicked, this, &MainWindow::onDeleteUser);
    connect(ui->btnFilter, &QPushButton::clicked, this, &MainWindow::onFilterChanged);

    connect(ui->lineFilter, &QLineEdit::textChanged, this, &MainWindow::onFilterChanged);

    // 初始化界面
    setupTableView();

    // 尝试自动打开当前目录下的数据库
    QString defaultDb = QDir::currentPath() + "/user_data.db";
    if (QFile::exists(defaultDb)) {
        loadDatabase(defaultDb);
    }
}

MainWindow::~MainWindow()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    delete ui;
}

void MainWindow::setupTableView()
{
    // 初始化表格视图
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->verticalHeader()->setDefaultSectionSize(24);
}

void MainWindow::loadDatabase(const QString& path)
{
    // 关闭现有连接
    if (m_db.isOpen()) {
        m_db.close();
    }

    // 创建新的数据库连接
    QString connectionName = QString("viewer_connection_%1").arg(QDateTime::currentMSecsSinceEpoch());
    m_db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    m_db.setDatabaseName(path);

    if (!m_db.open()) {
        QMessageBox::critical(this, "错误",
            QString("无法打开数据库文件:\n%1\n\n错误信息: %2")
            .arg(path)
            .arg(m_db.lastError().text()));
        return;
    }

    // 设置数据模型
    if (m_model) {
        delete m_model;
    }
    m_model = new QSqlTableModel(this, m_db);
    m_model->setTable("users");
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_model->select();

    // 设置列标题
    m_model->setHeaderData(0, Qt::Horizontal, "ID");
    m_model->setHeaderData(1, Qt::Horizontal, "用户名");
    m_model->setHeaderData(2, Qt::Horizontal, "密码");
    m_model->setHeaderData(3, Qt::Horizontal, "邮箱");
    m_model->setHeaderData(4, Qt::Horizontal, "最高分");
    m_model->setHeaderData(5, Qt::Horizontal, "创建时间");

    // 设置代理模型用于过滤和排序
    if (m_proxyModel) {
        delete m_proxyModel;
    }
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setFilterKeyColumn(-1); // 搜索所有列
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->tableView->setModel(m_proxyModel);
    ui->tableView->resizeColumnsToContents();

    // 设置窗口标题
    setWindowTitle(QString("SQLite 数据库查看器 - %1").arg(QFileInfo(path).fileName()));

    // 更新状态栏
    updateStatusBar();

    // 检查并显示表结构
    QSqlQuery query(m_db);
    if (query.exec("PRAGMA table_info(users)")) {
        QStringList columns;
        while (query.next()) {
            columns << query.value(1).toString();
        }
        ui->textInfo->appendPlainText("表结构: " + columns.join(", "));
    }
}

void MainWindow::onOpenDatabase()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "选择数据库文件",
        QDir::currentPath(),
        "SQLite 数据库 (*.db *.sqlite *.sqlite3);;所有文件 (*.*)");

    if (!fileName.isEmpty()) {
        loadDatabase(fileName);
    }
}

void MainWindow::onRefreshData()
{
    if (m_model) {
        m_model->select();
        ui->tableView->resizeColumnsToContents();
        updateStatusBar();

        ui->textInfo->appendPlainText(QString("数据已刷新 - %1")
            .arg(QDateTime::currentDateTime().toString("HH:mm:ss")));
    }
}

void MainWindow::onExecuteSQL()
{
    QString sql = ui->textSQL->toPlainText().trimmed();
    if (sql.isEmpty()) {
        return;
    }

    QSqlQuery query(m_db);
    query.prepare(sql);

    if (query.exec()) {
        // 如果是 SELECT 查询，显示结果
        if (sql.trimmed().left(6).toUpper() == "SELECT") {
            QString result;
            result += "查询成功!\n";
            result += "结果集: " + QString::number(query.size()) + " 行\n";

            int row = 0;
            while (query.next() && row < 100) { // 限制显示100行
                result += QString("第%1行: ").arg(row + 1);
                for (int i = 0; i < query.record().count(); i++) {
                    result += query.value(i).toString() + " | ";
                }
                result += "\n";
                row++;
            }

            if (query.size() > 100) {
                result += QString("... (只显示前100行，共%1行)").arg(query.size());
            }

            ui->textInfo->setPlainText(result);
        }
        else {
            ui->textInfo->setPlainText(QString("执行成功！影响行数: %1")
                .arg(query.numRowsAffected()));
        }

        // 如果是修改数据的查询，刷新表格
        if (sql.trimmed().left(6).toUpper() != "SELECT") {
            onRefreshData();
        }
    }
    else {
        ui->textInfo->setPlainText("执行失败: " + query.lastError().text());
    }
}

void MainWindow::onExportData()
{
    if (!m_model) return;

    QString fileName = QFileDialog::getSaveFileName(this,
        "导出数据",
        QDir::currentPath() + "/users_export.csv",
        "CSV 文件 (*.csv);;文本文件 (*.txt)");

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法创建文件");
        return;
    }

    QTextStream stream(&file);

    // 写入表头
    for (int col = 0; col < m_model->columnCount(); col++) {
        stream << m_model->headerData(col, Qt::Horizontal).toString();
        if (col < m_model->columnCount() - 1) {
            stream << ",";
        }
    }
    stream << "\n";

    // 写入数据
    for (int row = 0; row < m_model->rowCount(); row++) {
        for (int col = 0; col < m_model->columnCount(); col++) {
            QModelIndex index = m_model->index(row, col);
            QString value = m_model->data(index).toString();
            // 处理包含逗号或引号的字段
            if (value.contains(',') || value.contains('"')) {
                value = "\"" + value.replace("\"", "\"\"") + "\"";
            }
            stream << value;
            if (col < m_model->columnCount() - 1) {
                stream << ",";
            }
        }
        stream << "\n";
    }

    file.close();
    QMessageBox::information(this, "成功", QString("数据已导出到: %1").arg(fileName));
}

void MainWindow::onAddUser()
{
    bool ok;
    QString username = QInputDialog::getText(this, "添加用户", "用户名:",
        QLineEdit::Normal, "", &ok);
    if (!ok || username.isEmpty()) return;

    QString password = QInputDialog::getText(this, "添加用户", "密码:",
        QLineEdit::Normal, "", &ok);
    if (!ok || password.isEmpty()) return;

    QString email = QInputDialog::getText(this, "添加用户", "邮箱:",
        QLineEdit::Normal, "", &ok);
    if (!ok) return;

    QString score = QInputDialog::getText(this, "添加用户", "初始分数:",
        QLineEdit::Normal, "0", &ok);
    if (!ok) return;

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO users (username, password, email, high_score) "
        "VALUES (:username, :password, :email, :score)");
    query.bindValue(":username", username);
    query.bindValue(":password", password);
    query.bindValue(":email", email);
    query.bindValue(":score", score.toInt());

    if (query.exec()) {
        onRefreshData();
        ui->textInfo->appendPlainText(QString("用户 %1 添加成功").arg(username));
    }
    else {
        QMessageBox::warning(this, "错误", "添加失败: " + query.lastError().text());
    }
}

void MainWindow::onDeleteUser()
{
    QModelIndexList selected = ui->tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择一行");
        return;
    }

    QModelIndex proxyIndex = selected.first();
    QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    int row = sourceIndex.row();

    // 获取用户名
    QString username = m_model->index(row, 1).data().toString();

    if (confirmAction("确认删除", QString("确定要删除用户 '%1' 吗？").arg(username))) {
        QSqlQuery query(m_db);
        query.prepare("DELETE FROM users WHERE id = :id");
        query.bindValue(":id", m_model->index(row, 0).data().toInt());

        if (query.exec()) {
            onRefreshData();
            ui->textInfo->appendPlainText(QString("用户 %1 已删除").arg(username));
        }
        else {
            QMessageBox::warning(this, "错误", "删除失败: " + query.lastError().text());
        }
    }
}

void MainWindow::onFilterChanged()
{
    if (m_proxyModel) {
        m_proxyModel->setFilterWildcard(ui->lineFilter->text());
        updateStatusBar();
    }
}

void MainWindow::onShowStats()
{
    if (!m_db.isOpen()) return;

    QSqlQuery query(m_db);
    QString stats;

    // 总用户数
    query.exec("SELECT COUNT(*) FROM users");
    if (query.next()) {
        stats += QString("总用户数: %1\n").arg(query.value(0).toInt());
    }

    // 平均分
    query.exec("SELECT AVG(high_score) FROM users");
    if (query.next()) {
        stats += QString("平均分数: %.1f\n").arg(query.value(0).toDouble());
    }

    // 最高分
    query.exec("SELECT MAX(high_score) FROM users");
    if (query.next()) {
        stats += QString("最高分数: %1\n").arg(query.value(0).toInt());
    }

    // 最低分
    query.exec("SELECT MIN(high_score) FROM users");
    if (query.next()) {
        stats += QString("最低分数: %1\n").arg(query.value(0).toInt());
    }

    // 最近创建的用户
    query.exec("SELECT username, created_at FROM users ORDER BY created_at DESC LIMIT 1");
    if (query.next()) {
        stats += QString("最近用户: %1 (%2)\n")
            .arg(query.value(0).toString())
            .arg(query.value(1).toString());
    }

    // 数据库信息
    query.exec("PRAGMA database_list");
    while (query.next()) {
        stats += QString("数据库: %1\n").arg(query.value(2).toString());
    }

    query.exec("PRAGMA page_size");
    if (query.next()) {
        stats += QString("页大小: %1\n").arg(query.value(0).toInt());
    }

    query.exec("PRAGMA page_count");
    if (query.next()) {
        stats += QString("总页数: %1\n").arg(query.value(0).toInt());
    }

    ui->textInfo->setPlainText("=== 数据库统计 ===\n" + stats);
}

void MainWindow::onBackupDatabase()
{
    if (!m_db.isOpen()) return;

    QString backupPath = QFileDialog::getSaveFileName(this,
        "备份数据库",
        QDir::currentPath() + "/user_data_backup_" +
        QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".db",
        "SQLite 数据库 (*.db)");

    if (backupPath.isEmpty()) return;

    // 关闭数据库连接
    QString dbPath = m_db.databaseName();
    m_db.close();

    // 复制文件
    if (QFile::copy(dbPath, backupPath)) {
        QMessageBox::information(this, "成功",
            QString("数据库已备份到:\n%1").arg(backupPath));
    }
    else {
        QMessageBox::critical(this, "错误", "备份失败");
    }

    // 重新打开数据库
    m_db.open();
}

void MainWindow::updateStatusBar()
{
    if (m_model) {
        int totalRows = m_model->rowCount();
        int filteredRows = m_proxyModel ? m_proxyModel->rowCount() : totalRows;

        QString status = QString("总记录: %1 | 筛选后: %2")
            .arg(totalRows)
            .arg(filteredRows);

        if (m_db.isOpen()) {
            status += " | 已连接";
        }

        ui->statusbar->showMessage(status);
    }
}

bool MainWindow::confirmAction(const QString& title, const QString& message)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, title, message,
        QMessageBox::Yes | QMessageBox::No);
    return (reply == QMessageBox::Yes);
}