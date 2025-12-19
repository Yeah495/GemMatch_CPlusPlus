#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QList>
#include <QMap>
#include <algorithm>
#include <QStandardPaths>
#include <QDir>

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
        if (m_users.contains(user)) return false; // 用户名已存在
        UserData newData = { user, pass, email, 0 };
        m_users[user] = newData;
        saveData();
        return true;
    }

    // 登录：验证密码
    bool login(const QString& user, const QString& pass) {
        if (!m_users.contains(user)) return false;
        if (m_users[user].password == pass) {
            m_currentUser = user;
            return true;
        }
        return false;
    }

    // 更新当前用户分数
    void updateScore(int score) {
        if (m_users.contains(m_currentUser)) {
            if (score > m_users[m_currentUser].highScore) {
                m_users[m_currentUser].highScore = score;
                saveData();
            }
        }
    }

    // 获取前10名数据
    QList<UserData> getTop10() {
        QList<UserData> list;
        for (auto it = m_users.begin(); it != m_users.end(); ++it) {
            list.append(it.value());
        }
        // 降序排序
        std::sort(list.begin(), list.end(), [](const UserData& a, const UserData& b) {
            return a.highScore > b.highScore;
            });
        if (list.size() > 10) list = list.mid(0, 10);
        return list;
    }

    QString getCurrentUser() const { return m_currentUser; }

private:
    UserManager() { loadData(); }

    QString m_currentUser;
    QMap<QString, UserData> m_users;

    QString getDataPath() {
        // 保存到当前程序运行目录，方便查找
        return "user_data.json";
    }

    void loadData() {
        QFile file(getDataPath());
        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            QJsonArray arr = doc.array();
            for (auto val : arr) {
                QJsonObject obj = val.toObject();
                UserData u;
                u.username = obj["u"].toString();
                u.password = obj["p"].toString();
                u.email = obj["e"].toString();
                u.highScore = obj["s"].toInt();
                m_users[u.username] = u;
            }
            file.close();
        }
    }

    void saveData() {
        QJsonArray arr;
        for (auto it = m_users.begin(); it != m_users.end(); ++it) {
            const UserData& u = it.value();
            QJsonObject obj;
            obj["u"] = u.username;
            obj["p"] = u.password;
            obj["e"] = u.email;
            obj["s"] = u.highScore;
            arr.append(obj);
        }
        QFile file(getDataPath());
        if (file.open(QIODevice::WriteOnly)) {
            file.write(QJsonDocument(arr).toJson());
            file.close();
        }
    }
};

#endif // USERMANAGER_H