#include "PageAdmin.h"
#include "MainWindow.h"
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QBrush>
#include <QFont>
#include <QAbstractItemView>
#include <QFile>
#include <QStatusTipEvent>

QMessageBox* PageAdmin::createStyledMessageBox(const QString& title, const QString& text,
    QMessageBox::Icon icon) {
    QMessageBox* msgBox = new QMessageBox(this);
    msgBox->setWindowTitle(title);
    msgBox->setText(text);
    msgBox->setIcon(icon);

    // 设置样式
    QString styleSheet =
        "QMessageBox {"
        "   background-color: white;"
        "}"
        "QMessageBox QLabel {"
        "   color: black;"
        "   font-size: 14px;"
        "}"
        "QMessageBox QPushButton {"
        "   background-color: #3498db;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   padding: 8px 16px;"
        "   min-width: 80px;"
        "}"
        "QMessageBox QPushButton:hover {"
        "   background-color: #2980b9;"
        "}";

    msgBox->setStyleSheet(styleSheet);

    return msgBox;
}

PageAdmin::PageAdmin(MainWindow* mainWin)
    : QWidget(mainWin), m_mainWin(mainWin), m_currentSelectedUser("")
{
    setupDatabase();
    setupUI();
    loadUserData();
}

PageAdmin::~PageAdmin()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

void PageAdmin::setupDatabase()
{
    // 创建独立的数据库连接，避免与UserManager冲突
    m_db = QSqlDatabase::addDatabase("QSQLITE", "admin_connection");
    QString dbPath = QDir::currentPath() + "/user_data.db";
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        QMessageBox::critical(this, "数据库错误",
            QString("无法打开数据库：%1").arg(m_db.lastError().text()));
    }
}

void PageAdmin::setupUI()
{
    // 设置窗口背景
    setStyleSheet("background-color: #f0f0f0;");

    // 主布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(15);

    // ==================== 顶部标题栏 ====================
    QWidget* titleBar = new QWidget();
    titleBar->setStyleSheet("background-color: #2c3e50; border-radius: 10px;");
    QHBoxLayout* titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(20, 10, 20, 10);

    m_titleLabel = new QLabel("🧑‍💼 用户管理系统");
    m_titleLabel->setStyleSheet("color: white; font-size: 24px; font-weight: bold;");

    titleLayout->addWidget(m_titleLabel);
    titleLayout->addStretch();

    m_btnBack = new QPushButton("返回登录页");
    m_btnBack->setStyleSheet(
        "QPushButton {"
        "   background-color: #e74c3c;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 8px 16px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #c0392b;"
        "}"
    );
    QObject::connect(m_btnBack, &QPushButton::clicked, [this]() {
        if (m_mainWin) {
            m_mainWin->switchPage(0); // 返回登录页
        }
        });
    titleLayout->addWidget(m_btnBack);

    m_mainLayout->addWidget(titleBar);

    // ==================== 搜索栏 ====================
    QWidget* searchBar = new QWidget();
    searchBar->setStyleSheet("background-color: white; border-radius: 10px; padding: 15px;");
    QHBoxLayout* searchLayout = new QHBoxLayout(searchBar);

    m_searchLabel = new QLabel("搜索用户：");
    m_searchLabel->setStyleSheet("font-weight: bold; font-size: 14px;");

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("输入用户名或邮箱...");
    m_searchEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 2px solid #bdc3c7;"
        "   border-radius: 8px;"
        "   padding: 8px;"
        "   font-size: 14px;"
        "}"
        "QLineEdit:focus {"
        "   border-color: #3498db;"
        "}"
    );
    m_searchEdit->setMinimumWidth(300);

    m_filterCombo = new QComboBox();
    m_filterCombo->addItems({ "所有用户", "简单模式玩家", "普通模式玩家", "困难模式玩家" });
    m_filterCombo->setStyleSheet(
        "QComboBox {"
        "   border: 2px solid #bdc3c7;"
        "   border-radius: 8px;"
        "   padding: 8px;"
        "   font-size: 14px;"
        "}"
    );

    m_showInactiveCheck = new QCheckBox("显示零分用户");
    m_showInactiveCheck->setStyleSheet("font-weight: bold;");

    m_btnSearch = new QPushButton("🔍 搜索");
    m_btnSearch->setStyleSheet(
        "QPushButton {"
        "   background-color: #3498db;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 8px 20px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #2980b9;"
        "}"
    );
    QObject::connect(m_btnSearch, &QPushButton::clicked, this, &PageAdmin::searchUsers);

    m_btnClear = new QPushButton("清除");
    m_btnClear->setStyleSheet(
        "QPushButton {"
        "   background-color: #95a5a6;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 8px 20px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #7f8c8d;"
        "}"
    );
    QObject::connect(m_btnClear, &QPushButton::clicked, this, &PageAdmin::clearSearch);

    searchLayout->addWidget(m_searchLabel);
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_filterCombo);
    searchLayout->addWidget(m_showInactiveCheck);
    searchLayout->addWidget(m_btnSearch);
    searchLayout->addWidget(m_btnClear);
    searchLayout->addStretch();

    m_mainLayout->addWidget(searchBar);

    // ==================== 按钮栏 ====================
    m_buttonLayout = new QHBoxLayout();

    m_btnRefresh = new QPushButton("🔄 刷新");
    m_btnRefresh->setStyleSheet(
        "QPushButton {"
        "   background-color: #2ecc71;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 8px 20px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #27ae60;"
        "}"
    );
    QObject::connect(m_btnRefresh, &QPushButton::clicked, this, &PageAdmin::refreshTable);

    m_btnDelete = new QPushButton("🗑️ 删除选中");
    m_btnDelete->setStyleSheet(
        "QPushButton {"
        "   background-color: #e74c3c;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 8px 20px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #c0392b;"
        "}"
    );
    QObject::connect(m_btnDelete, &QPushButton::clicked, this, &PageAdmin::deleteSelectedUser);

    m_btnDeleteAll = new QPushButton("⚠️ 清空所有");
    m_btnDeleteAll->setStyleSheet(
        "QPushButton {"
        "   background-color: #d35400;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 8px 20px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #a35200;"
        "}"
    );
    QObject::connect(m_btnDeleteAll, &QPushButton::clicked, this, &PageAdmin::deleteAllUsers);

    m_btnResetPwd = new QPushButton("🔑 重置密码");
    m_btnResetPwd->setStyleSheet(
        "QPushButton {"
        "   background-color: #f39c12;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 8px 20px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #d68910;"
        "}"
    );
    QObject::connect(m_btnResetPwd, &QPushButton::clicked, this, &PageAdmin::resetUserPassword);

    m_btnExport = new QPushButton("📊 导出CSV");
    m_btnExport->setStyleSheet(
        "QPushButton {"
        "   background-color: #9b59b6;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 8px 20px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #8e44ad;"
        "}"
    );
    QObject::connect(m_btnExport, &QPushButton::clicked, this, &PageAdmin::exportToCSV);

    m_btnStats = new QPushButton("📈 统计数据");
    m_btnStats->setStyleSheet(
        "QPushButton {"
        "   background-color: #1abc9c;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 8px 20px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #16a085;"
        "}"
    );
    QObject::connect(m_btnStats, &QPushButton::clicked, this, &PageAdmin::showStatistics);

    m_buttonLayout->addWidget(m_btnRefresh);
    m_buttonLayout->addWidget(m_btnDelete);
    m_buttonLayout->addWidget(m_btnDeleteAll);
    m_buttonLayout->addWidget(m_btnResetPwd);
    m_buttonLayout->addWidget(m_btnExport);
    m_buttonLayout->addWidget(m_btnStats);
    m_buttonLayout->addStretch();

    m_mainLayout->addLayout(m_buttonLayout);

    // ==================== 用户表格 ====================
    m_table = new QTableWidget();
    setupTable();
    m_mainLayout->addWidget(m_table, 1); // 设置伸缩因子为1，让表格占据更多空间

    // ==================== 用户详情面板 ====================
    m_detailPanel = new QWidget();
    m_detailPanel->setStyleSheet(
        "background-color: white;"
        "border: 2px solid #3498db;"
        "border-radius: 10px;"
        "padding: 15px;"
    );
    m_detailPanel->setVisible(false); // 初始隐藏

    m_detailLayout = new QVBoxLayout(m_detailPanel);

    QLabel* detailTitle = new QLabel("👤 用户详细信息");
    detailTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; margin-bottom: 10px;");
    m_detailLayout->addWidget(detailTitle);

    m_detailUsername = new QLabel("用户名：");
    m_detailEmail = new QLabel("邮箱：");
    m_detailEasyScore = new QLabel("简单模式最高分：");
    m_detailNormalScore = new QLabel("普通模式最高分：");
    m_detailHardScore = new QLabel("困难模式最高分：");
    m_detailCreatedAt = new QLabel("注册时间：");

    QList<QLabel*> detailLabels = {
        m_detailUsername, m_detailEmail, m_detailEasyScore,
        m_detailNormalScore, m_detailHardScore, m_detailCreatedAt
    };

    for (QLabel* label : detailLabels) {
        label->setStyleSheet("font-size: 14px; margin: 5px 0;");
        m_detailLayout->addWidget(label);
    }

    m_detailLayout->addStretch();
    m_mainLayout->addWidget(m_detailPanel);

    // 连接表格点击信号
    QObject::connect(m_table, &QTableWidget::cellClicked, this, &PageAdmin::onCellClicked);
}

void PageAdmin::setupTable()
{
    // 设置表格列数
    QStringList headers = {
        "ID", "用户名", "邮箱", "简单最高分", "普通最高分",
        "困难最高分", "注册时间", "操作"
    };
    m_table->setColumnCount(headers.size());
    m_table->setHorizontalHeaderLabels(headers);

    // 设置表格样式
    m_table->setStyleSheet(
        "QTableWidget {"
        "   background-color: white;"
        "   border: 2px solid #bdc3c7;"
        "   border-radius: 10px;"
        "   gridline-color: #ecf0f1;"
        "}"
        "QTableWidget::item {"
        "   padding: 8px;"
        "}"
        "QTableWidget::item:selected {"
        "   background-color: #3498db;"
        "   color: white;"
        "}"
        "QHeaderView::section {"
        "   background-color: #2c3e50;"
        "   color: white;"
        "   padding: 8px;"
        "   border: none;"
        "   font-weight: bold;"
        "}"
    );

    // 设置表格属性
    m_table->setAlternatingRowColors(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);

    // 设置列宽
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setColumnWidth(0, 50);   // ID
    m_table->setColumnWidth(1, 150);  // 用户名
    m_table->setColumnWidth(2, 200);  // 邮箱
    m_table->setColumnWidth(3, 120);  // 简单最高分
    m_table->setColumnWidth(4, 120);  // 普通最高分
    m_table->setColumnWidth(5, 120);  // 困难最高分
    m_table->setColumnWidth(6, 180);  // 注册时间
}

void PageAdmin::loadUserData()
{
    if (!m_db.isOpen()) {
        QMessageBox::warning(this, "错误", "数据库未连接！");
        return;
    }

    // 清除现有数据
    m_table->setRowCount(0);

    QList<QStringList> users = getAllUsers();

    // 填充表格
    for (int i = 0; i < users.size(); i++) {
        const QStringList& userData = users[i];

        m_table->insertRow(i);

        // 填充数据
        for (int j = 0; j < userData.size() && j < m_table->columnCount() - 1; j++) {
            QTableWidgetItem* item = new QTableWidgetItem(userData[j]);
            item->setTextAlignment(Qt::AlignCenter);

            // 为分数列设置特殊颜色
            if (j >= 3 && j <= 5) { // 分数列
                int score = userData[j].toInt();
                if (score > 0) {
                    item->setForeground(QBrush(QColor(46, 204, 113))); // 绿色
                    item->setFont(QFont("Arial", 10, QFont::Bold));
                }
                else {
                    item->setForeground(QBrush(QColor(149, 165, 166))); // 灰色
                }
            }

            m_table->setItem(i, j, item);
        }

        // 添加操作按钮
        QWidget* widget = new QWidget();
        QHBoxLayout* layout = new QHBoxLayout(widget);
        layout->setContentsMargins(5, 5, 5, 5);
        layout->setSpacing(5);

        QPushButton* btnDetails = new QPushButton("详情");
        btnDetails->setStyleSheet(
            "QPushButton {"
            "   background-color: #3498db;"
            "   color: white;"
            "   border: none;"
            "   border-radius: 5px;"
            "   padding: 5px 10px;"
            "   font-size: 12px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #2980b9;"
            "}"
        );

        QObject::connect(btnDetails, &QPushButton::clicked, [this, userData]() {
            m_currentSelectedUser = userData[1];
            showUserDetails();
            });

        layout->addWidget(btnDetails);
        layout->addStretch();

        m_table->setCellWidget(i, m_table->columnCount() - 1, widget);
    }

    // 更新状态栏
    m_table->setStatusTip(QString("共 %1 条记录").arg(users.size()));
}

QList<QStringList> PageAdmin::getAllUsers()
{
    QList<QStringList> users;

    QString sql = "SELECT id, username, email, "
        "easy_high_score, normal_high_score, hard_high_score, "
        "created_at FROM users";

    // 根据复选框决定是否显示零分用户
    if (!m_showInactiveCheck->isChecked()) {
        sql += " WHERE (easy_high_score > 0 OR normal_high_score > 0 OR hard_high_score > 0)";
    }

    // 根据筛选器添加条件
    int filterIndex = m_filterCombo->currentIndex();
    if (filterIndex > 0) {
        QString condition = (sql.contains("WHERE") ? " AND " : " WHERE ");
        switch (filterIndex) {
        case 1: // 简单模式玩家
            condition += "easy_high_score > 0";
            break;
        case 2: // 普通模式玩家
            condition += "normal_high_score > 0";
            break;
        case 3: // 困难模式玩家
            condition += "hard_high_score > 0";
            break;
        }
        sql += condition;
    }

    sql += " ORDER BY created_at DESC";

    QSqlQuery query(sql, m_db);

    while (query.next()) {
        QStringList user;
        user << query.value(0).toString();      // id
        user << query.value(1).toString();      // username
        user << query.value(2).toString();      // email
        user << query.value(3).toString();      // easy_high_score
        user << query.value(4).toString();      // normal_high_score
        user << query.value(5).toString();      // hard_high_score

        // 格式化时间
        QDateTime createTime = QDateTime::fromString(query.value(6).toString(), Qt::ISODate);
        QString timeStr = createTime.toString("yyyy-MM-dd hh:mm:ss");
        user << timeStr;

        users.append(user);
    }

    return users;
}

void PageAdmin::searchUsers()
{
    loadUserData();
}

void PageAdmin::clearSearch()
{
    m_searchEdit->clear();
    m_filterCombo->setCurrentIndex(0);
    m_showInactiveCheck->setChecked(false);
    loadUserData();
}

void PageAdmin::deleteSelectedUser()
{
    int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "警告", "请先选择要删除的用户！");
        return;
    }

    QString username = m_table->item(row, 1)->text();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除",
        QString("确定要删除用户 '%1' 吗？此操作不可恢复！").arg(username),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (deleteUser(username)) {
            QMessageBox::information(this, "成功", "用户删除成功！");
            loadUserData();
        }
        else {
            QMessageBox::critical(this, "错误", "删除失败！");
        }
    }
}

bool PageAdmin::deleteUser(const QString& username)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM users WHERE username = :username");
    query.bindValue(":username", username);

    return query.exec();
}

void PageAdmin::deleteAllUsers()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::critical(this, "⚠️ 危险操作",
        "确定要删除所有用户数据吗？\n"
        "此操作将清空所有用户记录，包括最高分！\n"
        "此操作不可恢复！",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QSqlQuery query(m_db);
        if (query.exec("DELETE FROM users")) {
            QMessageBox::information(this, "成功", "所有用户数据已清空！");
            loadUserData();
        }
        else {
            QMessageBox::critical(this, "错误", "清空失败！");
        }
    }
}

void PageAdmin::resetUserPassword()
{
    if (m_currentSelectedUser.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择一个用户！");
        return;
    }

    bool ok;
    QString newPassword = QInputDialog::getText(this, "重置密码",
        QString("为用户 '%1' 设置新密码：").arg(m_currentSelectedUser),
        QLineEdit::Password, "", &ok);

    if (ok && !newPassword.isEmpty()) {
        if (resetPassword(m_currentSelectedUser, newPassword)) {
            QMessageBox::information(this, "成功", "密码重置成功！");
        }
        else {
            QMessageBox::critical(this, "错误", "密码重置失败！");
        }
    }
}

bool PageAdmin::resetPassword(const QString& username, const QString& newPassword)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE users SET password = :password WHERE username = :username");
    query.bindValue(":password", newPassword);
    query.bindValue(":username", username);

    return query.exec();
}

void PageAdmin::exportToCSV()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出CSV",
        QString("用户数据_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "CSV文件 (*.csv)");

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法创建文件！");
        return;
    }

    QTextStream stream(&file);

    // 写入表头
    QStringList headers;
    for (int i = 0; i < m_table->columnCount() - 1; i++) { // -1 排除操作列
        headers << m_table->horizontalHeaderItem(i)->text();
    }
    stream << headers.join(",") << "\n";

    // 写入数据
    for (int i = 0; i < m_table->rowCount(); i++) {
        QStringList rowData;
        for (int j = 0; j < m_table->columnCount() - 1; j++) { // -1 排除操作列
            QTableWidgetItem* item = m_table->item(i, j);
            if (item) {
                // CSV特殊字符处理
                QString text = item->text();
                if (text.contains(',') || text.contains('"') || text.contains('\n')) {
                    text = "\"" + text.replace("\"", "\"\"") + "\"";
                }
                rowData << text;
            }
            else {
                rowData << "";
            }
        }
        stream << rowData.join(",") << "\n";
    }

    file.close();

    QMessageBox::information(this, "导出成功",
        QString("数据已成功导出到：\n%1").arg(fileName));

    // 询问是否打开文件
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "打开文件", "是否现在打开导出的CSV文件？",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
    }
}

void PageAdmin::refreshTable()
{
    loadUserData();
}

void PageAdmin::onCellClicked(int row, int column)
{
    if (row >= 0 && row < m_table->rowCount()) {
        m_currentSelectedUser = m_table->item(row, 1)->text();

        // 高亮选中的行
        for (int i = 0; i < m_table->rowCount(); i++) {
            for (int j = 0; j < m_table->columnCount(); j++) {
                QTableWidgetItem* item = m_table->item(i, j);
                if (item) {
                    item->setBackground(i == row ? QBrush(QColor(52, 152, 219, 50)) : QBrush(Qt::white));
                }
            }
        }
    }
}

void PageAdmin::showUserDetails()
{
    if (m_currentSelectedUser.isEmpty()) return;

    QSqlQuery query(m_db);
    query.prepare(
        "SELECT username, email, easy_high_score, normal_high_score, "
        "hard_high_score, easy_recent_scores, normal_recent_scores, "
        "hard_recent_scores, created_at FROM users WHERE username = :username"
    );
    query.bindValue(":username", m_currentSelectedUser);

    if (query.exec() && query.next()) {
        // 显示详情面板
        m_detailPanel->setVisible(true);

        // 填充数据
        m_detailUsername->setText(QString("用户名：%1").arg(query.value(0).toString()));
        m_detailEmail->setText(QString("邮箱：%1").arg(query.value(1).toString()));
        m_detailEasyScore->setText(QString("简单模式最高分：%1").arg(query.value(2).toInt()));
        m_detailNormalScore->setText(QString("普通模式最高分：%1").arg(query.value(3).toInt()));
        m_detailHardScore->setText(QString("困难模式最高分：%1").arg(query.value(4).toInt()));

        // 解析最近得分
        QString easyScores = query.value(5).toString();
        QString normalScores = query.value(6).toString();
        QString hardScores = query.value(7).toString();

        // 计算最近10场的平均分
        auto calcAverage = [](const QString& jsonStr) -> double {
            QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
            QJsonArray array = doc.array();

            if (array.isEmpty()) return 0.0;

            double sum = 0.0;
            for (const auto& value : array) {
                sum += value.toInt();
            }

            return sum / array.size();
            };

        double easyAvg = calcAverage(easyScores);
        double normalAvg = calcAverage(normalScores);
        double hardAvg = calcAverage(hardScores);

        // 格式化注册时间
        QDateTime createTime = QDateTime::fromString(query.value(8).toString(), Qt::ISODate);
        QString timeStr = createTime.toString("yyyy年MM月dd日 hh:mm:ss");
        m_detailCreatedAt->setText(QString("注册时间：%1").arg(timeStr));

        // 添加额外信息
        QJsonDocument easyDoc = QJsonDocument::fromJson(easyScores.toUtf8());
        QJsonArray easyArray = easyDoc.array();

        if (m_detailLayout->count() > 7) {
            // 移除旧的额外信息
            while (m_detailLayout->count() > 7) {
                QLayoutItem* item = m_detailLayout->takeAt(7);
                if (item) {
                    delete item->widget();
                    delete item;
                }
            }
        }

        // 添加最近得分信息
        if (!easyArray.isEmpty()) {
            QLabel* recentLabel = new QLabel("📊 最近得分统计：");
            recentLabel->setStyleSheet("font-weight: bold; color: #2c3e50; margin-top: 10px;");
            m_detailLayout->insertWidget(7, recentLabel);

            QLabel* easyAvgLabel = new QLabel(QString("• 简单模式平均分：%1").arg(easyAvg, 0, 'f', 1));
            easyAvgLabel->setStyleSheet("color: #27ae60;");
            m_detailLayout->insertWidget(8, easyAvgLabel);

            QLabel* normalAvgLabel = new QLabel(QString("• 普通模式平均分：%1").arg(normalAvg, 0, 'f', 1));
            normalAvgLabel->setStyleSheet("color: #f39c12;");
            m_detailLayout->insertWidget(9, normalAvgLabel);

            QLabel* hardAvgLabel = new QLabel(QString("• 困难模式平均分：%1").arg(hardAvg, 0, 'f', 1));
            hardAvgLabel->setStyleSheet("color: #e74c3c;");
            m_detailLayout->insertWidget(10, hardAvgLabel);
        }
    }
}

void PageAdmin::showStatistics()
{
    if (!m_db.isOpen()) {
        QMessageBox::warning(this, "错误", "数据库未连接！");
        return;
    }

    // 统计查询
    QSqlQuery totalQuery("SELECT COUNT(*) FROM users", m_db);
    QSqlQuery activeQuery("SELECT COUNT(*) FROM users WHERE easy_high_score > 0 OR normal_high_score > 0 OR hard_high_score > 0", m_db);
    QSqlQuery easyQuery("SELECT COUNT(*) FROM users WHERE easy_high_score > 0", m_db);
    QSqlQuery normalQuery("SELECT COUNT(*) FROM users WHERE normal_high_score > 0", m_db);
    QSqlQuery hardQuery("SELECT COUNT(*) FROM users WHERE hard_high_score > 0", m_db);
    QSqlQuery topEasyQuery("SELECT username, easy_high_score FROM users WHERE easy_high_score > 0 ORDER BY easy_high_score DESC LIMIT 1", m_db);
    QSqlQuery topNormalQuery("SELECT username, normal_high_score FROM users WHERE normal_high_score > 0 ORDER BY normal_high_score DESC LIMIT 1", m_db);
    QSqlQuery topHardQuery("SELECT username, hard_high_score FROM users WHERE hard_high_score > 0 ORDER BY hard_high_score DESC LIMIT 1", m_db);

    int totalUsers = 0, activeUsers = 0, easyPlayers = 0, normalPlayers = 0, hardPlayers = 0;
    QString topEasyUser, topNormalUser, topHardUser;
    int topEasyScore = 0, topNormalScore = 0, topHardScore = 0;

    if (totalQuery.next()) totalUsers = totalQuery.value(0).toInt();
    if (activeQuery.next()) activeUsers = activeQuery.value(0).toInt();
    if (easyQuery.next()) easyPlayers = easyQuery.value(0).toInt();
    if (normalQuery.next()) normalPlayers = normalQuery.value(0).toInt();
    if (hardQuery.next()) hardPlayers = hardQuery.value(0).toInt();

    if (topEasyQuery.next()) {
        topEasyUser = topEasyQuery.value(0).toString();
        topEasyScore = topEasyQuery.value(1).toInt();
    }
    if (topNormalQuery.next()) {
        topNormalUser = topNormalQuery.value(0).toString();
        topNormalScore = topNormalQuery.value(1).toInt();
    }
    if (topHardQuery.next()) {
        topHardUser = topHardQuery.value(0).toString();
        topHardScore = topHardQuery.value(1).toInt();
    }

    // 计算百分比
    double activePercent = totalUsers > 0 ? (activeUsers * 100.0 / totalUsers) : 0;

    // 创建统计信息对话框
    QString statsText = QString(
        "📊 用户数据统计\n"
        "===============================\n"
        "总用户数：%1 人\n"
        "活跃用户：%2 人 (%3%)\n"
        "简单模式玩家：%4 人\n"
        "普通模式玩家：%5 人\n"
        "困难模式玩家：%6 人\n"
        "===============================\n"
        "🏆 最高分记录\n"
        "简单模式：%7 (%8 分)\n"
        "普通模式：%9 (%10 分)\n"
        "困难模式：%11 (%12 分)\n"
        "===============================\n"
        "📅 统计时间：%13"
    ).arg(totalUsers)
        .arg(activeUsers).arg(activePercent, 0, 'f', 1)
        .arg(easyPlayers).arg(normalPlayers).arg(hardPlayers)
        .arg(topEasyUser).arg(topEasyScore)
        .arg(topNormalUser).arg(topNormalScore)
        .arg(topHardUser).arg(topHardScore)
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    QMessageBox::information(this, "统计数据", statsText);
}