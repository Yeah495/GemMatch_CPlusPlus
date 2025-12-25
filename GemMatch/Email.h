#ifndef SMTP_H
#define SMTP_H

#include <QObject>
#include <QSslSocket>
#include <QString>
#include <QTextStream>
#include <QAbstractSocket>

class Smtp : public QObject
{
    Q_OBJECT
public:
    // host: SMTP服务器(如 smtp.qq.com), port: 端口(如 465 SSL), user: 邮箱账号, pass: 授权码(不是登录密码)
    Smtp(const QString& user, const QString& pass, const QString& host, int port = 465);
    ~Smtp();

    void sendMail(const QString& to, const QString& subject, const QString& body);

signals:
    void status(const QString&);

private slots:
    void stateChanged(QAbstractSocket::SocketState socketState);
    void errorReceived(QAbstractSocket::SocketError socketError);
    void disconnected();
    void connected();
    void readyRead();

private:
    QString message;
    QTextStream* t;
    QSslSocket* socket;
    QString from;
    QString rcpt;
    QString response;
    QString user;
    QString pass;
    QString host;
    int port;
    enum states { Tls, HandShake, Auth, User, Pass, Rcpt, Mail, Data, Init, Body, Quit, Close };
    int state;
};

#endif // SMTP_H