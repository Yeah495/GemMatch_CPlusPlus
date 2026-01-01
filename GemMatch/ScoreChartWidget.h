#pragma once
#ifndef SCORECHARTWIDGET_H
#define SCORECHARTWIDGET_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QColor>

class ScoreChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit ScoreChartWidget(QWidget* parent = nullptr);

    // 设置数据
    void setScores(const QList<int>& scores, const QString& title = "");
    void setDifficulty(int difficulty); 

    // 获取统计数据
    int getMaxScore() const { return m_yMax; }
    int getMinScore() const { return m_yMin; }
    int getAverageScore() const;
    int getScoreCount() const { return m_scores.size(); }

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void calculateLayout();
    QPoint scoreToPoint(int index, int score) const;

private:
    QList<int> m_scores;
    QString m_title;
    int m_difficulty;

    // 绘图区域
    QRect m_chartRect;

    // 坐标轴范围
    int m_xMax;
    int m_yMax;
    int m_yMin;

    // 颜色
    QColor m_chartColor;
};

#endif 