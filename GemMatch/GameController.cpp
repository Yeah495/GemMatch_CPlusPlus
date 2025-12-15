/*● GameController.h/cpp
  ○ 职责：监听 View 的点击信号，调用 Model 的函数。
  ○ 流程：
    ⅰ. 用户在 SceneGame 点击宝石 A 和 B。
    ⅱ. SceneGame 告诉 GameController：“用户想交换 (x1, y1) 和 (x2, y2)”。
    ⅲ. GameController 调用 GameMap::trySwap(...)。
    ⅳ. 如果成功，GameMap 返回消除数据，GameController 通知 SceneGame 播放动画并更新分数。*/



#include "GameController.h"
#include <QDebug>

GameController::GameController(MainWindow* view, QObject* parent)
    : QObject(parent), m_mainWindow(view), m_isProcessing(false)
{
    // 1. 初始化 Model
    m_gameCore = new GameCore();

    // 2. 获取 View 引用
    m_scene = m_mainWindow->getGameScene();

    // 3. 连接 View 的信号
    // 注意：SceneGame 里的宝石点击会触发此信号
    connect(m_scene, &SceneGame::gemClicked, this, &GameController::onGemClicked);

    // 初始化选中状态
    m_selectedPos = QPoint(-1, -1);
}

void GameController::startGame() {
    // Model 初始化数据
    m_gameCore->initGame();

    // View 渲染初始画面
    m_scene->renderBoard(m_gameCore->getBoard());

    // 重置状态
    m_selectedPos = QPoint(-1, -1);
    m_isProcessing = false;
}

void GameController::undo() {
    if (m_isProcessing) return; // 动画中禁止撤销

    if (m_gameCore->undo()) {
        // 撤销成功，重新渲染界面
        // 简单粗暴：直接重绘整个盘面，因为撤销不常发生
        m_scene->renderBoard(m_gameCore->getBoard());
        m_selectedPos = QPoint(-1, -1);
    }
}

void GameController::onGemClicked(int row, int col) {
    // 1. 守卫：如果正在处理动画，忽略所有点击
    if (m_isProcessing) return;

    QPoint currentClick(row, col);

    // 2. 情况A：当前没有选中任何宝石
    if (m_selectedPos.x() == -1) {
        m_selectedPos = currentClick;
        m_scene->setGemSelected(row, col, true); // 通知 View 高亮
        return;
    }

    // 3. 情况B：点击了同一个宝石 -> 取消选中
    if (m_selectedPos == currentClick) {
        m_scene->setGemSelected(m_selectedPos.x(), m_selectedPos.y(), false);
        m_selectedPos = QPoint(-1, -1);
        return;
    }

    // 4. 情况C：点击了相邻的宝石 -> 尝试交换
    if (abs(m_selectedPos.x() - row) + abs(m_selectedPos.y() - col) == 1) {
        // 取消高亮
        m_scene->setGemSelected(m_selectedPos.x(), m_selectedPos.y(), false);

        // 锁定输入
        m_isProcessing = true;

        // 执行交换逻辑
        attemptSwap(m_selectedPos, currentClick);

        // 重置选中
        m_selectedPos = QPoint(-1, -1);
    }
    // 5. 情况D：点击了不相邻的宝石 -> 切换选中目标
    else {
        m_scene->setGemSelected(m_selectedPos.x(), m_selectedPos.y(), false);
        m_selectedPos = currentClick;
        m_scene->setGemSelected(row, col, true);
    }
}

void GameController::attemptSwap(const QPoint& p1, const QPoint& p2) {
    // 【步骤 1】先让 View 播放交换动画（此时 Model 还没动）
    // 动画也是一种用户反馈，无论成不成功都要先动一下
    m_scene->animateSwap(p1.x(), p1.y(), p2.x(), p2.y(), [=]() {

        // --- 动画结束后的回调 (Callback) ---

        // 【步骤 2】调用 Model 进行逻辑判断
        SwapResult result = m_gameCore->trySwap(p1.x(), p1.y(), p2.x(), p2.y());

        if (result == SwapResult::Success) {
            // A. 交换成功且消除了
            // 此时 Model 里的数据已经是消除后的状态（GemType::Empty）
            // 我们需要找出哪些位置爆炸了，通知 View 播放爆炸

            // 为了简单，我们让 View 遍历一遍 Model 找出 Exploding 状态的宝石
            // 或者更优化的做法是 trySwap 返回消除列表

            // 这里演示通用做法：获取需要爆炸的坐标
            std::vector<QPoint> explodePoints;
            const Board& board = m_gameCore->getBoard();
            for (int r = 0; r < BOARD_ROWS; ++r) {
                for (int c = 0; c < BOARD_COLS; ++c) {
                    if (board.getGem(r, c).state == GemState::Exploding) {
                        explodePoints.push_back(QPoint(r, c));
                    }
                }
            }

            // 【步骤 3】播放消除动画
            m_scene->animateExplosion(explodePoints, [=]() {
                // 消除动画结束，开始处理下落
                processFallAndMatch();
                });

        }
        else {
            // B. 交换无效（没有形成三连）
            // 【步骤 3-Fail】播放回弹动画（交换回来）
            m_scene->animateSwap(p2.x(), p2.y(), p1.x(), p1.y(), [=]() {
                // 彻底结束，解锁输入
                m_isProcessing = false;
                });
        }
        });
}

void GameController::processFallAndMatch() {
    // 这是一个递归函数，用于处理 "下落 -> 消除 -> 下落 -> 消除" 的连锁反应

    // 【步骤 4】通知 Model 计算物理下落和新生成
    // processNextState 会执行 applyGravity，如果有新消除返回 true
    // 注意：Model 内部会将刚才的 Exploding 状态清理掉，并填充新宝石
    bool needMoreAnimation = m_gameCore->processNextState();

    // 获取最新的盘面数据传给 View
    // View 需要根据这些数据计算哪些是从上面掉下来的，哪些是新生成的
    m_scene->animateFall(m_gameCore->getBoard(), [=]() {

        // 【步骤 5】下落动画结束

        if (needMoreAnimation) {
            // 如果 Model 说还有消除发生（连击），我们需要再次播放爆炸
            // 此时 Model 里的某些宝石状态又是 Exploding 了

            std::vector<QPoint> explodePoints;
            const Board& board = m_gameCore->getBoard();
            for (int r = 0; r < BOARD_ROWS; ++r) {
                for (int c = 0; c < BOARD_COLS; ++c) {
                    if (board.getGem(r, c).state == GemState::Exploding) {
                        explodePoints.push_back(QPoint(r, c));
                    }
                }
            }

            if (!explodePoints.empty()) {
                // 播放连击的爆炸，然后递归调用自己
                m_scene->animateExplosion(explodePoints, [=]() {
                    processFallAndMatch();
                    });
            }
            else {
                // 理论上不会走到这，除非 processNextState 返回 true 但没标记爆炸
                m_isProcessing = false;
            }
        }
        else {
            // 没有更多消除了，回合完全结束
            m_isProcessing = false;
        }
        });
}