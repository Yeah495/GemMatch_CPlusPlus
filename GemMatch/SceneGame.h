#pragma once
/*  a. 技术建议：使用 QGraphicsView 和 QGraphicsScene 框架。更适合做游戏，能支持平滑的宝石交换动画。
  b. 动画逻辑：当 Model 层发生交换时，View 层负责播放 0.3 秒的移动动画，动画结束再更新数据。*/





#ifndef SCENEGAME_H
#define SCENEGAME_H

#include <QGraphicsScene>
#include <QParallelAnimationGroup>
#include "GemItem.h"
#include "Board.h" // 仅用于初始化读取数据，不修改

class SceneGame : public QGraphicsScene {
    Q_OBJECT
public:
    explicit SceneGame(QObject* parent = nullptr);

    // 根据 Model 数据完全重绘界面（初始化或悔棋时用）
    void renderBoard(const Board& board);

    // --- 动画接口 (供 Controller 调用) ---

    // 1. 播放交换动画
    // finishedCallback 是动画结束后的回调，通知 Controller 进行下一步逻辑
    void animateSwap(int r1, int c1, int r2, int c2, std::function<void()> finishedCallback);

    // 2. 播放消除爆炸动画
    void animateExplosion(const std::vector<QPoint>& points, std::function<void()> finishedCallback);

    // 3. 播放下落动画 (包含新生成宝石的入场)
    // dropMoves: 记录每个位置的宝石是从哪一行掉下来的
    void animateFall(const Board& newBoard, std::function<void()> finishedCallback);

    // 设置选中状态（高亮显示）
    void setGemSelected(int r, int c, bool selected);

signals:
    // 转发宝石的点击信号给 Controller
    void gemClicked(int row, int col);

private:
    // 二维数组管理图元指针，方便查找
    GemItem* m_items[BOARD_ROWS][BOARD_COLS];

    // 屏幕布局常量
    const int CELL_SIZE = 65; // 格子大小（略大于宝石，留空隙）
    const int MARGIN_LEFT = 100;
    const int MARGIN_TOP = 100;

    // 辅助：计算屏幕坐标
    QPointF getScreenPos(int row, int col) const;
};

#endif // SCENEGAME_H