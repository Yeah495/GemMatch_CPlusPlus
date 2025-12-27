#ifndef TEXTEFFECT_H
#define TEXTEFFECT_H

#include <QGraphicsTextItem>
#include <QPropertyAnimation>
#include <QFont>
#include <QPen>
#include <QBrush>

class TextEffect : public QGraphicsTextItem {
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(qreal scale READ scale WRITE setScale)
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)

public:
    TextEffect(const QString& text, const QColor& color, int fontSize = 40, QGraphicsItem* parent = nullptr);
    void startAnimation();

signals:
    void animationFinished();

private:
    QColor m_color;
};

#endif // TEXTEFFECT_H
