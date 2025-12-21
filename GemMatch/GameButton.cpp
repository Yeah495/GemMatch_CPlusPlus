#include "GameButton.h"
#include <QPainter>

GameButton::GameButton(const QString& pixmapPath, QWidget* parent)
    : QPushButton(parent), m_scale(1.0)
{
    // 加载图片
    m_pixmap.load(pixmapPath);

    // 去掉默认边框和背景，因为我们要自己画图
    setFlat(true);
    setStyleSheet("border: none; background: transparent;");

    // 设置按钮固定大小为图片大小
    if (!m_pixmap.isNull()) {
        setFixedSize(m_pixmap.size());
    }

    // 初始化动画
    m_anim = new QPropertyAnimation(this, "scale", this);
    m_anim->setDuration(150); // 动画时长 150ms
    m_anim->setEasingCurve(QEasingCurve::OutQuad);
}

void GameButton::setScale(qreal s) {
    m_scale = s;
    update(); // 触发重绘
}

void GameButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 变换坐标系到中心，实现中心缩放
    painter.translate(width() / 2, height() / 2);
    painter.scale(m_scale, m_scale);
    painter.translate(-width() / 2, -height() / 2);

    // 绘制图片
    painter.drawPixmap(0, 0, width(), height(), m_pixmap);
}

void GameButton::startAnim(qreal endValue) {
    m_anim->stop();
    m_anim->setEndValue(endValue);
    m_anim->start();
}

// 悬停：放大 1.1 倍
void GameButton::enterEvent(QEnterEvent* event) {
    QPushButton::enterEvent(event);
    startAnim(1.1);
}

// 离开：恢复 1.0 倍
void GameButton::leaveEvent(QEvent* event) {
    QPushButton::leaveEvent(event);
    startAnim(1.0);
}

// 按下：凹陷 0.9 倍
void GameButton::mousePressEvent(QMouseEvent* event) {
    QPushButton::mousePressEvent(event);
    startAnim(0.9);
}

// 松开：如果还在按钮上，恢复悬停状态(1.1)，否则恢复原始(1.0)
void GameButton::mouseReleaseEvent(QMouseEvent* event) {
    QPushButton::mouseReleaseEvent(event);
    if (rect().contains(event->pos())) {
        startAnim(1.1);
    }
    else {
        startAnim(1.0);
    }
}