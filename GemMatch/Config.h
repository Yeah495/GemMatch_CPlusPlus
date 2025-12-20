#pragma once
#ifndef CONFIG_H
#define CONFIG_H

// 游戏配置 
const int BOARD_ROWS = 8;
const int BOARD_COLS = 8;
const int GEM_TYPE_COUNT = 7; // 7种不同颜色的宝石

// 宝石类型枚举
enum class GemType {
    Empty = 0, // 空（消除后）
    Red,
    Blue,
    Green,
    Yellow,
    Purple,
    Orange,
    White
};

// 宝石状态（用于UI动画判断，Model层主要负责标记）
enum class GemState {
    Static,     // 静止
    Swapping,   // 交换中
    Exploding,  // 消除爆炸中
    Falling     // 下落中
};

#endif // CONFIG_H