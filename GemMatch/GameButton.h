#pragma once
#ifndef GAMEBUTTON_H
#define GAMEBUTTON_H

#include <QPushButton>
#include <QPropertyAnimation>
#include <QMouseEvent>

class GameButton : public QPushButton {
    Q_OBJECT
        // 注册 scale 属性用于动画
        Q_PROPERTY(qreal scale READ scale WRITE setScale)

public:
    explicit GameButton(const QString& pixmapPath, QWidget* parent = nullptr);

    qreal scale() const { return m_scale; }
    void setScale(qreal s);

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    qreal m_scale;           // 当前缩放比例
    QPixmap m_pixmap;        // 按钮图片
    QPropertyAnimation* m_anim; // 缩放动画对象

    void startAnim(qreal endValue);
};

#endif // GAMEBUTTON_H