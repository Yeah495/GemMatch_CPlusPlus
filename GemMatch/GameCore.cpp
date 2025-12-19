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

    // 先执行一次重力，填补初始生成的任何空洞
    GravitySystem::applyGravity(*m_board); 

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
    
    // 【重要】初始化结束后，强制将所有宝石设为静止，避免带着 Falling 状态进游戏
    for(int r=0; r<BOARD_ROWS; ++r)
        for(int c=0; c<BOARD_COLS; ++c) {
             Gem g = m_board->getGem(r,c);
             g.state = GemState::Static;
             m_board->setGem(r,c,g);
        }
}

void GameCore::resetGemStates() {
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            Gem g = m_board->getGem(r, c);
            // 不管之前是什么状态，只要存在，就归位静止
            if (g.type != GemType::Empty) {
                g.state = GemState::Static;
                m_board->setGem(r, c, g);
            }
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

    // 检查是否产生消除
    int size = 0;
    if (!findAndMarkMatches(&size)) { // 使用统一的标记函数
        m_board->swapGem(r1, c1, r2, c2); // 回退
        Board tempBoard;
        m_session->undo(tempBoard);
        return SwapResult::Fail;
    }

    // 计算得分 
    m_session->addScore(size * 10);

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

void GameCore::applyGravityOnly() {
    // 调用之前的队列逻辑，实现宝石下沉和顶部补全
    GravitySystem::applyGravity(*m_board);
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
    return !gravityMoved;
}

const Board& GameCore::getBoard() const {
    return *m_board;
}

int GameCore::getScore() const {
    return m_session->getScore();
}

// 确保 findAndMarkMatches 安全
bool GameCore::findAndMarkMatches(int* size) {
    auto matches = MatchFinder::findMatches(*m_board);
    if (size) *size = matches.size(); // 增加空指针保护

    if (matches.empty()) return false;

    for (const auto& p : matches) {
        Gem g = m_board->getGem(p.r, p.c);
        g.state = GemState::Exploding; // 仅改状态，不改 type
        m_board->setGem(p.r, p.c, g);
    }
    return true;
}

// 确保 clearMatches 彻底清除爆炸痕迹
void GameCore::clearMatches() {
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            Gem g = m_board->getGem(r, c);
            if (g.state == GemState::Exploding) {
                g.type = GemType::Empty;  // 正式变为空
                g.state = GemState::Static; // 重置为静态，防止下一轮误判
                m_board->setGem(r, c, g);
            }
        }
    }
}