#include "GemItem.h"
#include "ResourceLoader.h"
#include <QCursor>

GemItem::GemItem(int row, int col, GemType type, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_row(row), m_col(col), m_type(type), m_isSelected(false)
{
    setAcceptHoverEvents(true);

    setTransformOriginPoint(GEM_SIZE / 2, GEM_SIZE / 2); //设置变换中心点为宝石的中心

    //初始化旋转动画
    m_animRotate = new QPropertyAnimation(this, "rotation", this);
    m_animRotate->setDuration(1000);
    m_animRotate->setStartValue(0);
    m_animRotate->setEndValue(360);
    m_animRotate->setLoopCount(-1); 
    m_animRotate->setEasingCurve(QEasingCurve::Linear);
}

void GemItem::setType(GemType type) {
    m_type = type;
    update();
}

QRectF GemItem::boundingRect() const {
    return QRectF(0, 0, GEM_SIZE, GEM_SIZE);
}

void GemItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (m_type == GemType::Empty) return;

    //获取图片
    QPixmap pixmap = ResourceLoader::instance().getGemPixmap(m_type);

    if (pixmap.isNull()) {
        //获取图片失败
        QColor debugColor;
        switch (m_type) {
        case GemType::Red: debugColor = Qt::red; break;
        case GemType::Blue: debugColor = Qt::blue; break;
        case GemType::Green: debugColor = Qt::green; break;
        case GemType::Yellow: debugColor = Qt::yellow; break;
        case GemType::Purple: debugColor = Qt::magenta; break;
        case GemType::Orange: debugColor = QColor(255, 165, 0); break;
        case GemType::White: debugColor = Qt::white; break;
        default: debugColor = Qt::gray; break;
        }
        painter->setBrush(debugColor);
        painter->setPen(Qt::white);
        painter->drawRect(0, 0, GEM_SIZE, GEM_SIZE);

        painter->setPen(Qt::black);
        painter->drawText(boundingRect(), Qt::AlignCenter, QString("%1,%2").arg(m_row).arg(m_col));
    }
    else {
        //图片正常
        painter->drawPixmap(0, 0, GEM_SIZE, GEM_SIZE, pixmap);
    }
}

void GemItem::setSelected(bool selected) {
    m_isSelected = selected;

    if (m_isSelected) {
        //选中时，开始旋转
        m_animRotate->start();
    }
    else {
        //取消选中时，停止动画并复位
        m_animRotate->stop();
        setRotation(0);
    }

    update();
}

//点击事件处理
void GemItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    emit clicked(m_row, m_col);
    QGraphicsObject::mousePressEvent(event);
}