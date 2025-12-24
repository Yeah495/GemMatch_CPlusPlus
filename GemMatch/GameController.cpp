#include "GameController.h"
#include <QDebug>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include "UserManager.h"

GameController::GameController(MainWindow* view, QObject* parent)
    : QObject(parent), m_mainWindow(view), m_isProcessing(false)
{
    // 1. 初始化 Model
    m_gameCore = new GameCore();

    // 2. 获取 View 引用
    m_scene = m_mainWindow->getGamePage();

    // 3. 连接 View 的信号
    connect(m_scene, &SceneGame::gemClicked, this, &GameController::onGemClicked);

    // 连接道具点击和暂停游戏的信号
    connect(m_scene, &SceneGame::pauseGame, this, &GameController::onPauseClicked);
    connect(m_scene, &SceneGame::skillBomb, this, &GameController::onSkillBomb);
    connect(m_scene, &SceneGame::skillShuffle, this, &GameController::onSkillShuffle);
    connect(m_scene, &SceneGame::skillTime, this, &GameController::onSkillTime);

    // 初始化选中状态
    m_selectedPos = QPoint(-1, -1);

    m_gameTimer = new QTimer(this);
    connect(m_gameTimer, &QTimer::timeout, this, &GameController::onGameTick);

    // 初始化音效
    m_soundClick = new QSoundEffect(this);
    m_soundClick->setSource(QUrl("qrc:/assets/sound/click.wav"));
    m_soundClick->setVolume(100.0f);

    m_soundClear = new QSoundEffect(this);
    m_soundClear->setSource(QUrl("qrc:/assets/sound/mouth.wav"));
    m_soundClear->setVolume(1.0f);

    // 初始化难度
    m_currentDifficulty = 3; // 默认简单难度
}

// 获取难度名称
QString GameController::getDifficultyName(int difficulty) {
    switch (difficulty) {
    case 3: return "简单";
    case 5: return "普通";
    case 7: return "困难";
    default: return "未知";
    }
}

// 1. 暂停功能
void GameController::onPauseClicked() {
    if (m_isPaused) {
        // 恢复游戏
        m_isPaused = false;
        m_gameTimer->start();
        m_scene->setPauseButtonText("暂停游戏");
    }
    else {
        // 暂停游戏
        m_isPaused = true;
        m_gameTimer->stop();
        m_scene->setPauseButtonText("继续游戏");
    }
}

// 炸弹技能：随机炸掉 5 个宝石
void GameController::onSkillBomb() {
    if (m_isPaused || m_isProcessing) return;

    m_remainBomb--;
    updateSkillButtons();

    m_isProcessing = true; // 锁定输入

    // 1. 收集所有【非空】的宝石坐标作为候选名单
    std::vector<QPoint> candidates;
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            if (m_gameCore->getBoard().getGem(r, c).type != GemType::Empty) {
                candidates.push_back(QPoint(r, c));
            }
        }
    }

    // 2. 如果盘面上宝石少于 5 个（极端情况），就炸掉剩下的所有
    int bombCount = std::min((int)candidates.size(), 5);

    // 3. 使用标准库进行乱序（洗牌候选名单）
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(candidates.begin(), candidates.end(), std::default_random_engine(seed));

    // 4. 取前 5 个作为目标
    std::vector<QPoint> targets;
    for (int i = 0; i < bombCount; ++i) {
        targets.push_back(candidates[i]);
    }

    // 5. 修改 Model 状态
    for (const auto& p : targets) {
        Gem g = m_gameCore->getBoard().getGem(p.x(), p.y());
        g.state = GemState::Exploding;
        m_gameCore->getBoardPtr()->setGem(p.x(), p.y(), g);
    }

    m_scene->startShakeAnimation();
    m_soundClear->play(); // 炸弹音效

    // 6. 播放动画并进入下落流程
    m_scene->animateExplosion(targets, [=]() {
        // 增加一点分数作为奖励
        m_gameCore->addScoreSession(100);
        m_scene->updateScore(m_gameCore->getScore());

        processFallAndMatch(); // 复用消除后的下落逻辑
        });
}

// 洗牌技能：重新打乱当前盘面
void GameController::onSkillShuffle() {
    if (m_isPaused || m_isProcessing) return;

    m_remainShuffle--;
    updateSkillButtons();

    m_gameCore->shuffleBoard();
    m_scene->renderBoard(m_gameCore->getBoard());

    int comboSize = 0;
    // 检测并标记 Exploding 状态
    if (m_gameCore->findAndMarkMatches(&comboSize)) {

        m_isProcessing = true; // 锁定输入，因为要开始播动画了

        // 播放音效
        m_soundClear->play();
        m_comboLevel = 1;

        // 加分
        m_gameCore->addScoreSession(comboSize * 10);
        m_scene->updateScore(m_gameCore->getScore());

        // 收集爆炸点 (因为 renderBoard 时是 Static，现在 Model 已经是 Exploding 了)
        std::vector<QPoint> explodePoints;
        const Board& board = m_gameCore->getBoard();
        for (int r = 0; r < BOARD_ROWS; ++r) {
            for (int c = 0; c < BOARD_COLS; ++c) {
                if (board.getGem(r, c).state == GemState::Exploding) {
                    explodePoints.push_back(QPoint(r, c));
                }
            }
        }

        // 4. 触发标准的消除->下落流程
        // 这里的逻辑和 attemptSwap 成功后是一模一样的
        m_scene->animateExplosion(explodePoints, [=]() {
            processFallAndMatch();
            });
    }
}

// 时间冻结：冻结时间 5 秒
void GameController::onSkillTime() {
    if (m_isPaused || m_isTimeFrozen) return; // 避免重复使用

    m_remainTime--;
    updateSkillButtons();

    m_isTimeFrozen = true;
    m_freezeCounter = 5; // 冻结 5 个 Tick

    // 可选：给 UI 一个视觉反馈，比如时间变蓝
    m_scene->updateTime(m_remainingTime, true);
}

void GameController::updateSkillButtons() {
    // 调用 View 的接口刷新界面
    m_scene->updateSkillButtonText(m_remainBomb, m_remainShuffle, m_remainTime);
}

void GameController::startGame(int difficultyLevel) {
    // 记录当前难度
    m_currentDifficulty = difficultyLevel;

    // Model 初始化数据
    m_gameCore->initGame(difficultyLevel);

    // View 渲染初始画面
    m_scene->renderBoard(m_gameCore->getBoard());

    // 重置状态
    m_selectedPos = QPoint(-1, -1);
    m_isProcessing = false;
    m_comboLevel = 1;

    m_scene->updateScore(0); // 初始分数 0

    m_remainBomb = MAX_BOMB_COUNT;
    m_remainShuffle = MAX_SHUFFLE_COUNT;
    m_remainTime = MAX_TIME_COUNT;
    updateSkillButtons();

    m_remainingTime = GAME_DURATION;
    m_scene->updateTime(m_remainingTime, false); // 初始调用
    m_gameTimer->start(1000); // 每 1000ms (1秒) 触发一次
    m_isTimeFrozen = false;
    m_isPaused = false;
    m_scene->setPauseButtonText("暂停游戏");
}

void GameController::onGameTick() {
    if (m_isTimeFrozen) {
        m_freezeCounter--;
        if (m_freezeCounter <= 0) {
            m_isTimeFrozen = false;
        }
        m_scene->updateTime(m_remainingTime, true);
        return;
    }

    m_remainingTime--;
    m_scene->updateTime(m_remainingTime, false);

    if (m_remainingTime <= 0) {
        m_gameTimer->stop();
        m_isProcessing = true;

        // 获取最终得分
        int finalScore = m_gameCore->getScore();

        // 更新数据库中的用户分数（根据难度）
        if (!UserManager::instance().getCurrentUser().isEmpty()) {
            QString currentUser = UserManager::instance().getCurrentUser();
            qDebug() << "游戏结束，用户：" << currentUser
                << "，难度：" << m_currentDifficulty
                << "，得分：" << finalScore;

            UserManager::instance().updateScore(finalScore, m_currentDifficulty);

            // 立即验证分数是否保存成功
            int savedScore = UserManager::instance().getCurrentUserHighScore(m_currentDifficulty);
            qDebug() << "数据库保存验证 - 难度" << m_currentDifficulty
                << "的最高分：" << savedScore;
        }
        else {
            qDebug() << "游戏结束，但用户未登录，分数不保存";
        }

        // 弹出游戏结束提示
        QString difficultyName = getDifficultyName(m_currentDifficulty);
        QString msg = QString("时间到！\n\n游戏难度: %1\n本局最终得分: %2")
            .arg(difficultyName)
            .arg(finalScore);

        // 使用非阻塞的方式显示消息框
        QTimer::singleShot(100, [this, msg]() {
            QMessageBox::information(m_mainWindow, "游戏结束", msg);
            });

        // 延迟切换页面，确保消息框显示
        QTimer::singleShot(500, [this]() {
            m_mainWindow->switchPage(1);
            });
    }
}

void GameController::undo() {
    if (m_isProcessing) return; // 动画中禁止撤销

    if (m_gameCore->undo()) {
        // 撤销成功，重新渲染界面
        m_scene->renderBoard(m_gameCore->getBoard());
        m_selectedPos = QPoint(-1, -1);
    }
}

void GameController::endGame() {
    m_gameTimer->stop();
}

void GameController::onGemClicked(int row, int col) {
    // 1. 守卫：如果正在处理动画，忽略所有点击
    if (m_isPaused || m_isProcessing) return;

    m_soundClick->play();

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
    m_scene->animateSwap(p1.x(), p1.y(), p2.x(), p2.y(), [=]() {
        // --- 动画结束后的回调 (Callback) ---

        // 【步骤 2】调用 Model 进行逻辑判断
        SwapResult result = m_gameCore->trySwap(p1.x(), p1.y(), p2.x(), p2.y());

        if (result == SwapResult::Success) {
            // 播放消除音效
            m_soundClear->play();

            // A. 交换成功且消除了
            m_comboLevel = 1;  // 初始化为1连击
            m_scene->updateScore(m_gameCore->getScore());

            // 获取需要爆炸的坐标
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
            // 这里也可以加一个小音效提示交换失败

            // 【步骤 3-Fail】播放回弹动画（交换回来）
            m_scene->animateSwap(p2.x(), p2.y(), p1.x(), p1.y(), [=]() {
                // 彻底结束，解锁输入
                m_isProcessing = false;
                });
        }
        });
}

void GameController::processFallAndMatch() {
    // 1. 【清理】把上一轮炸掉的宝石在数据层变为空
    m_gameCore->clearMatches();

    // 2. 【填充】执行下落和生成新宝石
    m_gameCore->applyGravityOnly(m_currentDifficulty); // 使用当前难度

    // 3. 【动画】播放下落动画
    m_scene->animateFall(m_gameCore->getBoard(), [=]() {
        m_gameCore->resetGemStates(); // 全部重置为Static状态

        // 4. 【检测】扫描新盘面
        int comboSize = 0;
        bool hasNewMatches = m_gameCore->findAndMarkMatches(&comboSize);

        if (hasNewMatches) {
            // 连击消除音效
            m_soundClear->play();

            m_comboLevel++;  // 连击层数增加
            int score = comboSize * 10 * m_comboLevel;  // 连击翻倍
            m_gameCore->addScoreSession(score);
            m_scene->updateScore(m_gameCore->getScore());

            // 5. 【准备数据】搜集新的爆炸点
            std::vector<QPoint> explodePoints;
            const Board& board = m_gameCore->getBoard();
            for (int r = 0; r < BOARD_ROWS; ++r) {
                for (int c = 0; c < BOARD_COLS; ++c) {
                    if (board.getGem(r, c).state == GemState::Exploding) {
                        explodePoints.push_back(QPoint(r, c));
                    }
                }
            }

            // 6. 【动画】播放新一轮爆炸，炸完再递归
            m_scene->animateExplosion(explodePoints, [=]() {
                processFallAndMatch(); // 递归进入下一轮
                });
        }
        else {
            // 没有新连击了，彻底稳了
            m_isProcessing = false;
        }
        });
}