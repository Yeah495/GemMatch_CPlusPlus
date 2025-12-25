#pragma once
#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QObject>
#include <QPoint>
#include <QTimer>
#include <QSoundEffect>
#include <chrono>
#include <random>
#include <algorithm>
#include "GameCore.h"
#include "SceneGame.h"
#include "MainWindow.h"

class GameController : public QObject {
    Q_OBJECT
public:
    explicit GameController(MainWindow* view, QObject* parent = nullptr);

    // 启动游戏
    void startGame(int difficultyLevel);

    // 撤销操作
    void undo();

    void endGame();

    bool isPaused() const { return m_isPaused; }

    void stopAllSounds();

public slots:
    // 响应 View 层的宝石点击
    void onGemClicked(int row, int col);
    void onGameTick(); //定时器槽函数

    void onPauseClicked();
    void onSkillBomb();
    void onSkillShuffle();
    void onSkillTime();
    void onSkillAll();
    void onHintClicked();

private:
    // --- 内部流程控制函数 ---

    // 尝试交换两个宝石
    void attemptSwap(const QPoint& p1, const QPoint& p2);

    // 处理消除后的下落和连锁反应 (递归核心)
    void processFallAndMatch();

    // 辅助函数：获取难度名称
    QString getDifficultyName(int difficulty);

    // 辅助函数：统一刷新按钮文字
    void updateSkillButtons();

    // --- 成员变量 ---

    GameCore* m_gameCore;   // Model 对象
    SceneGame* m_scene;     // View 对象 (通过 MainWindow 获取)
    MainWindow* m_mainWindow;

    QPoint m_selectedPos;   // 当前选中的宝石坐标 (-1, -1 表示未选)
    bool m_isProcessing;    // 锁：是否正在播放动画（禁止玩家点击）

    int m_comboLevel; // 记录当前的连击层数
    QTimer* m_gameTimer;
    int m_remainingTime;
    const int GAME_DURATION = 60; // 游戏时长常量

    bool m_isPaused;       // 暂停状态
    bool m_isTimeFrozen;   // 时间冻结状态
    int m_freezeCounter;   // 冻结倒计时

    QSoundEffect* m_soundClick;
    QSoundEffect* m_soundMouth;
    QSoundEffect* m_soundBoom;
    QSoundEffect* m_soundShuffle;
    QSoundEffect* m_soundIce;
    QSoundEffect* m_soundAll;
    QSoundEffect* m_soundLaser;

    int m_currentDifficulty; // 当前游戏难度（3=简单，5=普通，7=困难）

    const int MAX_BOMB_COUNT = 3;
    const int MAX_SHUFFLE_COUNT = 1;
    const int MAX_TIME_COUNT = 2;
    const int MAX_ALL_COUNT = 1;

    //当前剩余次数
    int m_remainBomb;
    int m_remainShuffle;
    int m_remainTime;
    int m_remainAll;
    bool m_isTerminated = false;
};

#endif // GAMECONTROLLER_H