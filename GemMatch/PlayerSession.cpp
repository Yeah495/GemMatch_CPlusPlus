/*● PlayerSession（玩家状态）
  ○ 描述：玩家状态。
  ○ 属性：当前分数、剩余步数/时间、当前关卡。
  ○ 数据结构应用 - 栈 (Stack)：
    ■ 扩展功能：实现“悔棋” (Undo) 功能。每次操作前，把当前地图状态快照压入栈中。*/



#include "PlayerSession.h"

PlayerSession::PlayerSession() : m_score(0) {}

void PlayerSession::addScore(int points) {
    m_score += points;
}

int PlayerSession::getScore() const {
    return m_score;
}

void PlayerSession::resetScore() {
    m_score = 0;
    while (!m_historyStack.empty()) {
        m_historyStack.pop();
    }
}

void PlayerSession::pushHistory(const Board& board) {
    GameSnapshot snapshot;
    snapshot.grid = board.getGrid();
    snapshot.score = m_score;
    m_historyStack.push(snapshot);
}

bool PlayerSession::canUndo() const {
    return !m_historyStack.empty();
}

void PlayerSession::undo(Board& currentBoard) {
    if (canUndo()) {
        GameSnapshot snapshot = m_historyStack.top();
        m_historyStack.pop();

        // 恢复数据
        currentBoard.setGrid(snapshot.grid);
        m_score = snapshot.score;
    }
}