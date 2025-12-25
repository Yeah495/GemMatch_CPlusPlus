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

std::vector<QPoint> GameCore::findHint() {
    // 遍历每一个格子
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {

            // 尝试向右交换
            if (c + 1 < BOARD_COLS) {
                m_board->swapGem(r, c, r, c + 1); // 假装交换
                if (!MatchFinder::findMatches(*m_board).empty()) {
                    m_board->swapGem(r, c, r, c + 1); // 还原
                    return { QPoint(r, c), QPoint(r, c + 1) }; // 找到啦！
                }
                m_board->swapGem(r, c, r, c + 1); // 还原
            }

            // 尝试向下交换
            if (r + 1 < BOARD_ROWS) {
                m_board->swapGem(r, c, r + 1, c); // 假装交换
                if (!MatchFinder::findMatches(*m_board).empty()) {
                    m_board->swapGem(r, c, r + 1, c); // 还原
                    return { QPoint(r, c), QPoint(r + 1, c) }; // 找到啦！
                }
                m_board->swapGem(r, c, r + 1, c); // 还原
            }
        }
    }
    return {}; // 没找到任何可以消除的一步（死局）
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
    // 1. 定义一个局部变量，作为本次扫描的“临时开关”
    // 只要发现一次4连，就把它设为 true，之后绝不改回 false
    bool hasSpecialEffect = false;

    std::set<std::pair<int, int>> pointsToExplode;

    // ==========================================
    // 1. 横向扫描 (Rows)
    // ==========================================
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ) {
            GemType currentType = m_board->getGem(r, c).type;
            if (currentType == GemType::Empty || m_board->getGem(r, c).state == GemState::Exploding) {
                c++; continue;
            }

            int k = c + 1;
            while (k < BOARD_COLS) {
                if (m_board->getGem(r, k).type == currentType && m_board->getGem(r, k).state != GemState::Exploding) k++;
                else break;
            }

            int matchLen = k - c;
            if (matchLen >= 3) {
                if (matchLen >= 4) {
                    // --- 触发大招 ---
                    hasSpecialEffect = true; // 【关键】标记为真！

                    for (int col = 0; col < BOARD_COLS; ++col) {
                        if (m_board->getGem(r, col).type != GemType::Empty)
                            pointsToExplode.insert({ r, col });
                    }
                }
                else {
                    // --- 普通消除 ---
                    // 【关键】这里只处理消除，绝对不要去修改 hasSpecialEffect
                    // 不要写 hasSpecialEffect = false; 
                    for (int j = c; j < k; ++j) {
                        pointsToExplode.insert({ r, j });
                    }
                }
            }
            c = k;
        }
    }

    // ==========================================
    // 2. 纵向扫描 (Cols)
    // ==========================================
    for (int c = 0; c < BOARD_COLS; ++c) {
        for (int r = 0; r < BOARD_ROWS; ) {
            GemType currentType = m_board->getGem(r, c).type;
            if (currentType == GemType::Empty || m_board->getGem(r, c).state == GemState::Exploding) {
                r++; continue;
            }

            int k = r + 1;
            while (k < BOARD_ROWS) {
                if (m_board->getGem(k, c).type == currentType && m_board->getGem(k, c).state != GemState::Exploding) k++;
                else break;
            }

            int matchLen = k - r;
            if (matchLen >= 3) {
                if (matchLen >= 4) {
                    // --- 触发大招 ---
                    hasSpecialEffect = true; // 【关键】标记为真！

                    for (int row = 0; row < BOARD_ROWS; ++row) {
                        if (m_board->getGem(row, c).type != GemType::Empty)
                            pointsToExplode.insert({ row, c });
                    }
                }
                else {
                    // --- 普通消除 ---
                    // 【关键】这里不做任何状态改变，防止覆盖掉之前可能已经发现的大招
                    for (int j = r; j < k; ++j) {
                        pointsToExplode.insert({ j, c });
                    }
                }
            }
            r = k;
        }
    }

    // ==========================================
    // 3. 应用状态 (Commit)
    // ==========================================
    if (pointsToExplode.empty()) {
        m_isSpecialMatch = false; // 没有消除，肯定是 false
        return false;
    }

    // 【最终提交】
    // 将本次扫描的最终结果赋值给成员变量
    // 这样，只要上面的循环里有任何一次触发了 >=4，这里就是 true
    m_isSpecialMatch = hasSpecialEffect;

    if (size) *size = pointsToExplode.size();

    for (const auto& p : pointsToExplode) {
        int r = p.first;
        int c = p.second;
        Gem g = m_board->getGem(r, c);
        g.state = GemState::Exploding;
        m_board->setGem(r, c, g);
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

int GameCore::explodeAllColor(GemType targetColor) {
    int count = 0;
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            Gem g = m_board->getGem(r, c);

            // 只要是目标颜色，或者它本身就是万能宝石（自己也要炸），就标记爆炸
            if (g.type == targetColor || g.type == GemType::Universal) {
                g.state = GemState::Exploding;
                m_board->setGem(r, c, g);
                count++;
            }
        }
    }
    return count;
}