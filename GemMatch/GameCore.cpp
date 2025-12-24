/*● GameCore.h (外观总管)
  ○ 算法核心：处理消除后的下落填充。  
  ○ 职责：Controller 层的唯一交互对象。它内部持有 Board、MatchFinder 和 GravitySystem，协调三者工作。*/




#include "GameCore.h"

#include <algorithm>     // for std::shuffle
#include <random>
#include <ctime>
#include <cstdlib>
#include <chrono>

GameCore::GameCore() {
    m_board = std::unique_ptr<Board>(new Board());
    m_session = std::unique_ptr<PlayerSession>(new PlayerSession());
}

//初始化棋盘并清除所有自动产生的初始连消（保证稳定开局）
void GameCore::initGame(int gemTypeCount) {
    m_board->initRandomBoard(gemTypeCount);
    m_session->resetScore();

    // 先执行一次重力，填补初始生成的任何空洞
    GravitySystem::applyGravity(*m_board, gemTypeCount);

    bool stable = false;
    while (!stable) {
        // 使用 MatchFinder 找但不标记状态，直接清空
        auto matches = MatchFinder::findMatches(*m_board);
        if (!matches.empty()) {
            for (auto& p : matches) {
                Gem g = m_board->getGem(p.r, p.c);
                g.type = GemType::Empty;
                m_board->setGem(p.r, p.c, g);
            }
            GravitySystem::applyGravity(*m_board, gemTypeCount);
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

void GameCore::applyGravityOnly(int gemTypeCount) {
    // 调用之前的队列逻辑，实现宝石下沉和顶部补全
    GravitySystem::applyGravity(*m_board, gemTypeCount);
}

const Board& GameCore::getBoard() const {
    return *m_board;
}

void GameCore::addScoreSession(int score){
   m_session->addScore(score);
}

int GameCore::getScore() const {
    return m_session->getScore();
}

void GameCore::shuffleBoard() {
    // 1. 收集所有非空格子（只收集颜色）
    std::vector<GemType> gems;
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            Gem g = m_board->getGem(r, c);
            if (g.type != GemType::Empty) {
                gems.push_back(g.type);
            }
        }
    }

    if (gems.empty()) return;

    // 2. 纯随机打乱 (不检查是否连消，生成啥就是啥)
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine engine(seed);
    std::shuffle(gems.begin(), gems.end(), engine);

    // 3. 填回棋盘
    int idx = 0;
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            Gem g = m_board->getGem(r, c);
            if (g.type != GemType::Empty) {
                g.type = gems[idx++];
                g.state = GemState::Static; // 设为静止
                m_board->setGem(r, c, g);
            }
        }
    }
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