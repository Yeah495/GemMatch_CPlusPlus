#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QString>
#include <QList>
#include <QMap>
#include <QStandardPaths>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <algorithm>

// 用户数据结构体
struct UserData {
    QString username;
    QString password;
    QString email;
    int highScore;
};

class UserManager {
public:
    static UserManager& instance() {
        static UserManager i;
        return i;
    }

    // 注册：返回是否成功
    bool registerUser(const QString& user, const QString& pass, const QString& email) {
        // 检查用户名是否已存在
        QSqlQuery checkQuery(m_db);
        checkQuery.prepare("SELECT COUNT(*) FROM users WHERE username = :username");
        checkQuery.bindValue(":username", user);

        if (checkQuery.exec() && checkQuery.next()) {
            if (checkQuery.value(0).toInt() > 0) {
                return false; // 用户名已存在
            }
        }

        // 插入新用户
        QSqlQuery insertQuery(m_db);
        insertQuery.prepare("INSERT INTO users (username, password, email, high_score) "
            "VALUES (:username, :password, :email, :score)");
        insertQuery.bindValue(":username", user);
        insertQuery.bindValue(":password", pass);
        insertQuery.bindValue(":email", email);
        insertQuery.bindValue(":score", 0);

        return insertQuery.exec();
    }

    // 登录：验证密码
    bool login(const QString& user, const QString& pass) {
        QSqlQuery query(m_db);
        query.prepare("SELECT password FROM users WHERE username = :username");
        query.bindValue(":username", user);

        if (query.exec() && query.next()) {
            QString storedPassword = query.value(0).toString();
            if (storedPassword == pass) {
                m_currentUser = user;
                return true;
            }
        }
        return false;
    }

    // 更新当前用户分数
    void updateScore(int score) {
        if (m_currentUser.isEmpty()) return;

        // 先获取当前最高分
        QSqlQuery selectQuery(m_db);
        selectQuery.prepare("SELECT high_score FROM users WHERE username = :username");
        selectQuery.bindValue(":username", m_currentUser);

        int currentHighScore = 0;
        if (selectQuery.exec() && selectQuery.next()) {
            currentHighScore = selectQuery.value(0).toInt();
        }

        // 如果新分数更高，更新数据库
        if (score > currentHighScore) {
            QSqlQuery updateQuery(m_db);
            updateQuery.prepare("UPDATE users SET high_score = :score WHERE username = :username");
            updateQuery.bindValue(":score", score);
            updateQuery.bindValue(":username", m_currentUser);
            updateQuery.exec();
        }
    }

    // 获取前10名数据
    QList<UserData> getTop10() {
        QList<UserData> list;
        QSqlQuery query(m_db);
        query.prepare("SELECT username, email, high_score FROM users "
            "ORDER BY high_score DESC LIMIT 10");

        if (query.exec()) {
            while (query.next()) {
                UserData user;
                user.username = query.value(0).toString();
                user.email = query.value(1).toString();
                user.highScore = query.value(2).toInt();
                list.append(user);
            }
        }
        return list;
    }

    // 获取用户当前最高分
    int getCurrentUserHighScore() {
        if (m_currentUser.isEmpty()) return 0;

        QSqlQuery query(m_db);
        query.prepare("SELECT high_score FROM users WHERE username = :username");
        query.bindValue(":username", m_currentUser);

        if (query.exec() && query.next()) {
            return query.value(0).toInt();
        }
        return 0;
    }

    QString getCurrentUser() const { return m_currentUser; }

    // 数据库连接测试
    bool isDatabaseConnected() const {
        return m_db.isOpen();
    }

private:
    UserManager() {
        initDatabase();
    }

    ~UserManager() {
        if (m_db.isOpen()) {
            m_db.close();
        }
    }

    QString m_currentUser;
    QSqlDatabase m_db;

    void initDatabase() {
        // 设置数据库连接
        m_db = QSqlDatabase::addDatabase("QSQLITE", "user_connection");

        // 数据库文件路径（程序运行目录）
        QString dbPath = QDir::currentPath() + "/user_data.db";
        m_db.setDatabaseName(dbPath);

        // 打开数据库
        if (!m_db.open()) {
            qDebug() << "Database connection failed:" << m_db.lastError().text();
            return;
        }

        // 创建用户表（如果不存在）
        QSqlQuery query(m_db);
        QString createTable = R"(
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT UNIQUE NOT NULL,
                password TEXT NOT NULL,
                email TEXT,
                high_score INTEGER DEFAULT 0,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        )";

        if (!query.exec(createTable)) {
            qDebug() << "Create table failed:" << query.lastError().text();
        }

        // 创建索引以提高查询性能
        query.exec("CREATE INDEX IF NOT EXISTS idx_username ON users(username)");
        query.exec("CREATE INDEX IF NOT EXISTS idx_high_score ON users(high_score DESC)");
    }
};

#endif // USERMANAGER_H