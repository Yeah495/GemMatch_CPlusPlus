#pragma once
/*● GravitySystem.h (物理规则)  
  ○ 数据结构应用 - 队列 (Queue)：
    ■ 用途：消除后上方宝石下落。对于每一列，可以把非空格子入队，然后重新从底部向上填入数组，空出的顶部生成新宝石。*/



#ifndef GRAVITYSYSTEM_H
#define GRAVITYSYSTEM_H

#include "Board.h"
#include <queue>

class GravitySystem {
public:
    // 应用重力：处理空洞，上方宝石下落，顶部生成新宝石
    // 返回：是否发生了下落（用于判断动画是否需要继续）
    static bool applyGravity(Board& board);
};

#endif // GRAVITYSYSTEM_H