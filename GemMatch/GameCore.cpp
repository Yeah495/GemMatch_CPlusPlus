/*● GameCore.h (外观总管)
  ○ 算法核心：处理消除后的下落填充。  
  ○ 职责：Controller 层的唯一交互对象。它内部持有 Board、MatchFinder 和 GravitySystem，协调三者工作。*/




#include "GameCore.h"

GameCore::GameCore() {
    m_board = std::unique_ptr<Board>(new Board());
    m_session = std::unique_ptr<PlayerSession>(new PlayerSession());
}

//初始化棋盘并清除所有自动产生的初始连消（保证稳定开局）
void GameCore::initGame() {
    m_board->initRandomBoard();
    m_session->resetScore();

    // 初始盘面可能自带消除，先静默处理掉，保证开局是静止的
    bool stable = false;
    while (!stable) {
        int count = executeElimination();
        if (count > 0) {
            GravitySystem::applyGravity(*m_board);
        }
        else {
            stable = true;
        }
    }
}

SwapResult GameCore::trySwap(int r1, int c1, int r2, int c2) {
    // 1. 验证坐标
    if (!m_board->isValid(r1, c1) || !m_board->isValid(r2, c2)) return SwapResult::Fail;

    // 2. 必须是相邻的
    if (std::abs(r1 - r2) + std::abs(c1 - c2) != 1) return SwapResult::Fail;

    // 3. 记录历史（压栈），准备悔棋用 
    m_session->pushHistory(*m_board);

    // 4. 逻辑交换
    m_board->swapGem(r1, c1, r2, c2);

    // 5. 检查是否产生消除 
    auto matches = MatchFinder::findMatches(*m_board);
    if (matches.empty()) {
        // 交换失败：数据回滚（不存入历史栈，或者弹出刚才压入的）
        // 这里简单处理：再换回来
        m_board->swapGem(r1, c1, r2, c2);
        // 既然无效，就把刚才压入的无效历史弹出来，避免悔棋回到这个错误状态
        Board tempBoard;
        m_session->undo(tempBoard);
        return SwapResult::Fail;
    }

    // 6. 交换成功：执行消除逻辑
    // 注意：这里不立即执行 Gravity，而是标记消除，让Controller播放“爆炸动画”
    // 爆炸动画结束后，Controller 会再次调用 processNextState 来处理下落
    for (const auto& p : matches) {
        Gem g = m_board->getGem(p.r, p.c);
        g.type = GemType::Empty; // 逻辑上变空
        g.state = GemState::Exploding; // 状态设为爆炸，供UI显示
        m_board->setGem(p.r, p.c, g);
    }

    // 计算得分 
    m_session->addScore(matches.size() * 10);

    return SwapResult::Success;
}

bool GameCore::undo() {
    if (m_session->canUndo()) {
        m_session->undo(*m_board);
        return true;
    }
    return false;
}

int GameCore::executeElimination() {
    auto matches = MatchFinder::findMatches(*m_board);
    for (const auto& p : matches) {
        Gem g = m_board->getGem(p.r, p.c);
        g.type = GemType::Empty;
        m_board->setGem(p.r, p.c, g);
    }
    return matches.size();
}

bool GameCore::processNextState() {
    // 1. 先应用重力（下落）
    bool gravityMoved = GravitySystem::applyGravity(*m_board);

    // 2. 下落后检查是否有新的消除（连锁反应）
    int eliminated = executeElimination();
    if (eliminated > 0) {
        m_session->addScore(eliminated * 20); // 连击得分更高
        // 如果有消除，说明状态还没稳，需要UI继续播放消除动画，然后再次回调这里
        return true;
    }

    // 如果仅仅是重力下落了，但没有新消除，也需要返回true让UI播放下落动画
    // 只有当既没有下落也没有消除时，回合才算彻底结束
    return gravityMoved;
}

const Board& GameCore::getBoard() const {
    return *m_board;
}

int GameCore::getScore() const {
    return m_session->getScore();
}