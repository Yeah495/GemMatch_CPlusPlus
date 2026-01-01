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
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>

// 用户数据结构体
struct UserData {
    QString username;
    QString password;
    QString email;

    // 各难度最高分
    int easyHighScore;
    int normalHighScore;
    int hardHighScore;

    // 各难度最近10场得分
    QString easyRecentScores;
    QString normalRecentScores;
    QString hardRecentScores;

    // 各难度总游戏场次
    int easyTotalGames;
    int normalTotalGames;
    int hardTotalGames;
};

class UserManager {
public:
    static UserManager& instance() {
        static UserManager i;
        return i;
    }

    enum RegisterStatus {
        REGISTER_SUCCESS,
        USERNAME_EXISTS,
        EMAIL_EXISTS,
        DATABASE_ERROR
    };

    // 注册用户
    RegisterStatus registerUser(const QString& user, const QString& pass, const QString& email) {

        QSqlQuery insertQuery(m_db);
        insertQuery.prepare(
            "INSERT INTO users (username, password, email, "
            "easy_high_score, normal_high_score, hard_high_score, "
            "easy_recent_scores, normal_recent_scores, hard_recent_scores, "
            "easy_total_games, normal_total_games, hard_total_games) " 
            "VALUES (:username, :password, :email, "
            "0, 0, 0, "
            "'[]', '[]', '[]', "
            "0, 0, 0)"  
        );
        insertQuery.bindValue(":username", user);
        insertQuery.bindValue(":password", pass);
        insertQuery.bindValue(":email", email);

        if (insertQuery.exec()) {
            return REGISTER_SUCCESS;
        }
        else {
            return DATABASE_ERROR;
        }
    }

    // 用户登录
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

	// 检查用户名是否存在
    bool checkUsernameExists(const QString& username) {
        QSqlQuery query(m_db);
        query.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
        query.addBindValue(username);
        if (query.exec() && query.next()) {
            return query.value(0).toInt() > 0;
        }
        return false;
    }

	// 检查邮箱是否存在
    bool checkEmailExists(const QString& email) {
        QSqlQuery query(m_db);
        query.prepare("SELECT COUNT(*) FROM users WHERE email = ?");
        query.addBindValue(email);
        if (query.exec() && query.next()) {
            return query.value(0).toInt() > 0;
        }
        return false;
    }

    // 更新分数
    int updateScore(int score, int difficultyLevel) {
        if (m_currentUser.isEmpty()) return 0;

        QString scoreField, recentField, totalGamesField;
        switch (difficultyLevel) {
        case 3:
            scoreField = "easy_high_score";
            recentField = "easy_recent_scores";
            totalGamesField = "easy_total_games";
            break;
        case 5:
            scoreField = "normal_high_score";
            recentField = "normal_recent_scores";
            totalGamesField = "normal_total_games";
            break;
        case 7:
            scoreField = "hard_high_score";
            recentField = "hard_recent_scores";
            totalGamesField = "hard_total_games";
            break;
        default: return 0;
        }

        int recordStatus = 0; 

        m_db.transaction();
        try {
            QSqlQuery query(m_db);
            //打破记录
            query.exec(QString("SELECT MAX(%1) FROM users").arg(scoreField));
            int globalMax = 0;
            if (query.next()) globalMax = query.value(0).toInt();

            query.prepare(QString("SELECT %1, %2 FROM users WHERE username = :u").arg(scoreField).arg(recentField));
            query.bindValue(":u", m_currentUser);

            int currentPersonalMax = 0;
            QString recentJson = "[]";
            if (query.exec() && query.next()) {
                currentPersonalMax = query.value(0).toInt();
                recentJson = query.value(1).toString();
            }

            if (score > globalMax) {
                recordStatus = 2; 
            }
            else if (score > currentPersonalMax) {
                recordStatus = 1; 
            }

           
            // 更新最高分
            if (score > currentPersonalMax) {
                QSqlQuery updateHigh(m_db);
                updateHigh.prepare(QString("UPDATE users SET %1 = :s WHERE username = :u").arg(scoreField));
                updateHigh.bindValue(":s", score);
                updateHigh.bindValue(":u", m_currentUser);
                updateHigh.exec();
            }

            //更新最近得分数组
            QJsonDocument doc = QJsonDocument::fromJson(recentJson.toUtf8());
            QJsonArray arr = doc.array();
            arr.prepend(score);
            while (arr.size() > 10) arr.removeLast();
            QJsonDocument newDoc(arr);

            QSqlQuery updateRecent(m_db);
            updateRecent.prepare(QString("UPDATE users SET %1 = :j WHERE username = :u").arg(recentField));
            updateRecent.bindValue(":j", newDoc.toJson(QJsonDocument::Compact));
            updateRecent.bindValue(":u", m_currentUser);
            updateRecent.exec();

            // 更新总游戏场次
            QSqlQuery updateTotalGames(m_db);
            updateTotalGames.prepare(
                QString("UPDATE users SET %1 = %1 + 1 WHERE username = :u").arg(totalGamesField)
            );
            updateTotalGames.bindValue(":u", m_currentUser);
            updateTotalGames.exec();

            m_db.commit();
        }
        catch (...) {
            m_db.rollback();
            return 0;
        }

        return recordStatus;
    }

    //获取指定难度级别的排行榜
    QList<UserData> getRanking(int difficultyLevel) {
        QList<UserData> list;

        QString scoreField;
        switch (difficultyLevel) {
        case 3: scoreField = "easy_high_score"; break;
        case 5: scoreField = "normal_high_score"; break;
        case 7: scoreField = "hard_high_score"; break;
        default: return list;
        }

        QSqlQuery query(m_db);
        QString sql = QString(
            "SELECT username, email, %1, "
            "easy_recent_scores, normal_recent_scores, hard_recent_scores, "
            "easy_total_games, normal_total_games, hard_total_games " 
            "FROM users "
            "WHERE %1 > 0 "
            "ORDER BY %1 DESC LIMIT 10"
        ).arg(scoreField);

        if (query.exec(sql)) {
            while (query.next()) {
                UserData user;
                user.username = query.value(0).toString();
                user.email = query.value(1).toString();

                user.easyHighScore = 0;
                user.normalHighScore = 0;
                user.hardHighScore = 0;

                int score = query.value(2).toInt();
                switch (difficultyLevel) {
                case 3: user.easyHighScore = score; break;
                case 5: user.normalHighScore = score; break;
                case 7: user.hardHighScore = score; break;
                }

                user.easyRecentScores = query.value(3).toString();
                user.normalRecentScores = query.value(4).toString();
                user.hardRecentScores = query.value(5).toString();

                user.easyTotalGames = query.value(6).toInt();
                user.normalTotalGames = query.value(7).toInt();
                user.hardTotalGames = query.value(8).toInt();

                list.append(user);
            }
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
            QString("SELECT %1 FROM users WHERE username = :username").arg(scoreField)
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
            QString("SELECT %1 FROM users WHERE username = :username").arg(recentField)
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

    // 获取当前用户指定难度的总游戏场次
    int getCurrentUserTotalGames(int difficultyLevel) {
        if (m_currentUser.isEmpty()) return 0;

        QString totalGamesField;
        switch (difficultyLevel) {
        case 3: totalGamesField = "easy_total_games"; break;
        case 5: totalGamesField = "normal_total_games"; break;
        case 7: totalGamesField = "hard_total_games"; break;
        default: return 0;
        }

        QSqlQuery query(m_db);
        query.prepare(
            QString("SELECT %1 FROM users WHERE username = :username").arg(totalGamesField)
        );
        query.bindValue(":username", m_currentUser);

        if (query.exec() && query.next()) {
            return query.value(0).toInt();
        }
        return 0;
    }

    // 获取当前用户所有难度的总游戏场次
    int getCurrentUserAllTotalGames() {
        if (m_currentUser.isEmpty()) return 0;

        QSqlQuery query(m_db);
        query.prepare(
            "SELECT easy_total_games, normal_total_games, hard_total_games "
            "FROM users WHERE username = :username"
        );
        query.bindValue(":username", m_currentUser);

        if (query.exec() && query.next()) {
            int easy = query.value(0).toInt();
            int normal = query.value(1).toInt();
            int hard = query.value(2).toInt();
            return easy + normal + hard;
        }
        return 0;
    }

    QString getCurrentUser() const { return m_currentUser; }

	//检查数据库连接状态
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
        m_db = QSqlDatabase::addDatabase("QSQLITE", "user_connection");
        QString dbPath = QDir::currentPath() + "/user_data.db";
        m_db.setDatabaseName(dbPath);

        if (!m_db.open()) {
            qDebug() << "Database connection failed:" << m_db.lastError().text();
            return;
        }

        // 创建用户表
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
                
                -- 各难度最近10场得分
                easy_recent_scores TEXT DEFAULT '[]',
                normal_recent_scores TEXT DEFAULT '[]',
                hard_recent_scores TEXT DEFAULT '[]',
                
                -- 各难度总游戏场次
                easy_total_games INTEGER DEFAULT 0,
                normal_total_games INTEGER DEFAULT 0,
                hard_total_games INTEGER DEFAULT 0,
                
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        )";

        if (!query.exec(createTable)) {
            qDebug() << "Create table failed:" << query.lastError().text();
        }

        query.exec("CREATE INDEX IF NOT EXISTS idx_username ON users(username)");
        query.exec("CREATE INDEX IF NOT EXISTS idx_easy_score ON users(easy_high_score DESC)");
        query.exec("CREATE INDEX IF NOT EXISTS idx_normal_score ON users(normal_high_score DESC)");
        query.exec("CREATE INDEX IF NOT EXISTS idx_hard_score ON users(hard_high_score DESC)");

        
        upgradeDatabase();
    }

    // 升级数据库
    void upgradeDatabase() {
        QSqlQuery query(m_db);

        bool hasTotalGamesFields = false;
        query.exec("PRAGMA table_info(users)");
        while (query.next()) {
            QString columnName = query.value(1).toString();
            if (columnName == "easy_total_games") {
                hasTotalGamesFields = true;
                break;
            }
        }

        if (!hasTotalGamesFields) {
            qDebug() << "升级数据库：添加总场次字段...";
            query.exec("ALTER TABLE users ADD COLUMN easy_total_games INTEGER DEFAULT 0");
            query.exec("ALTER TABLE users ADD COLUMN normal_total_games INTEGER DEFAULT 0");
            query.exec("ALTER TABLE users ADD COLUMN hard_total_games INTEGER DEFAULT 0");


            initTotalGamesForExistingUsers();
        }
    }

    // 为现有用户初始化总场次
    void initTotalGamesForExistingUsers() {
        qDebug() << "初始化现有用户的总场次...";

        QSqlQuery selectUsers(m_db);
        selectUsers.exec("SELECT username, easy_recent_scores, normal_recent_scores, hard_recent_scores FROM users");

        while (selectUsers.next()) {
            QString username = selectUsers.value(0).toString();
            QString easyScoresJson = selectUsers.value(1).toString();
            QString normalScoresJson = selectUsers.value(2).toString();
            QString hardScoresJson = selectUsers.value(3).toString();

            int easyGames = 0;
            int normalGames = 0;
            int hardGames = 0;

            QJsonDocument easyDoc = QJsonDocument::fromJson(easyScoresJson.toUtf8());
            if (!easyDoc.isNull() && easyDoc.isArray()) {
                easyGames = easyDoc.array().size();
            }

            QJsonDocument normalDoc = QJsonDocument::fromJson(normalScoresJson.toUtf8());
            if (!normalDoc.isNull() && normalDoc.isArray()) {
                normalGames = normalDoc.array().size();
            }

            QJsonDocument hardDoc = QJsonDocument::fromJson(hardScoresJson.toUtf8());
            if (!hardDoc.isNull() && hardDoc.isArray()) {
                hardGames = hardDoc.array().size();
            }

            QSqlQuery updateQuery(m_db);
            updateQuery.prepare(
                "UPDATE users SET easy_total_games = :easy, "
                "normal_total_games = :normal, hard_total_games = :hard "
                "WHERE username = :username"
            );
            updateQuery.bindValue(":easy", easyGames);
            updateQuery.bindValue(":normal", normalGames);
            updateQuery.bindValue(":hard", hardGames);
            updateQuery.bindValue(":username", username);
            updateQuery.exec();
        }

        qDebug() << "总场次初始化完成";
    }
};

#endif 