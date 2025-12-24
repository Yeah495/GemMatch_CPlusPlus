/*● GravitySystem.h (物理规则)  
  ○ 数据结构应用 - 队列 (Queue)：
    ■ 用途：消除后上方宝石下落。对于每一列，可以把非空格子入队，然后重新从底部向上填入数组，空出的顶部生成新宝石。*/



#include "GravitySystem.h"

bool GravitySystem::applyGravity(Board& board , int gemTypeCount) {
    bool moved = false;
    static std::mt19937 rng(std::time(nullptr));
    std::uniform_int_distribution<int> dist(1, gemTypeCount);

    for (int c = 0; c < BOARD_COLS; ++c) {
        std::vector<std::pair<int, Gem>> existingGems;

        // 1. 收集非空宝石
        for (int r = 0; r < BOARD_ROWS; ++r) {
            Gem g = board.getGem(r, c);
            if (g.type != GemType::Empty) {
                // 存入 {行号, 宝石}
                existingGems.push_back({ r, g });
            }
        }

        if (existingGems.size() < BOARD_ROWS) {
            moved = true;
            int emptySlots = BOARD_ROWS - existingGems.size();

            // 2. 填充顶部（新生成的宝石）
            for (int r = 0; r < emptySlots; ++r) {
                Gem newGem(static_cast<GemType>(dist(rng)));
                // 新生成的肯定是从天上下来的，必须 Falling
                newGem.state = GemState::Falling;
                board.setGem(r, c, newGem);
            }

            // 3. 填充底部（旧宝石）
            for (int i = 0; i < existingGems.size(); ++i) {
                // 取出原行号和宝石数据
                int originalRow = existingGems[i].first;
                Gem g = existingGems[i].second;

                // 计算新行号
                int newRow = i + emptySlots;

                // 只有当 新位置 != 旧位置 时，才标记为 Falling
                if (newRow != originalRow) {
                    g.state = GemState::Falling;
                }
                else {
                    // 如果位置没变（比如消除行下方的宝石），强制设为 Static
                    // 这样 UI 就不会去动它了
                    g.state = GemState::Static;
                }

                board.setGem(newRow, c, g);
            }
        }
    }
    return moved;
}