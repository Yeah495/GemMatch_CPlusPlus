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
#include <QJsonDocument>
#include <QJsonArray>

// 用户数据结构体 - 更新为多难度分数
struct UserData {
    QString username;
    QString password;
    QString email;

    // 各难度最高分
    int easyHighScore;
    int normalHighScore;
    int hardHighScore;

    // 各难度最近10场得分（用JSON字符串存储）
    QString easyRecentScores;  // JSON数组字符串
    QString normalRecentScores;
    QString hardRecentScores;
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

        // 插入新用户，初始化所有分数为0
        QSqlQuery insertQuery(m_db);
        insertQuery.prepare(
            "INSERT INTO users (username, password, email, "
            "easy_high_score, normal_high_score, hard_high_score, "
            "easy_recent_scores, normal_recent_scores, hard_recent_scores) "
            "VALUES (:username, :password, :email, "
            "0, 0, 0, "
            "'[]', '[]', '[]')"
        );
        insertQuery.bindValue(":username", user);
        insertQuery.bindValue(":password", pass);
        insertQuery.bindValue(":email", email);

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

    // 更新分数（根据难度）
    void updateScore(int score, int difficultyLevel) {
        if (m_currentUser.isEmpty()) return;

        QString scoreField, recentField;
        switch (difficultyLevel) {
        case 3: // 简单
            scoreField = "easy_high_score";
            recentField = "easy_recent_scores";
            break;
        case 5: // 普通
            scoreField = "normal_high_score";
            recentField = "normal_recent_scores";
            break;
        case 7: // 困难
            scoreField = "hard_high_score";
            recentField = "hard_recent_scores";
            break;
        default:
            return; // 无效难度
        }

        // 开启事务
        m_db.transaction();

        try {
            // 1. 获取当前最高分和最近得分列表
            QSqlQuery selectQuery(m_db);
            selectQuery.prepare(
                QString("SELECT %1, %2 FROM users WHERE username = :username")
                .arg(scoreField).arg(recentField)
            );
            selectQuery.bindValue(":username", m_currentUser);

            int currentHighScore = 0;
            QString recentScoresJson = "[]";

            if (selectQuery.exec() && selectQuery.next()) {
                currentHighScore = selectQuery.value(0).toInt();
                recentScoresJson = selectQuery.value(1).toString();
            }

            // 2. 更新最高分（如果新分数更高）
            bool updateHighScore = false;
            if (score > currentHighScore) {
                QSqlQuery updateHighQuery(m_db);
                updateHighQuery.prepare(
                    QString("UPDATE users SET %1 = :score WHERE username = :username")
                    .arg(scoreField)
                );
                updateHighQuery.bindValue(":score", score);
                updateHighQuery.bindValue(":username", m_currentUser);
                updateHighQuery.exec();
                updateHighScore = true;
            }

            // 3. 更新最近得分列表
            QJsonDocument doc = QJsonDocument::fromJson(recentScoresJson.toUtf8());
            QJsonArray scoresArray = doc.array();

            // 添加新得分到数组开头
            scoresArray.prepend(score);

            // 保持最多10个得分
            while (scoresArray.size() > 10) {
                scoresArray.removeLast();
            }

            // 转换为JSON字符串
            QJsonDocument newDoc(scoresArray);
            QString newScoresJson = newDoc.toJson(QJsonDocument::Compact);

            // 更新数据库
            QSqlQuery updateRecentQuery(m_db);
            updateRecentQuery.prepare(
                QString("UPDATE users SET %1 = :scores WHERE username = :username")
                .arg(recentField)
            );
            updateRecentQuery.bindValue(":scores", newScoresJson);
            updateRecentQuery.bindValue(":username", m_currentUser);
            updateRecentQuery.exec();

            // 提交事务
            m_db.commit();

            qDebug() << "分数更新成功！难度：" << difficultyLevel
                << "，本次得分：" << score
                << "，最高分更新：" << updateHighScore;

        }
        catch (...) {
            // 发生错误，回滚事务
            m_db.rollback();
            qDebug() << "更新分数时发生错误！";
        }
    }

    QList<UserData> getRanking(int difficultyLevel) {
        QList<UserData> list;

        QString scoreField;
        switch (difficultyLevel) {
        case 3: scoreField = "easy_high_score"; break;
        case 5: scoreField = "normal_high_score"; break;
        case 7: scoreField = "hard_high_score"; break;
        default: return list; // 无效难度
        }

        QSqlQuery query(m_db);
        QString sql = QString(
            "SELECT username, email, %1, "
            "easy_recent_scores, normal_recent_scores, hard_recent_scores "
            "FROM users "
            "WHERE %1 > 0 "  // 只显示有分数的用户
            "ORDER BY %1 DESC LIMIT 10"
        ).arg(scoreField);

        qDebug() << "执行SQL查询：" << sql;

        if (query.exec(sql)) {
            qDebug() << "SQL查询成功，开始解析结果...";
            while (query.next()) {
                UserData user;
                user.username = query.value(0).toString();
                user.email = query.value(1).toString();

                // 初始化所有分数
                user.easyHighScore = 0;
                user.normalHighScore = 0;
                user.hardHighScore = 0;

                // 根据难度设置对应分数
                int score = query.value(2).toInt();
                switch (difficultyLevel) {
                case 3: user.easyHighScore = score; break;
                case 5: user.normalHighScore = score; break;
                case 7: user.hardHighScore = score; break;
                }

                // 最近得分
                user.easyRecentScores = query.value(3).toString();
                user.normalRecentScores = query.value(4).toString();
                user.hardRecentScores = query.value(5).toString();

                qDebug() << "查询到用户：" << user.username << "，分数：" << score;
                list.append(user);
            }
            qDebug() << "共查询到" << list.size() << "条记录";
        }
        else {
            qDebug() << "SQL查询失败：" << query.lastError().text();
        }

        return list;
    }

    // 获取当前用户的指定难度最高分
    int getCurrentUserHighScore(int difficultyLevel) {
        if (m_currentUser.isEmpty()) return 0;

        QString scoreField;
        switch (difficultyLevel) {
        case 3: scoreField = "easy_high_score"; break;
        case 5: scoreField = "normal_high_score"; break;
        case 7: scoreField = "hard_high_score"; break;
        default: return 0;
        }

        QSqlQuery query(m_db);
        query.prepare(
            QString("SELECT %1 FROM users WHERE username = :username")
            .arg(scoreField)
        );
        query.bindValue(":username", m_currentUser);

        if (query.exec() && query.next()) {
            return query.value(0).toInt();
        }
        return 0;
    }

    // 获取当前用户的指定难度最近得分列表
    QList<int> getCurrentUserRecentScores(int difficultyLevel) {
        QList<int> scores;
        if (m_currentUser.isEmpty()) return scores;

        QString recentField;
        switch (difficultyLevel) {
        case 3: recentField = "easy_recent_scores"; break;
        case 5: recentField = "normal_recent_scores"; break;
        case 7: recentField = "hard_recent_scores"; break;
        default: return scores;
        }

        QSqlQuery query(m_db);
        query.prepare(
            QString("SELECT %1 FROM users WHERE username = :username")
            .arg(recentField)
        );
        query.bindValue(":username", m_currentUser);

        if (query.exec() && query.next()) {
            QString jsonStr = query.value(0).toString();
            QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
            QJsonArray array = doc.array();

            for (const auto& value : array) {
                scores.append(value.toInt());
            }
        }
        return scores;
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
        QString dbPath = QDir::currentPath() + "/user_data.db";
        m_db.setDatabaseName(dbPath);

        // 打开数据库
        if (!m_db.open()) {
            qDebug() << "Database connection failed:" << m_db.lastError().text();
            return;
        }

        // 创建用户表（如果不存在） - 更新表结构
        QSqlQuery query(m_db);
        QString createTable = R"(
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT UNIQUE NOT NULL,
                password TEXT NOT NULL,
                email TEXT,
                
                -- 各难度最高分
                easy_high_score INTEGER DEFAULT 0,
                normal_high_score INTEGER DEFAULT 0,
                hard_high_score INTEGER DEFAULT 0,
                
                -- 各难度最近10场得分（JSON格式存储）
                easy_recent_scores TEXT DEFAULT '[]',
                normal_recent_scores TEXT DEFAULT '[]',
                hard_recent_scores TEXT DEFAULT '[]',
                
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        )";

        if (!query.exec(createTable)) {
            qDebug() << "Create table failed:" << query.lastError().text();
        }

        // 创建索引以提高查询性能
        query.exec("CREATE INDEX IF NOT EXISTS idx_username ON users(username)");
        query.exec("CREATE INDEX IF NOT EXISTS idx_easy_score ON users(easy_high_score DESC)");
        query.exec("CREATE INDEX IF NOT EXISTS idx_normal_score ON users(normal_high_score DESC)");
        query.exec("CREATE INDEX IF NOT EXISTS idx_hard_score ON users(hard_high_score DESC)");
    }
};

#endif // USERMANAGER_H