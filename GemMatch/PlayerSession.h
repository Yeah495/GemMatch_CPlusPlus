#pragma once

#ifndef PLAYERSESSION_H
#define PLAYERSESSION_H

#include "Board.h"
#include <stack>
#include <vector>


struct GameSnapshot {
    std::vector<std::vector<Gem>> grid;
    int score;
};

class PlayerSession {
public:
    PlayerSession();

    // 分数管理 
    void addScore(int points);
    int getScore() const;
    void resetScore();

    // 历史记录管理 
    void pushHistory(const Board& board); 
    bool canUndo() const;
    void undo(Board& currentBoard);       

private:
    int m_score;
    std::stack<GameSnapshot> m_historyStack;
};

#endif 