#include "GameLogo.h"
#include <QPainter>

GameLogo::GameLogo(const QString& pixmapPath, QWidget* parent)
    : QWidget(parent), m_yOffset(-300), m_scale(1.0) // 初始位置在上方
{
    setAttribute(Qt::WA_TranslucentBackground);
    m_pixmap.load(pixmapPath);
    if (!m_pixmap.isNull()) {
        setFixedSize(m_pixmap.size());
    }

    // 1. 掉落动画 (OutBounce 产生弹性)
    QPropertyAnimation* dropAnim = new QPropertyAnimation(this, "yOffset");
    dropAnim->setStartValue(-300); // 从上方 300 像素处掉落
    dropAnim->setEndValue(0);      // 落到原位
    dropAnim->setDuration(1200);
    dropAnim->setEasingCurve(QEasingCurve::OutBounce); // 弹性曲线

    // 2. 呼吸放大动画
    QPropertyAnimation* breathUp = new QPropertyAnimation(this, "scale");
    breathUp->setStartValue(1.0);
    breathUp->setEndValue(1.05); // 放大一点点
    breathUp->setDuration(1500);
    breathUp->setEasingCurve(QEasingCurve::SineCurve);

    // 3. 呼吸缩小动画
    QPropertyAnimation* breathDown = new QPropertyAnimation(this, "scale");
    breathDown->setStartValue(1.05);
    breathDown->setEndValue(1.0);
    breathDown->setDuration(1500);
    breathDown->setEasingCurve(QEasingCurve::SineCurve);

    // 组合呼吸循环
    QSequentialAnimationGroup* loopGroup = new QSequentialAnimationGroup(this);
    loopGroup->addAnimation(breathUp);
    loopGroup->addAnimation(breathDown);
    loopGroup->setLoopCount(-1); // 无限循环

    // 组合整体流程：先掉落，再循环呼吸
    m_groupAnim = new QSequentialAnimationGroup(this);
    m_groupAnim->addAnimation(dropAnim);
    m_groupAnim->addAnimation(loopGroup);
}

void GameLogo::startEntrance() {
    m_groupAnim->stop();
    m_groupAnim->start();
}

void GameLogo::setYOffset(qreal y) {
    m_yOffset = y;
    update();
}

void GameLogo::setScale(qreal s) {
    m_scale = s;
    update();
}

void GameLogo::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 坐标系变换：先移动到绘制位置(含掉落偏移)，再中心缩放
    painter.translate(width() / 2, height() / 2 + m_yOffset);
    painter.scale(m_scale, m_scale);
    painter.translate(-width() / 2, -height() / 2);

    painter.drawPixmap(0, 0, width(), height(), m_pixmap);
}