#pragma once
#ifndef GEMITEM_H
#define GEMITEM_H

#include <QGraphicsObject>
#include <QPainter>
#include "Config.h"
#include <qpropertyanimation.h>

//ui动画中的宝石类
class GemItem : public QGraphicsObject {
    Q_OBJECT
public:
    GemItem(int row, int col, GemType type, QGraphicsItem* parent = nullptr);

    //更新宝石类型
    void setType(GemType type);
    GemType getType() const { return m_type; }

    //获取在网格中的坐标
    int getRow() const { return m_row; }
    int getCol() const { return m_col; }
    void setGridPos(int row, int col) { m_row = row; m_col = col; }

    //选中状态控制
    void setSelected(bool selected);
signals:
    //当被点击时发送信号，参数是自己的逻辑坐标
    void clicked(int row, int col);
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
private:
    int m_row, m_col;
    GemType m_type;
    bool m_isSelected;  //是否选中
    const int GEM_SIZE = 60;

    //旋转对象
    QPropertyAnimation* m_animRotate; 
};

#endif 