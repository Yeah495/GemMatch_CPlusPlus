/*● MatchFinder.h (算法引擎)
  ○ 数据结构应用 - 图 (Graph)：
    ■ 难点：判断消除时，把同色相邻宝石看作“连通分量”。
    ■ 算法：实现 BFS (广度优先搜索) 或 DFS 查找相连的同色块。若连通数量 ≥3\geq 3≥3，则标记为消除。*/


#include "MatchFinder.h"

std::vector<Point> MatchFinder::findMatches(const Board& board) {
    std::set<Point> uniqueMatches; // 使用Set去重，因为一个宝石可能同时在横竖两个方向被消除
    checkLines(board, uniqueMatches);

    // 转换为Vector返回
    return std::vector<Point>(uniqueMatches.begin(), uniqueMatches.end());
}

void MatchFinder::checkLines(const Board& board, std::set<Point>& matches) {
    // 1. 横向扫描 (Row by Row)
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS - 2; ++c) {
            GemType currentType = board.getGem(r, c).type;
            if (currentType == GemType::Empty) continue;

            if (board.getGem(r, c + 1).type == currentType &&
                board.getGem(r, c + 2).type == currentType) {
                // 发现至少3个相连
                matches.insert({ r, c });
                matches.insert({ r, c + 1 });
                matches.insert({ r, c + 2 });

                // 继续向后检查是否超过3个
                int k = c + 3;
                while (k < BOARD_COLS && board.getGem(r, k).type == currentType) {
                    matches.insert({ r, k });
                    k++;
                }
            }
        }
    }

    // 2. 纵向扫描 (Col by Col)
    for (int c = 0; c < BOARD_COLS; ++c) {
        for (int r = 0; r < BOARD_ROWS - 2; ++r) {
            GemType currentType = board.getGem(r, c).type;
            if (currentType == GemType::Empty) continue;

            if (board.getGem(r + 1, c).type == currentType &&
                board.getGem(r + 2, c).type == currentType) {
                matches.insert({ r, c });
                matches.insert({ r + 1, c });
                matches.insert({ r + 2, c });

                int k = r + 3;
                while (k < BOARD_ROWS && board.getGem(k, c).type == currentType) {
                    matches.insert({ k, c });
                    k++;
                }
            }
        }
    }
}