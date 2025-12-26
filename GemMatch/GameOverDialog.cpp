#include "GameOverDialog.h"
#include <QGridLayout>
#include <QPainter>
#include <QDebug>
#include <QDateTime> 


// 引入QR Code生成库
#include "qrcodegen.hpp"

using namespace qrcodegen;

GameOverDialog::GameOverDialog(int score, QWidget* parent)
    : QDialog(parent), m_score(score)
{
    setWindowTitle("游戏结束");
    setFixedSize(400, 550);
    setModal(true);

    // 设置窗口样式
    setStyleSheet(
        "QDialog {"
        "   background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "       stop:0 #667eea, stop:1 #764ba2);"
        "   border-radius: 15px;"
        "}"
    );

    setupUI();
}

GameOverDialog::~GameOverDialog() {}

void GameOverDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(30, 30, 30, 30);

    // 标题
    m_titleLabel = new QLabel("🎉 游戏结束 🎉");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet(
        "font-size: 28px;"
        "font-weight: bold;"
        "color: white;"
        "background: transparent;"
    );
    mainLayout->addWidget(m_titleLabel);

    // 分数显示
    m_scoreLabel = new QLabel(QString("得分: %1").arg(m_score));
    m_scoreLabel->setAlignment(Qt::AlignCenter);
    m_scoreLabel->setStyleSheet(
        "font-size: 36px;"
        "font-weight: bold;"
        "color: #FFD700;"
        "background: rgba(255, 255, 255, 0.2);"
        "border-radius: 10px;"
        "padding: 15px;"
    );
    mainLayout->addWidget(m_scoreLabel);

    // 二维码容器
    QWidget* qrContainer = new QWidget();
    qrContainer->setStyleSheet(
        "background: white;"
        "border-radius: 10px;"
        "padding: 15px;"
    );
    QVBoxLayout* qrLayout = new QVBoxLayout(qrContainer);

    // 生成二维码
    QString shareUrl = generateShareUrl(m_score);
    QPixmap qrPixmap = generateQRCode(shareUrl, 150);

    m_qrLabel = new QLabel();
    m_qrLabel->setPixmap(qrPixmap);
    m_qrLabel->setAlignment(Qt::AlignCenter);
    qrLayout->addWidget(m_qrLabel);

    // 提示文字
    m_hintLabel = new QLabel("📱 扫码分享你的成绩");
    m_hintLabel->setAlignment(Qt::AlignCenter);
    m_hintLabel->setStyleSheet(
        "font-size: 14px;"
        "color: #666;"
        "background: transparent;"
    );
    qrLayout->addWidget(m_hintLabel);

    mainLayout->addWidget(qrContainer);

    // 按钮区域
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(15);


    m_menuBtn = new QPushButton("🏠 返回主菜单");
    m_menuBtn->setFixedHeight(50);
    m_menuBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(255, 255, 255, 0.3);"
        "   color: white;"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "   border: 2px solid white;"
        "   border-radius: 25px;"
        "   padding: 10px 20px;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(255, 255, 255, 0.4);"
        "}"
        "QPushButton:pressed {"
        "   background-color: rgba(255, 255, 255, 0.2);"
        "}"
    );


    btnLayout->addWidget(m_menuBtn);
    mainLayout->addLayout(btnLayout);

    // 连接信号

    connect(m_menuBtn, &QPushButton::clicked, this, &GameOverDialog::backToMenu);
}

QPixmap GameOverDialog::generateQRCode(const QString& text, int size) {
    try {
        // 使用 QR Code 库生成二维码
        QrCode qr = QrCode::encodeText(text.toUtf8().constData(), QrCode::Ecc::MEDIUM);

        int qrSize = qr.getSize();
        int scale = size / qrSize;
        int actualSize = qrSize * scale;

        QImage img(actualSize, actualSize, QImage::Format_RGB32);
        img.fill(Qt::white);

        QPainter painter(&img);
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::black);

        for (int y = 0; y < qrSize; y++) {
            for (int x = 0; x < qrSize; x++) {
                if (qr.getModule(x, y)) {
                    painter.drawRect(x * scale, y * scale, scale, scale);
                }
            }
        }

        return QPixmap::fromImage(img);
    }
    catch (const std::exception& e) {
        qDebug() << "QR Code generation failed:" << e.what();

        // 返回一个错误提示图片
        QPixmap errorPixmap(size, size);
        errorPixmap.fill(Qt::white);
        QPainter p(&errorPixmap);
        p.setPen(Qt::red);
        p.drawText(errorPixmap.rect(), Qt::AlignCenter, "生成失败");
        return errorPixmap;
    }
}

QString GameOverDialog::generateShareUrl(int score) {
    // 方案1：生成一个包含分数的分享链接
    // 这里可以是你的游戏网站地址 + 参数
    QString baseUrl = "https://github.com/Yeah495/GemMatch_CPlusPlus";
    //QString shareUrl = QString("%1?score=%2&time=%3")
    //    .arg(baseUrl)
    //    .arg(score)
    //    .arg(QDateTime::currentDateTime().toSecsSinceEpoch());
    return baseUrl;


    //// 按照要求格式化字符串
    //// %1 会被 score 替换
    //QString shareText = QString("我在《宝石迷阵》中获得了 %1 分！你也来试试吧！https://github.com/Yeah495/GemMatch_CPlusPlus")
    //    .arg(score);

    //return shareText;
}
