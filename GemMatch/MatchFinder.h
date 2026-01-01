#ifndef MATCHFINDER_H
#define MATCHFINDER_H

#include "Board.h"
#include <vector>
#include <set>

struct Point {
    int r, c;
    bool operator<(const Point& other) const {
        return r < other.r || (r == other.r && c < other.c);
    }
};

class MatchFinder {
public:
    //核心算法：查找所有满足消除条件的宝石坐标
    static std::vector<Point> findMatches(const Board& board);

private:
    //检查水平和垂直方向的连续性
    static void checkLines(const Board& board, std::set<Point>& matches);
};

#endif // MATCHFINDER_H