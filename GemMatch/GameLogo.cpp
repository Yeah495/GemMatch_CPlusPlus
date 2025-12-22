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

    // 1. 掉落动画 (保持不变)
    QPropertyAnimation* dropAnim = new QPropertyAnimation(this, "yOffset");
    dropAnim->setStartValue(-300);
    dropAnim->setEndValue(0);
    dropAnim->setDuration(1200);
    dropAnim->setEasingCurve(QEasingCurve::OutBounce);

    // ================== 修改开始 ==================

    // 2. 呼吸动画 (合并为一个完整的周期动画)
    QPropertyAnimation* breathAnim = new QPropertyAnimation(this, "scale");
    breathAnim->setDuration(3000); // 总周期 3秒 (1.5s 上 + 1.5s 下)
    breathAnim->setLoopCount(-1);  // 无限循环

    // 关键帧设置：定义整个周期的路径
    breathAnim->setStartValue(1.0);         // 0% 时间点：原始大小
    breathAnim->setKeyValueAt(0.5, 1.10);   // 50% 时间点：放大到 1.10
    breathAnim->setEndValue(1.0);           // 100% 时间点：回到原始大小

    // 【核心关键】使用 InOutSine 曲线
    // InOut:意味着两头慢，中间快。
    // Sine: 正弦曲线，最符合自然界呼吸的规律。
    breathAnim->setEasingCurve(QEasingCurve::InOutSine);

    // 3. 组合整体流程
    // 注意：不再需要 loopGroup 了，因为 breathAnim 自己就是无限循环的
    m_groupAnim = new QSequentialAnimationGroup(this);
    m_groupAnim->addAnimation(dropAnim);   // 先执行掉落
    m_groupAnim->addAnimation(breathAnim); // 掉落完后，执行呼吸(无限卡在这里)

    // ================== 修改结束 ==================
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

    // 开启抗锯齿，这对于呼吸动画非常重要，否则边缘会有锯齿抖动
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 1. 将坐标原点移动到控件的几何中心
    // 加上 m_yOffset 实现掉落效果
    qreal centerX = width() / 2.0;
    qreal centerY = height() / 2.0 + m_yOffset;

    painter.translate(centerX, centerY);

    // 2. 执行呼吸缩放
    painter.scale(m_scale, m_scale);

    // 3. 绘制图片
    // 关键点：因为原点已经在中心了，我们需要向左上角偏移图片宽/高的一半
    // 这样图片的中心才会和坐标原点对齐
    qreal pixW = m_pixmap.width();
    qreal pixH = m_pixmap.height();

    // 这里的坐标是 (-w/2, -h/2)，确保绘制的是原图大小，不是拉伸后的 widget 大小
    painter.drawPixmap(-pixW / 2.0, -pixH / 2.0, m_pixmap);
}