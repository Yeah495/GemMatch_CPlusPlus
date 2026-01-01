#pragma once
#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <random>
#include <ctime>
#include "Gem.h"

class Board {
public:
    Board();

    //初始化地图
    void initRandomBoard(int gemTypeCount = 3);

    //获取/设置宝石
    Gem getGem(int row, int col) const;
    void setGem(int row, int col, const Gem& gem);

    //交换两个位置的宝石
    void swapGem(int r1, int c1, int r2, int c2);

    //检查坐标是否有效
    bool isValid(int r, int c) const;

    //获取原始二维数组数据
    const std::vector<std::vector<Gem>>& getGrid() const;
    void setGrid(const std::vector<std::vector<Gem>>& newGrid);

private:
    //数据结构：二维数组 
    std::vector<std::vector<Gem>> m_grid;

    //随机数生成器
    std::mt19937 m_rng;
};

#endif // BOARD_H