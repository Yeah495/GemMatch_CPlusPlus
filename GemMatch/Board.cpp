/*● Board.h (纯数据容器)
  ○ 描述：整个棋盘的数据抽象。
  ○ 数据结构应用 - 二维数组：Gem grid[8][8]*/



#include "Board.h"

Board::Board() {
    m_rng.seed(std::time(nullptr));
    m_grid.resize(BOARD_ROWS, std::vector<Gem>(BOARD_COLS));
}

void Board::initRandomBoard(int gemTypeCount) {
    // 简单的随机填充，实际项目中通常需要检查生成时不能直接出现3连
    // 这里为了简化，先完全随机，GameCore 初始化时会进行一次清理
    std::uniform_int_distribution<int> dist(1, gemTypeCount);

    for (int i = 0; i < BOARD_ROWS; ++i) {
        for (int j = 0; j < BOARD_COLS; ++j) {
            m_grid[i][j] = Gem(static_cast<GemType>(dist(m_rng)));
        }
    }
}

Gem Board::getGem(int row, int col) const {
    if (!isValid(row, col)) return Gem(GemType::Empty);
    return m_grid[row][col];
}

void Board::setGem(int row, int col, const Gem& gem) {
    if (isValid(row, col)) {
        m_grid[row][col] = gem;
    }
}

void Board::swapGem(int r1, int c1, int r2, int c2) {
    if (isValid(r1, c1) && isValid(r2, c2)) {
        std::swap(m_grid[r1][c1], m_grid[r2][c2]);
    }
}

bool Board::isValid(int r, int c) const {
    return r >= 0 && r < BOARD_ROWS && c >= 0 && c < BOARD_COLS;
}

const std::vector<std::vector<Gem>>& Board::getGrid() const {
    return m_grid;
}

void Board::setGrid(const std::vector<std::vector<Gem>>& newGrid) {
    m_grid = newGrid;
}

