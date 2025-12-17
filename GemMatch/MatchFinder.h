/*● MatchFinder.h (算法引擎)  
  ○ 数据结构应用 - 图 (Graph)：
    ■ 难点：判断消除时，把同色相邻宝石看作“连通分量”。
    ■ 算法：实现 BFS (广度优先搜索) 或 DFS 查找相连的同色块。若连通数量 ≥3\geq 3≥3，则标记为消除。*/



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
    // 核心算法：查找所有满足消除条件的宝石坐标 
    static std::vector<Point> findMatches(const Board& board);

private:
    // 辅助：检查水平和垂直方向的连续性
    static void checkLines(const Board& board, std::set<Point>& matches);
};

#endif // MATCHFINDER_H