#pragma once
/*● GameController.h/cpp
  ○ 职责：监听 View 的点击信号，调用 Model 的函数。
  ○ 流程：
    ⅰ. 用户在 SceneGame 点击宝石 A 和 B。
    ⅱ. SceneGame 告诉 GameController：“用户想交换 (x1, y1) 和 (x2, y2)”。
    ⅲ. GameController 调用 GameMap::trySwap(...)。
    ⅳ. 如果成功，GameMap 返回消除数据，GameController 通知 SceneGame 播放动画并更新分数。*/






#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QObject>
#include <QPoint>
#include <QTimer>
#include "GameCore.h"
#include "SceneGame.h"
#include "MainWindow.h"

class GameController : public QObject {
    Q_OBJECT
public:
    explicit GameController(MainWindow* view, QObject* parent = nullptr);

    // 启动游戏
    void startGame();

    // 撤销操作
    void undo();

    void endGame();

public slots:
    // 响应 View 层的宝石点击
    void onGemClicked(int row, int col);
    void onGameTick(); //定时器槽函数

private:
    // --- 内部流程控制函数 ---

    // 尝试交换两个宝石
    void attemptSwap(const QPoint& p1, const QPoint& p2);

    // 处理消除后的下落和连锁反应 (递归核心)
    void processFallAndMatch();

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
};

#endif // GAMECONTROLLER_H