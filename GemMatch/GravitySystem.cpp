#include "GravitySystem.h"

//重力系统：让宝石下落并填充空位(效果是更新board)
bool GravitySystem::applyGravity(Board& board , int gemTypeCount) {
    bool moved = false;
    static std::mt19937 rng(std::time(nullptr));  //生成随机宝石
    std::uniform_int_distribution<int> dist(1, gemTypeCount);

    for (int c = 0; c < BOARD_COLS; ++c) {
        std::vector<std::pair<int, Gem>> existingGems;

        //收集非空宝石
        for (int r = 0; r < BOARD_ROWS; ++r) {
            Gem g = board.getGem(r, c);
            if (g.type != GemType::Empty) {
                //存入 {行号, 宝石}
                existingGems.push_back({ r, g });
            }
        }

        if (existingGems.size() < BOARD_ROWS) {
            moved = true;
            int emptySlots = BOARD_ROWS - existingGems.size();

            //填充顶部（新生成的宝石）
            for (int r = 0; r < emptySlots; ++r) {
                Gem newGem(static_cast<GemType>(dist(rng)));
                //新生成的肯定是从天上下来的，必须 Falling
                newGem.state = GemState::Falling;
                board.setGem(r, c, newGem);
            }

            //填充底部（旧宝石）
            for (int i = 0; i < existingGems.size(); ++i) {
                // 取出原行号和宝石数据
                int originalRow = existingGems[i].first;
                Gem g = existingGems[i].second;

                // 计算新行号
                int newRow = i + emptySlots;

                // 只有当新位置 != 旧位置 时，才标记为 Falling
                if (newRow != originalRow) {
                    g.state = GemState::Falling;
                }
                else {
                    g.state = GemState::Static;
                }
                //更新board
                board.setGem(newRow, c, g);
            }
        }
    }
    return moved;
}