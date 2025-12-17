/*● GravitySystem.h (物理规则)  
  ○ 数据结构应用 - 队列 (Queue)：
    ■ 用途：消除后上方宝石下落。对于每一列，可以把非空格子入队，然后重新从底部向上填入数组，空出的顶部生成新宝石。*/



#include "GravitySystem.h"

bool GravitySystem::applyGravity(Board& board) {
    bool moved = false;
    std::uniform_int_distribution<int> dist(1, GEM_TYPE_COUNT);
    std::mt19937 rng(std::time(nullptr));

    // 对每一列单独处理
    for (int c = 0; c < BOARD_COLS; ++c) {
        // 数据结构应用：队列 
        // 逻辑：将该列所有非空宝石按顺序入队
        std::queue<Gem> columnQueue;

        for (int r = 0; r < BOARD_ROWS; ++r) {
            Gem g = board.getGem(r, c);
            if (g.type != GemType::Empty) {
                columnQueue.push(g);
            }
        }

        // 如果队列大小等于行数，说明这列满了，没有消除，不需要下落
        if (columnQueue.size() == BOARD_ROWS) {
            continue;
        }

        moved = true;

        // 计算需要填充的空位数量（顶部）
        int emptySlots = BOARD_ROWS - columnQueue.size();

        // 1. 先在顶部填充新的随机宝石
        for (int r = 0; r < emptySlots; ++r) {
            Gem newGem(static_cast<GemType>(dist(rng)));
            newGem.state = GemState::Falling; // 标记状态供View层做入场动画
            board.setGem(r, c, newGem);
        }

        // 2. 再将队列中的旧宝石填回去（相当于下落到了底部）
        int currentRow = emptySlots;
        while (!columnQueue.empty()) {
            Gem g = columnQueue.front();
            columnQueue.pop();
            // 重置状态
            if (g.state == GemState::Exploding) g.state = GemState::Static;
            board.setGem(currentRow, c, g);
            currentRow++;
        }
    }

    return moved;
}