#include "Board.h"

Board::Board() {
    m_rng.seed(std::time(nullptr));
    m_grid.resize(BOARD_ROWS, std::vector<Gem>(BOARD_COLS));
}

//初始化宝石盘
void Board::initRandomBoard(int gemTypeCount) {
    std::uniform_int_distribution<int> dist(1, gemTypeCount);

    for (int i = 0; i < BOARD_ROWS; ++i) {
        for (int j = 0; j < BOARD_COLS; ++j) {
            m_grid[i][j] = Gem(static_cast<GemType>(dist(m_rng)));
        }
    }
}

//得到对应坐标处的宝石
Gem Board::getGem(int row, int col) const {
    if (!isValid(row, col)) return Gem(GemType::Empty);
    return m_grid[row][col];
}

//设置对应坐标处的宝石
void Board::setGem(int row, int col, const Gem& gem) {
    if (isValid(row, col)) {
        m_grid[row][col] = gem;
    }
}

//交换两个宝石坐标
void Board::swapGem(int r1, int c1, int r2, int c2) {
    if (isValid(r1, c1) && isValid(r2, c2)) {
        std::swap(m_grid[r1][c1], m_grid[r2][c2]);
    }
}

//判断坐标是否合法
bool Board::isValid(int r, int c) const {
    return r >= 0 && r < BOARD_ROWS && c >= 0 && c < BOARD_COLS;
}

//得到二维数组宝石盘
const std::vector<std::vector<Gem>>& Board::getGrid() const {
    return m_grid;
}

//设置二维数组宝石盘
void Board::setGrid(const std::vector<std::vector<Gem>>& newGrid) {
    m_grid = newGrid;
}

