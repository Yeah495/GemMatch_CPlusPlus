#pragma once
#ifndef GAMELOGO_H
#define GAMELOGO_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QMouseEvent>

class GameLogo : public QWidget {
    Q_OBJECT
        Q_PROPERTY(qreal yOffset READ yOffset WRITE setYOffset) // 用于掉落位移
        Q_PROPERTY(qreal scale READ scale WRITE setScale)       // 用于呼吸缩放

public:
    explicit GameLogo(const QString& pixmapPath, QWidget* parent = nullptr);

    // 启动入场动画（掉落 + 开始呼吸）
    void startEntrance();

    qreal yOffset() const { return m_yOffset; }
    void setYOffset(qreal y);

    qreal scale() const { return m_scale; }
    void setScale(qreal s);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QPixmap m_pixmap;
    qreal m_yOffset; // 垂直偏移量
    qreal m_scale;   // 缩放比例

    QSequentialAnimationGroup* m_groupAnim; // 串行动画组
};

#endif // GAMELOGO_H