#include "ScoreChartWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <algorithm>
#include <numeric>

ScoreChartWidget::ScoreChartWidget(QWidget* parent)
    : QWidget(parent),
    m_difficulty(3),
    m_xMax(10),
    m_yMax(1000),
    m_yMin(0) {

    // 设置默认颜色
    m_chartColor = QColor(255, 107, 53);

    // 设置最小大小
    setMinimumSize(400, 300);
}

void ScoreChartWidget::setScores(const QList<int>& scores, const QString& title) {
    m_scores = scores;
    m_title = title;

    if (!m_scores.isEmpty()) {
        // 计算Y轴范围
        int maxScore = *std::max_element(m_scores.begin(), m_scores.end());
        m_yMax = qMax(maxScore * 1.2, 100.0); 
        // 横轴最大值设置为数据点数量
        m_xMax = qMin(m_scores.size(), 10);
    }
    else {
        m_xMax = 1;  // 最少显示1个点
        m_yMax = 1000;
    }

    update(); // 重绘
}

void ScoreChartWidget::setDifficulty(int difficulty) {
    m_difficulty = difficulty;

    // 根据难度设置不同的线条颜色
    switch (difficulty) {
    case 3: 
        m_chartColor = QColor(76, 175, 80);
        break;
    case 5: 
        m_chartColor = QColor(33, 150, 243);
        break;
    case 7:
        m_chartColor = QColor(244, 67, 54);
        break;
    }

    update();
}

int ScoreChartWidget::getAverageScore() const {
    if (m_scores.isEmpty()) return 0;

    int sum = std::accumulate(m_scores.begin(), m_scores.end(), 0);
    return sum / m_scores.size();
}

void ScoreChartWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 计算布局
    calculateLayout();

    // 绘制背景
    painter.fillRect(rect(), QColor(255, 255, 255, 180));

    // 绘制网格
    painter.setPen(QPen(QColor(200, 200, 200, 100), 1, Qt::DotLine));
    int gridLines = 5;
    for (int i = 0; i <= gridLines; i++) {
        int y = m_chartRect.bottom() - (m_chartRect.height() * i / gridLines);
        painter.drawLine(m_chartRect.left(), y, m_chartRect.right(), y);
    }

    // 绘制坐标轴
    painter.setPen(QPen(QColor(64, 64, 64), 2));
    // X轴
    painter.drawLine(m_chartRect.left(), m_chartRect.bottom(),
        m_chartRect.right(), m_chartRect.bottom());
    // Y轴
    painter.drawLine(m_chartRect.left(), m_chartRect.top(),
        m_chartRect.left(), m_chartRect.bottom());

    // 绘制轴标签
    painter.setPen(QPen(QColor(64, 64, 64)));
    painter.setFont(QFont("Microsoft YaHei", 10));

    // X轴标签
    int dataCount = m_scores.size();
    int maxDisplayPoints = 10;  // 最多显示10个点

    // 确定实际显示的数据点数量
    int displayCount = qMin(dataCount, maxDisplayPoints);

    if (displayCount > 0) {
        for (int i = 0; i < displayCount; i++) {
            // 只显示每个数据点对应的标签
            int x = m_chartRect.left() + (m_chartRect.width() * i / qMax(displayCount - 1, 1));
            QString label = QString::number(i + 1);  
            QRect textRect(x - 20, m_chartRect.bottom() + 5, 40, 20);
            painter.drawText(textRect, Qt::AlignCenter, label);
        }
    }

    // Y轴标签
    for (int i = 0; i <= gridLines; i++) {
        int value = m_yMax * i / gridLines;
        int y = m_chartRect.bottom() - (m_chartRect.height() * i / gridLines);
        QString label = QString::number(value);
        QRect textRect(m_chartRect.left() - 50, y - 10, 45, 20);
        painter.drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, label);
    }

    // 绘制折线
    if (dataCount >= 2) {
        QPainterPath linePath;
        QPainterPath fillPath;

        for (int i = 0; i < displayCount; i++) {
            QPoint p = scoreToPoint(i, m_scores[i]);

            if (i == 0) {
                linePath.moveTo(p);
                fillPath.moveTo(p.x(), m_chartRect.bottom());
                fillPath.lineTo(p);
            }
            else {
                linePath.lineTo(p);
                fillPath.lineTo(p);
            }
        }

        // 闭合填充区域
        if (displayCount > 0) {
            QPoint lastPoint = scoreToPoint(displayCount - 1, m_scores[displayCount - 1]);
            fillPath.lineTo(lastPoint.x(), m_chartRect.bottom());
            fillPath.closeSubpath();
        }

        // 绘制填充区域
        QColor fillColor = m_chartColor;
        fillColor.setAlpha(50);
        painter.setBrush(QBrush(fillColor));
        painter.setPen(Qt::NoPen);
        painter.drawPath(fillPath);

        // 绘制折线
        painter.setPen(QPen(m_chartColor, 3));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(linePath);

        // 绘制数据点
        painter.setBrush(QBrush(Qt::white));
        painter.setPen(QPen(m_chartColor, 2));
        for (int i = 0; i < displayCount; i++) {
            QPoint p = scoreToPoint(i, m_scores[i]);
            painter.drawEllipse(p, 6, 6);
        }
    }
    else if (dataCount == 1) {
        // 只有一个数据点时显示点
        QPoint p = scoreToPoint(0, m_scores[0]);
        painter.setBrush(QBrush(Qt::white));
        painter.setPen(QPen(m_chartColor, 2));
        painter.drawEllipse(p, 6, 6);
    }

    // 绘制标题
    if (!m_title.isEmpty()) {
        painter.setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
        painter.setPen(QColor("#044BB7"));
        painter.drawText(rect().adjusted(0, 10, 0, 0), Qt::AlignTop | Qt::AlignHCenter, m_title);
    }
}

void ScoreChartWidget::resizeEvent(QResizeEvent* event) {
    Q_UNUSED(event);
    calculateLayout();
}

void ScoreChartWidget::calculateLayout() {
    // 计算图表区域，留出空间给坐标轴标签和标题
    int left = 60;
    int right = width() - 20;
    int top = 50;
    int bottom = height() - 40;

    m_chartRect = QRect(left, top, right - left, bottom - top);
}

QPoint ScoreChartWidget::scoreToPoint(int index, int score) const {
    int dataCount = qMin(m_scores.size(), 10);  // 最多显示10个点
    int x = m_chartRect.left() + (m_chartRect.width() * index / qMax(dataCount - 1, 1));
    int y = m_chartRect.bottom() - (m_chartRect.height() * score / m_yMax);

    return QPoint(x, y);
}