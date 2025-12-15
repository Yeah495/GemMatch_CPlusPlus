/*  a. 技术建议：使用 QGraphicsView 和 QGraphicsScene 框架。更适合做游戏，能支持平滑的宝石交换动画。
  b. 动画逻辑：当 Model 层发生交换时，View 层负责播放 0.3 秒的移动动画，动画结束再更新数据。*/




#include "SceneGame.h"
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QTimer>

SceneGame::SceneGame(QObject* parent) : QGraphicsScene(parent) {
    // 设置场景大小 (假设窗口 800x600)
    setSceneRect(0, 0, 800, 600);
    // 初始化指针数组
    for (int i = 0; i < BOARD_ROWS; ++i)
        for (int j = 0; j < BOARD_COLS; ++j)
            m_items[i][j] = nullptr;
}

QPointF SceneGame::getScreenPos(int row, int col) const {
    return QPointF(MARGIN_LEFT + col * CELL_SIZE, MARGIN_TOP + row * CELL_SIZE);
}

void SceneGame::renderBoard(const Board& board) {
    // 清理旧图元
    clear();
    // 注意：clear() 会删除所有 items，所以 m_items 指针需要重置
    // 但为了背景图等不被误删，实际工程中通常只删除 GemItem 或使用图元池
    // 这里为简化直接 clear 并重新添加背景

    // 添加背景 (简单示例)
    // addPixmap(ResourceLoader::instance().getBackground());

    // 生成新宝石
    for (int i = 0; i < BOARD_ROWS; ++i) {
        for (int j = 0; j < BOARD_COLS; ++j) {
            Gem g = board.getGem(i, j);
            GemItem* item = new GemItem(i, j, g.type);
            item->setPos(getScreenPos(i, j));
            addItem(item);
            m_items[i][j] = item;

            // 连接点击信号
            connect(item, &GemItem::clicked, this, &SceneGame::gemClicked);
        }
    }
}

void SceneGame::setGemSelected(int r, int c, bool selected) {
    if (m_items[r][c]) {
        m_items[r][c]->setSelected(selected);
    }
}

void SceneGame::animateSwap(int r1, int c1, int r2, int c2, std::function<void()> finishedCallback) {
    GemItem* item1 = m_items[r1][c1];
    GemItem* item2 = m_items[r2][c2];

    if (!item1 || !item2) {
        if (finishedCallback) finishedCallback();
        return;
    }

    // 交换指针，保证 grid 逻辑位置正确
    std::swap(m_items[r1][c1], m_items[r2][c2]);
    // 更新内部坐标记录
    item1->setGridPos(r2, c2);
    item2->setGridPos(r1, c1);

    // 创建动画组
    QParallelAnimationGroup* group = new QParallelAnimationGroup;

    QPropertyAnimation* anim1 = new QPropertyAnimation(item1, "pos");
    anim1->setDuration(300); // 300ms
    anim1->setStartValue(item1->pos());
    anim1->setEndValue(getScreenPos(r2, c2));

    QPropertyAnimation* anim2 = new QPropertyAnimation(item2, "pos");
    anim2->setDuration(300);
    anim2->setStartValue(item2->pos());
    anim2->setEndValue(getScreenPos(r1, c1));

    group->addAnimation(anim1);
    group->addAnimation(anim2);

    // 动画结束后触发回调
    // 注意：lambda 捕获 group 以便自动删除
    connect(group, &QAbstractAnimation::finished, this, [group, finishedCallback]() {
        if (finishedCallback) finishedCallback();
        group->deleteLater();
        });

    group->start();
}

void SceneGame::animateExplosion(const std::vector<QPoint>& points, std::function<void()> finishedCallback) {
    QParallelAnimationGroup* group = new QParallelAnimationGroup;

    for (const auto& p : points) {
        GemItem* item = m_items[p.x()][p.y()];
        if (item) {
            // 简单的淡出动画
            QPropertyAnimation* anim = new QPropertyAnimation(item, "opacity");
            anim->setDuration(200);
            anim->setEndValue(0.0);
            group->addAnimation(anim);
        }
    }

    connect(group, &QAbstractAnimation::finished, this, [this, points, group, finishedCallback]() {
        // 动画结束，物理上移除这些图元
        for (const auto& p : points) {
            GemItem* item = m_items[p.x()][p.y()];
            if (item) {
                removeItem(item);
                delete item;
                m_items[p.x()][p.y()] = nullptr; // 标记为空
            }
        }
        if (finishedCallback) finishedCallback();
        group->deleteLater();
        });

    group->start();
}

void SceneGame::animateFall(const Board& newBoard, std::function<void()> finishedCallback) {
    // 下落动画比较复杂，策略是：
    // 1. 读取 newBoard，找出现在的 (r,c) 和旧画面的差异
    // 2. 创建缺失的 GemItem (新生成的) 放在顶部外
    // 3. 计算所有宝石的目标位置，执行移动动画

    QParallelAnimationGroup* group = new QParallelAnimationGroup;
    bool anyMove = false;

    for (int c = 0; c < BOARD_COLS; ++c) {
        // 从底部向上遍历，容易处理
        for (int r = BOARD_ROWS - 1; r >= 0; --r) {
            Gem newGemData = newBoard.getGem(r, c);

            // 检查当前位置的 item 是否匹配
            GemItem* currentItem = m_items[r][c];

            // 如果该位置为空，或者现有的宝石类型不对（说明原来的宝石被消除了，或者是新掉下来的）
            // 在简单的实现中，我们可以重新创建所有图元并让它们飞过来
            // 但为了平滑，我们应该尽量复用图元。
            // 简化策略：
            // 实际上，animateExplosion 后 m_items 里有些是 nullptr。
            // 我们需要重新填充这些 nullptr。

            // 这里的完整逻辑较长，简化为：
            // 重新根据 Board 创建所有 Item，如果 Item 之前在上面，就做下落动画
            // 如果 Item 是新生成的（state == Falling），就从屏幕上方掉下来
        }
    }

    // --- 简化版实现 ---
    // 为演示架构，我们用一种简单的暴力重绘+动画方法：
    // 清空当前 items，根据 Board 重建所有 items
    // 如果是 Falling 状态，起始位置设在上方，目标位置设在格子

    // 注意：这种方法会丢失“哪个宝石是从哪掉下来”的视觉连贯性
    // 完美的实现需要 Model 层传递“移动路径”。

    // 这里我们仅演示如何根据 Model 状态播放入场：
    clear();
    for (int i = 0; i < BOARD_ROWS; ++i) {
        for (int j = 0; j < BOARD_COLS; ++j) {
            Gem g = newBoard.getGem(i, j);
            if (g.type == GemType::Empty) continue;

            GemItem* item = new GemItem(i, j, g.type);
            m_items[i][j] = item;
            addItem(item);
            connect(item, &GemItem::clicked, this, &SceneGame::gemClicked);

            QPointF targetPos = getScreenPos(i, j);

            if (g.state == GemState::Falling) {
                // 新生成的/下落的，从上方飞入
                QPointF startPos = targetPos - QPointF(0, 200); // 偏移200像素
                item->setPos(startPos);

                QPropertyAnimation* anim = new QPropertyAnimation(item, "pos");
                anim->setDuration(400);
                anim->setStartValue(startPos);
                anim->setEndValue(targetPos);
                anim->setEasingCurve(QEasingCurve::OutBounce); // 弹跳效果
                group->addAnimation(anim);
                anyMove = true;
            }
            else {
                item->setPos(targetPos);
            }
        }
    }

    if (anyMove) {
        connect(group, &QAbstractAnimation::finished, this, [group, finishedCallback]() {
            if (finishedCallback) finishedCallback();
            group->deleteLater();
            });
        group->start();
    }
    else {
        delete group;
        if (finishedCallback) finishedCallback();
    }
}