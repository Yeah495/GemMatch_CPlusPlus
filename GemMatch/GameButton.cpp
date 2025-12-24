#include "GameButton.h"
#include <QPainter>

#include <QPainterPath>

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
        setFixedSize(m_pixmap.width() * 1.25, m_pixmap.height() * 1.25);
    }

    // 初始化动画
    m_anim = new QPropertyAnimation(this, "scale", this);
    m_anim->setDuration(150); // 动画时长 150ms
    m_anim->setEasingCurve(QEasingCurve::OutQuad);
}

void GameButton::setSelected(bool selected) {
    if (m_isSelected == selected) return;
    m_isSelected = selected;
    update(); // 触发重绘
}

void GameButton::setScale(qreal s) {
    m_scale = s;
    update(); // 触发重绘
}

void GameButton::setPixmap(const QString& path) {
    // 1. 加载新图片
    m_pixmap.load(path);

    // 2. 触发重绘，这样 paintEvent 就会画新图了
    update();
}

void GameButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // --- 1. 处理坐标系变换 (保持你原有的缩放逻辑) ---
    painter.translate(width() / 2, height() / 2);
    painter.scale(m_scale, m_scale);

    // --- 2. 绘制图片背景 ---
    // 如果按钮被禁用 (setEnabled(false))，我们可以降低透明度，让它看起来变灰
    if (!isEnabled()) {
        painter.setOpacity(0.6);
    }

    painter.drawPixmap(-m_pixmap.width() / 2,
        -m_pixmap.height() / 2,
        m_pixmap);

    // --- 3. 绘制文字 (新增部分) ---
    if (!text().isEmpty()) {
        // A. 设置字体 (使用外部 setFont 设置的字体)
        painter.setFont(this->font());

        // 定义绘制区域（通常就是图片的大小）
        QRect rect(-m_pixmap.width() / 2, -m_pixmap.height() / 2,
            m_pixmap.width(), m_pixmap.height());

        // B. 简单的绘制方式 (直接画白色文字)
        /*
        painter.setPen(Qt::white);
        painter.drawText(rect, Qt::AlignCenter, text());
        */

        // C. 高级绘制方式：【带描边的文字】(强烈推荐，在游戏里看的最清楚)
        QPainterPath path;
        path.addText(rect.center() + QPointF(0, fontMetrics().descent()),
            this->font(), text());

        // 既然addText是基于基线的，我们需要重新居中一下
        // 为了简单，我们还是用 drawText 的方式，或者手动计算偏移
        // 这里提供一个最简单的描边模拟法：

        // C1. 先画黑色阴影/描边
        painter.setPen(QColor(0, 0, 0, 150)); // 半透明黑色
        // 向右下偏移一点点画一次，形成阴影
        QRect shadowRect = rect.translated(2, 2);
        painter.drawText(shadowRect, Qt::AlignCenter, text());

        // C2. 再画白色主体
        painter.setPen(Qt::yellow); // 或者 Qt::yellow, Qt::gold
        painter.drawText(rect, Qt::AlignCenter, text());
    }

    //检查是否需要画选中框
    if (m_isSelected) {
        QPen pen(QColor(255, 215, 0)); // 金色
        pen.setWidth(5);               // 边框宽度
        pen.setJoinStyle(Qt::RoundJoin); // 圆角连接

        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);

        // 这里的 rect 需要根据你的图片大小微调，通常比图片略大一点好看
        // 假设之前绘制区域是 m_pixmap 大小
        QRect borderRect = QRect(-m_pixmap.width() / 2, -m_pixmap.height() / 2,
            m_pixmap.width(), m_pixmap.height());

        painter.drawRect(borderRect);

        // 也画一个 √ 号表示选中
        painter.drawText(borderRect, Qt::AlignTop | Qt::AlignRight, "✔️");
    }
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