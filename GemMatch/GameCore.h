#pragma once
/*● GameCore.h 
Model 层对外的唯一接口，封装了复杂的子系统交互。

职责： 初始化游戏、处理交换请求、执行消除循环。

核心逻辑 (trySwap)：

验证与记录： 检查移动合法性，并立即将当前状态压入历史栈（为了悔棋）。

试错机制： 先交换数据，如果没有产生消除，则立即强制换回来（逻辑回滚），并弹出刚才无效的历史记录。

状态流转： 如果交换成功，不立即处理下落，而是将宝石标记为 Exploding 状态，等待 View 层播放完动画后再调用 processNextState。。*/



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