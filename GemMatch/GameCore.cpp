#include "GameCore.h"

#include <algorithm> 
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

    //先执行一次重力，填补初始生成的任何空洞
    GravitySystem::applyGravity(*m_board, gemTypeCount);

    //清除初始连消
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
    
    //初始化结束后，将所有宝石设为静止，避免带着 Falling 状态进游戏(重力系统导致)
    resetGemStates();
}

//重置所有宝石状态为静止
void GameCore::resetGemStates() {
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            Gem g = m_board->getGem(r, c);
            if (g.type != GemType::Empty) {
                g.state = GemState::Static;
                m_board->setGem(r, c, g);
            }
        }
    }
}

//找提示
std::vector<QPoint> GameCore::findHint() {
    //遍历每一个格子
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {

            //尝试向右交换
            if (c + 1 < BOARD_COLS) {
                m_board->swapGem(r, c, r, c + 1);
                if (!MatchFinder::findMatches(*m_board).empty()) {
                    m_board->swapGem(r, c, r, c + 1); //还原
                    return { QPoint(r, c), QPoint(r, c + 1) }; //找到
                }
                m_board->swapGem(r, c, r, c + 1); //还原
            }

            // 尝试向下交换
            if (r + 1 < BOARD_ROWS) {
                m_board->swapGem(r, c, r + 1, c);
                if (!MatchFinder::findMatches(*m_board).empty()) {
                    m_board->swapGem(r, c, r + 1, c); //还原
                    return { QPoint(r, c), QPoint(r + 1, c) }; //找到
                }
                m_board->swapGem(r, c, r + 1, c); //还原
            }
        }
    }
    return {}; //没找到任何可以消除的一步（死局）
}

//尝试交换
SwapResult GameCore::trySwap(int r1, int c1, int r2, int c2) {
    //验证坐标
    if (!m_board->isValid(r1, c1) || !m_board->isValid(r2, c2)) return SwapResult::Fail;

    //必须是相邻的
    if (std::abs(r1 - r2) + std::abs(c1 - c2) != 1) return SwapResult::Fail;

    //记录历史
    m_session->pushHistory(*m_board);

    //逻辑交换
    m_board->swapGem(r1, c1, r2, c2);

    //检查是否产生消除
    int size = 0;
    if (!findAndMarkMatches(&size)) {
        m_board->swapGem(r1, c1, r2, c2); //还原
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

//重力接口
void GameCore::applyGravityOnly(int gemTypeCount) {
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

//打乱盘
void GameCore::shuffleBoard() {
    //收集所有非空格子（只收集颜色）
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

    //纯随机打乱
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine engine(seed);
    std::shuffle(gems.begin(), gems.end(), engine);

    //填回棋盘
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

//返回成功与否的删除,并且设置特殊情况和正常情况的消除状态
bool GameCore::findAndMarkMatches(int* size) {
    //只要发现一次4个以上连，就把它设为 true
    bool hasSpecialEffect = false;

    std::set<std::pair<int, int>> pointsToExplode;

    //横向扫描
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
                    hasSpecialEffect = true; //标记为真

                    for (int col = 0; col < BOARD_COLS; ++col) {
                        if (m_board->getGem(r, col).type != GemType::Empty)
                            pointsToExplode.insert({ r, col });
                    }
                }
                else {
                    //普通消除
                    for (int j = c; j < k; ++j) {
                        pointsToExplode.insert({ r, j });
                    }
                }
            }
            c = k;
        }
    }


    //纵向扫描
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
                    hasSpecialEffect = true; //标记为真

                    //对应列都为消除
                    for (int row = 0; row < BOARD_ROWS; ++row) {
                        if (m_board->getGem(row, c).type != GemType::Empty)
                            pointsToExplode.insert({ row, c });
                    }
                }
                else {
                    //普通消除
                    for (int j = r; j < k; ++j) {
                        pointsToExplode.insert({ j, c });
                    }
                }
            }
            r = k;
        }
    }


    //设置状态
    if (pointsToExplode.empty()) {
        m_isSpecialMatch = false;
        return false;
    }
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

//清除爆炸状态(设置为空,才能重力下落)
void GameCore::clearMatches() {
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            Gem g = m_board->getGem(r, c);
            if (g.state == GemState::Exploding) {
                g.type = GemType::Empty;  //变为空
                g.state = GemState::Static; //重置为静态，防止下一轮误判
                m_board->setGem(r, c, g);
            }
        }
    }
}

//万能宝石消除指定颜色
int GameCore::explodeAllColor(GemType targetColor) {
    int count = 0;
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            Gem g = m_board->getGem(r, c);
            // 只要是目标颜色，或者它本身就是万能宝石（自己也要炸）
            if (g.type == targetColor || g.type == GemType::Universal) {
                g.state = GemState::Exploding;
                m_board->setGem(r, c, g);
                count++;
            }
        }
    }
    return count;
}