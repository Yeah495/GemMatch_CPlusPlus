#pragma once
/*这是在屏幕上移动的那个“宝石”。继承自 QGraphicsObject 以便支持 Qt 的属性动画 (QPropertyAnimation)。*/





#ifndef GEMITEM_H
#define GEMITEM_H

#include <QGraphicsObject>
#include <QPainter>
#include "Config.h"

// 继承 QGraphicsObject 从而支持信号槽和属性动画
class GemItem : public QGraphicsObject {
    Q_OBJECT
public:
    GemItem(int row, int col, GemType type, QGraphicsItem* parent = nullptr);

    // 更新宝石类型（用于复用图元）
    void setType(GemType type);
    GemType getType() const { return m_type; }

    // 获取在网格中的逻辑坐标
    int getRow() const { return m_row; }
    int getCol() const { return m_col; }
    void setGridPos(int row, int col) { m_row = row; m_col = col; }

    // 必须实现的纯虚函数
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    // 选中状态控制
    void setSelected(bool selected);

signals:
    // 当被点击时发送信号，参数是自己的逻辑坐标
    void clicked(int row, int col);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    int m_row, m_col;
    GemType m_type;
    bool m_isSelected;
    const int GEM_SIZE = 60; // 假设每个宝石 60x60 像素
};

#endif // GEMITEM_H