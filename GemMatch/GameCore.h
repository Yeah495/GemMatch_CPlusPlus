#pragma once
#ifndef GAMECORE_H
#define GAMECORE_H

#include "Board.h"
#include "MatchFinder.h"
#include "GravitySystem.h"
#include "PlayerSession.h"
#include <memory>
#include <qpoint.h>
#include <set>

//交换结果枚举
enum class SwapResult {
    Success,
    Fail,   
    NoMatch 
};

class GameCore {
public:
    GameCore();

    void initGame(int gemTypeCount = 3); //开始新游戏
    void resetGemStates(); //重置宝石状态
 
    //尝试交换两个宝石
    SwapResult trySwap(int r1, int c1, int r2, int c2);

    //得到宝石盘
    const Board& getBoard() const;
    
    //得到分数
    int getScore() const;
    void addScoreSession(int score); //添加分数

    //查找可消除的宝石
    bool findAndMarkMatches(int* size = nullptr);
    void clearMatches();
    //执行重力下落
    void applyGravityOnly(int gemTypeCount = 3);
	
    //技能接口
    int explodeAllColor(GemType targetColor);  //标记颜色爆炸状态
    void shuffleBoard(); //打乱宝石盘
    std::vector<QPoint> findHint();  //找到消除提示

    Board* getBoardPtr() { return m_board.get(); }  //得到宝石盘指针,不是常对象了,可以访问setGem这个非常函数

    bool isSpecialMatch() const { return m_isSpecialMatch; }

    //撤销上一步
    bool undo();
private:
    //宝石盘和用户数据
    std::unique_ptr<Board> m_board;
    std::unique_ptr<PlayerSession> m_session;
    bool m_isSpecialMatch = false;
};

#endif // GAMECORE_H