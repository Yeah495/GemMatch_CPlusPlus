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