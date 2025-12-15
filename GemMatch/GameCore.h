#pragma once
/*● GameCore.h (外观总管)
  ○ 算法核心：处理消除后的下落填充。  
  ○ 职责：Controller 层的唯一交互对象。它内部持有 Board、MatchFinder 和 GravitySystem，协调三者工作。*/



#ifndef GAMECORE_H
#define GAMECORE_H

#include "Board.h"
#include "MatchFinder.h"
#include "GravitySystem.h"
#include "PlayerSession.h"
#include <memory>

  // 交换结果反馈，用于 Controller 判断播放什么动画
enum class SwapResult {
    Success,    // 交换并导致了消除
    Fail,       // 交换无效（需回弹）
    NoMatch     // 仅交换（特殊模式下可能允许不消除也交换，标准版通常算Fail）
};

class GameCore {
public:
    GameCore();

    void initGame(); // 开始新游戏

    // 核心交互 API [cite: 12]
    // 尝试交换两个宝石。如果成功消除，Model内部会自动处理消除和得分
    SwapResult trySwap(int r1, int c1, int r2, int c2);

    // 撤销上一步
    bool undo();

    // 供 View 获取数据用于渲染
    const Board& getBoard() const;
    int getScore() const;

    // 每一轮消除后的自动处理（消除->下落->再检查消除）
    // 返回是否还有连锁反应在进行（用于连续动画）
    bool processNextState();

private:
    // 组合模式：持有各子模块
    std::unique_ptr<Board> m_board;
    std::unique_ptr<PlayerSession> m_session;

    // 内部处理消除逻辑
    int executeElimination(); // 返回消除的个数
};

#endif // GAMECORE_H