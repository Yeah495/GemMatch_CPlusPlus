#include "Email.h"
#include <QMessageBox>

Smtp::Smtp(const QString& user, const QString& pass, const QString& host, int port)
{
    this->socket = new QSslSocket(this);
    this->user = user;
    this->pass = pass;
    this->host = host;
    this->port = port;
    this->t = new QTextStream(this->socket);

    connect(socket, &QSslSocket::readyRead, this, &Smtp::readyRead);
    connect(socket, &QSslSocket::connected, this, &Smtp::connected);
    connect(socket, &QSslSocket::errorOccurred, this, &Smtp::errorReceived);
    connect(socket, &QSslSocket::disconnected, this, &Smtp::disconnected);
}

Smtp::~Smtp()
{
    delete t;
    delete socket;
}

void Smtp::sendMail(const QString& to, const QString& subject, const QString& body)
{
    this->rcpt = to;
    this->message = "To: " + to + "\r\n";
    this->message.append("From: " + user + "\r\n");
    this->message.append("Subject: " + subject + "\r\n");
    this->message.append("MIME-Version: 1.0\r\n");
    this->message.append("Content-Type: text/html; charset=UTF-8\r\n\r\n"); // 使用HTML格式，支持中文
    this->message.append(body);
    this->message.replace(QString::fromLatin1("\n"), QString::fromLatin1("\r\n"));
    this->message.replace(QString::fromLatin1("\r\n.\r\n"), QString::fromLatin1("\r\n..\r\n"));

    this->state = Init;
    socket->connectToHostEncrypted(host, port); // SSL连接
    if (!socket->waitForConnected(30000)) {
        qDebug() << "Connection timeout";
    }
}

void Smtp::stateChanged(QAbstractSocket::SocketState socketState)
{
    qDebug() << "State changed: " << socketState;
}

void Smtp::errorReceived(QAbstractSocket::SocketError socketError)
{
    qDebug() << "Socket error: " << socketError;
}

void Smtp::disconnected()
{
    qDebug() << "Disconnected";
    qDebug() << "Error: " << socket->errorString();
}

void Smtp::connected()
{
    qDebug() << "Connected";
}

void Smtp::readyRead()
{
    if (!socket || !socket->isOpen()) return;
    QTextStream ts(socket); // 局部流，避免悬指针
    QString responseLine;
    do {
        responseLine = socket->readLine();
        response += responseLine;
    } while (socket->canReadLine() && responseLine[3] != ' ');
    // 只保留前三位响应码
    responseLine.truncate(3);

    if (state == Init && responseLine == "220") {
        ts << "EHLO localhost" << "\r\n";
        ts.flush();
        state = HandShake;
    }
    else if (state == HandShake && responseLine == "250") {
        socket->startClientEncryption();
        if (!socket->waitForEncrypted(3000)) {
            qDebug() << socket->errorString();
            state = Tls;
        }
        *t << "AUTH LOGIN" << "\r\n";
        t->flush();
        state = Auth;
    }
    // 开始认证：AUTH LOGIN
    else if (state == Tls && responseLine == "250") {
        *t << "AUTH LOGIN" << "\r\n";
        t->flush();
        state = Auth;
    }
    else if (state == Auth && responseLine == "334") {
        *t << QByteArray().append(user.toUtf8()).toBase64() << "\r\n";
        t->flush();
        state = User;
    }
    // 服务器再次 334：需要密码（
    else if (state == User && responseLine == "334") {
        *t << QByteArray().append(pass.toUtf8()).toBase64() << "\r\n";
        t->flush();
        state = Pass;
    }

    // 认证成功：235
    else if (state == Pass && responseLine == "235") {
        *t << "MAIL FROM:<" << user << ">\r\n";
        t->flush();
        state = Mail;
    }
    // 服务器 250：MAIL FROM 成功
    else if (state == Mail && responseLine == "250") {
        *t << "RCPT TO:<" << rcpt << ">\r\n";
        t->flush();
        state = Rcpt;
    }
    // 服务器 250：RCPT TO 成功
    else if (state == Rcpt && responseLine == "250") {
        *t << "DATA\r\n";
        t->flush();
        state = Data;
    }
    // 服务器 354：允许发送邮件内容
    else if (state == Data && responseLine == "354") {
        *t << message << "\r\n.\r\n";
        t->flush();
        state = Body;
    }
    // 服务器 250：邮件内容接收成功
    else if (state == Body && responseLine == "250") {
        *t << "QUIT\r\n";
        t->flush();
        state = Quit;
    }
    // 服务器 221：会话结束
    else if (state == Quit && responseLine == "221") {
        // success
    }
}