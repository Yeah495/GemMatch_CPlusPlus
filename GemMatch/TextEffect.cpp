#include "TextEffect.h"
#include <QParallelAnimationGroup>
#include <QGraphicsScene>

TextEffect::TextEffect(const QString& text, const QColor& color, int fontSize, QGraphicsItem* parent)
    : QGraphicsTextItem(text, parent), m_color(color)
{
    QFont font("Arial", fontSize, QFont::Bold);
    setFont(font);
    setDefaultTextColor(color);
    
    // Center the origin for scaling
    setTransformOriginPoint(boundingRect().center());
    
    // Initial state
    setOpacity(0.0);
    setScale(0.5);
    setZValue(100); // Ensure it's on top
}

void TextEffect::startAnimation() {
    QParallelAnimationGroup* group = new QParallelAnimationGroup(this);

    // 1. Scale Animation (Pop up)
    QPropertyAnimation* scaleAnim = new QPropertyAnimation(this, "scale");
    scaleAnim->setDuration(500);
    scaleAnim->setStartValue(0.5);
    scaleAnim->setEndValue(1.5);
    scaleAnim->setEasingCurve(QEasingCurve::OutBack);

    // 2. Opacity Animation (Fade in then out)
    QPropertyAnimation* opacityAnim = new QPropertyAnimation(this, "opacity");
    opacityAnim->setDuration(1000);
    opacityAnim->setKeyValueAt(0.0, 0.0);
    opacityAnim->setKeyValueAt(0.2, 1.0);
    opacityAnim->setKeyValueAt(0.7, 1.0);
    opacityAnim->setKeyValueAt(1.0, 0.0);

    // 3. Position Animation (Float up)
    QPropertyAnimation* posAnim = new QPropertyAnimation(this, "pos");
    posAnim->setDuration(1000);
    posAnim->setStartValue(pos());
    posAnim->setEndValue(pos() - QPointF(0, 50)); // Move up 50 pixels

    group->addAnimation(scaleAnim);
    group->addAnimation(opacityAnim);
    group->addAnimation(posAnim);

    connect(group, &QAbstractAnimation::finished, this, [this]() {
        if (scene()) {
            scene()->removeItem(this);
        }
        deleteLater();
    });

    group->start(QAbstractAnimation::DeleteWhenStopped);
}
