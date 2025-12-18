#pragma once
/*这是在屏幕上移动的那个“宝石”。继承自 QGraphicsObject 以便支持 Qt 的属性动画 (QPropertyAnimation)。*/


#include "GemItem.h"
#include "ResourceLoader.h"
#include <QCursor>

GemItem::GemItem(int row, int col, GemType type, QGraphicsItem* parent)
    : QGraphicsObject(parent), m_row(row), m_col(col), m_type(type), m_isSelected(false)
{
    setAcceptHoverEvents(true); // 允许悬停事件（可选，用于改变鼠标样式）
}

void GemItem::setType(GemType type) {
    m_type = type;
    update(); // 触发重绘
}

QRectF GemItem::boundingRect() const {
    return QRectF(0, 0, GEM_SIZE, GEM_SIZE);
}

void GemItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (m_type == GemType::Empty) return;

    // 1. 尝试获取图片
    QPixmap pixmap = ResourceLoader::instance().getGemPixmap(m_type);

    // 2. 【核心修改】检测图片是否为空
    if (pixmap.isNull()) {
        // 如果图片没找到，画一个纯色方块代替！
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

        // 可选：画个文字显示坐标，方便调试
        painter->setPen(Qt::black);
        painter->drawText(boundingRect(), Qt::AlignCenter, QString("%1,%2").arg(m_row).arg(m_col));
    }
    else {
        // 图片正常才画图片
        painter->drawPixmap(0, 0, GEM_SIZE, GEM_SIZE, pixmap);
    }

    // 绘制选中框（保持原样）
    if (m_isSelected) {
        painter->setPen(QPen(Qt::white, 3));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(0, 0, GEM_SIZE, GEM_SIZE);
    }
}

void GemItem::setSelected(bool selected) {
    m_isSelected = selected;
    update();
}

void GemItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    emit clicked(m_row, m_col);
    QGraphicsObject::mousePressEvent(event);
}