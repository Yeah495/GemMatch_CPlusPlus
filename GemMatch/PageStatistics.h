#pragma once
#ifndef PAGESTATISTICS_H
#define PAGESTATISTICS_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QGraphicsProxyWidget>
#include <QListWidget>
#include <QLabel>
#include <QGridLayout>

#include "ScoreChartWidget.h"
#include "GameButton.h"
#include "GameLogo.h"

class MainWindow;

class PageStatistics : public QWidget {
    Q_OBJECT

public:
    explicit PageStatistics(MainWindow* mainWin);
    ~PageStatistics();

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onDifficultyChanged(int difficulty);
    void onBackClicked();

private:
    void setupUI();
    void loadStatisticsData();
    void updateChart(int difficultyLevel);
    void updateAchievements();

    // 成就检查函数
    void checkBasicAchievements();
    void checkScoreAchievements();
    void unlockAchievement(const QString& id, const QString& name, const QString& desc);
    bool isAchievementUnlocked(const QString& id);
    void loadAchievementProgress();
    void saveAchievementProgress();

private:
    MainWindow* m_mainWin;

    // 视频背景
    QGraphicsView* m_view;
    QGraphicsVideoItem* m_videoItem;
    QMediaPlayer* m_player;
    QAudioOutput* m_audioOutput;
    QString m_videoPath;

    // UI组件
    GameLogo* m_logo;
    GameButton* m_btnBack;
    GameButton* m_btnEasy;
    GameButton* m_btnNormal;
    GameButton* m_btnHard;

    // 统计图表
    ScoreChartWidget* m_chartWidget;

    // 数据标签
    QLabel* m_lblTotalGames;
    QLabel* m_lblAverageScore;
    QLabel* m_lblMaxScore;
    QLabel* m_lblWinRate;

    // 成就系统
    QListWidget* m_achievementsList;

    // 代理对象
    QGraphicsProxyWidget* m_containerProxy;
    QGraphicsProxyWidget* m_logoProxy;

    // 当前选择的难度
    int m_currentDifficulty;

    // 成就数据结构
    struct Achievement {
        QString id;
        QString name;
        QString description;
        bool unlocked;
        int progress;
        int target;
    };

    QList<Achievement> m_achievements;
};

#endif // PAGESTATISTICS_H