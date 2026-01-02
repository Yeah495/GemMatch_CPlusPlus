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
    // host: SMTP·þÎñÆ÷, port: ¶Ë¿Ú, user: ÓÊÏäÕËºÅ, pass: ÊÚÈ¨Âë
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