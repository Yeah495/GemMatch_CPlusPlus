#include "GameLogo.h"
#include <QPainter>

GameLogo::GameLogo(const QString& pixmapPath, QWidget* parent)
    : QWidget(parent), m_yOffset(-300), m_scale(1.0)
{
    setAttribute(Qt::WA_TranslucentBackground);
    m_pixmap.load(pixmapPath);
    if (!m_pixmap.isNull()) {
        setFixedSize(m_pixmap.size()*1.2);
    }

    // 掉落动画
    QPropertyAnimation* dropAnim = new QPropertyAnimation(this, "yOffset");
    dropAnim->setStartValue(-300);
    dropAnim->setEndValue(0);
    dropAnim->setDuration(1200);
    dropAnim->setEasingCurve(QEasingCurve::OutBounce);

    // 呼吸动画 
    QPropertyAnimation* breathAnim = new QPropertyAnimation(this, "scale");
    breathAnim->setDuration(3000); // 总周期 3秒 
    breathAnim->setLoopCount(-1);  // 无限循环

    // 关键帧定义整个周期的路径
    breathAnim->setStartValue(1.0);         //初始时间：原始大小
    breathAnim->setKeyValueAt(0.5, 1.10);   //中间时间：放大到 1.10
    breathAnim->setEndValue(1.0);           //结束时间：回到原始大小

    // InOutSine 正弦曲线
    breathAnim->setEasingCurve(QEasingCurve::InOutSine);

  
    //  breathAnim无限循环
    m_groupAnim = new QSequentialAnimationGroup(this);
    m_groupAnim->addAnimation(dropAnim);   // 先执行掉落
    m_groupAnim->addAnimation(breathAnim); // 掉落完后，执行呼吸


}

//开始播放动画
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

//调用绘制动画
void GameLogo::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);

    //开启抗锯齿
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    //将坐标原点移动到控件的几何中心
    //加上 m_yOffset 实现掉落效果
    qreal centerX = width() / 2.0;
    qreal centerY = height() / 2.0 + m_yOffset;

    painter.translate(centerX, centerY);

    //呼吸缩放
    painter.scale(m_scale, m_scale);

    // 绘制图片
    qreal pixW = m_pixmap.width();
    qreal pixH = m_pixmap.height();
    painter.drawPixmap(-pixW / 2.0, -pixH / 2.0, m_pixmap);
}