#pragma once
/*● PlayerSession（玩家状态）
  ○ 描述：玩家状态。
  ○ 属性：当前分数、剩余步数/时间、当前关卡。
  ○ 数据结构应用 - 栈 (Stack)：
    ■ 扩展功能：实现“悔棋” (Undo) 功能。每次操作前，把当前地图状态快照压入栈中。*/


#ifndef PLAYERSESSION_H
#define PLAYERSESSION_H

#include "Board.h"
#include <stack>
#include <vector>

    // 存储快照，用于撤销
struct GameSnapshot {
    std::vector<std::vector<Gem>> grid;
    int score;
};

class PlayerSession {
public:
    PlayerSession();

    // 分数管理 [cite: 15]
    void addScore(int points);
    int getScore() const;
    void resetScore();

    // 历史记录管理 (Stack)
    void pushHistory(const Board& board); // 操作前保存状态
    bool canUndo() const;
    void undo(Board& currentBoard);       // 恢复状态

private:
    int m_score;
    // 数据结构应用：栈，用于存储历史状态 [cite: 7]
    std::stack<GameSnapshot> m_historyStack;
};

#endif // PLAYERSESSION_H