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

    //结束游戏
    void endGame();

    bool isPaused() const { return m_isPaused; }

    void stopAllSounds();  //停止音效

public slots:
    void onGemClicked(int row, int col);
    void onGameTick();
    void onPauseClicked();
    void onSkillBomb();
    void onSkillShuffle();
    void onSkillTime();
    void onSkillAll();
    void onHintClicked();   
signals:
    void gameOver(int finalScore); //游戏结束信号

private:
    GameCore* m_gameCore;   //Model对象
    SceneGame* m_scene;     //View 对象
    MainWindow* m_mainWindow;

    //尝试交换两个宝石
    void attemptSwap(const QPoint& p1, const QPoint& p2);

    //处理消除后的下落和连锁反应
    void processFallAndMatch();

    //获取难度名称
    QString getDifficultyName(int difficulty);

    //统一刷新按钮文字
    void updateSkillButtons();

    QPoint m_selectedPos;   //当前选中的宝石坐标
   
    //时间管理
    QTimer* m_gameTimer;
    int m_remainingTime;
    const int GAME_DURATION = 60; // 游戏时长常量

    //音效
    QSoundEffect* m_soundClick;
    QSoundEffect* m_soundMouth;
    QSoundEffect* m_soundBoom;
    QSoundEffect* m_soundShuffle;
    QSoundEffect* m_soundIce;
    QSoundEffect* m_soundAll;
    QSoundEffect* m_soundLaser;

    //技能次数
    const int MAX_BOMB_COUNT = 3;
    const int MAX_SHUFFLE_COUNT = 1;
    const int MAX_TIME_COUNT = 2;
    const int MAX_ALL_COUNT = 1;

    //当前剩余次数
    int m_remainBomb;
    int m_remainShuffle;
    int m_remainTime;
    int m_remainAll;

    bool m_isTerminated = false; //游戏终止
    bool m_isProcessing;    //是否正在播放动画
    bool m_isPaused;       // 暂停状态
    bool m_isTimeFrozen;   // 时间冻结状态
    int m_freezeCounter;   // 冻结倒计时

    int m_comboLevel; //记录当前的连击层数
    int m_currentDifficulty; //当前游戏难度（3=简单，5=普通，7=困难）
};

#endif // GAMECONTROLLER_H