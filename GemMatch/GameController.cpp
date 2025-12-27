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
    connect(m_scene, &SceneGame::skillAll, this, &GameController::onSkillAll);
    connect(m_scene, &SceneGame::hintRequested, this, &GameController::onHintClicked);

    // 初始化选中状态
    m_selectedPos = QPoint(-1, -1);

    m_gameTimer = new QTimer(this);
    connect(m_gameTimer, &QTimer::timeout, this, &GameController::onGameTick);

    // 初始化音效
    m_soundClick = new QSoundEffect(this);
    m_soundClick->setSource(QUrl("qrc:/assets/sound/click.wav"));
    m_soundClick->setVolume(100.0f);

    m_soundMouth = new QSoundEffect(this);
    m_soundMouth->setSource(QUrl("qrc:/assets/sound/mouth.wav"));
    m_soundMouth->setVolume(1.0f);

    m_soundBoom = new QSoundEffect(this);
    m_soundBoom->setSource(QUrl("qrc:/assets/sound/boom.wav"));
    m_soundBoom->setVolume(0.05f);

    m_soundShuffle = new QSoundEffect(this);
    m_soundShuffle->setSource(QUrl("qrc:/assets/sound/shuffle.wav"));
    m_soundShuffle->setVolume(10.0f);

    m_soundIce = new QSoundEffect(this);
    m_soundIce->setSource(QUrl("qrc:/assets/sound/ice.wav"));
    m_soundIce->setVolume(1.0f);

    m_soundAll = new QSoundEffect(this);
    m_soundAll->setSource(QUrl("qrc:/assets/sound/all.wav"));
    m_soundAll->setVolume(1.0f);

    m_soundLaser = new QSoundEffect(this);
    m_soundLaser->setSource(QUrl("qrc:/assets/sound/laser.wav"));
    m_soundLaser->setVolume(1.0f);

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
        m_scene->setPauseButtonText("assets/images/暂停游戏.png");
    }
    else {
        // 暂停游戏
        m_isPaused = true;
        m_gameTimer->stop();
        m_scene->setPauseButtonText("assets/images/继续游戏.png");
    }
}

// 炸弹技能：随机炸掉 40 个宝石
void GameController::onSkillBomb() {
    if (m_isPaused || m_isProcessing) return;

    m_remainBomb--;
    updateSkillButtons();
    m_scene->stopHintAnimation();

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

    // 2. 如果盘面上宝石少于 40 个（极端情况），就炸掉剩下的所有
    int bombCount = std::min((int)candidates.size(), 40);

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
    m_soundBoom->play(); // 炸弹音效

    // 6. 播放动画并进入下落流程
    m_scene->animateExplosion(targets, [=]() {
        // 增加一点分数作为奖励
        m_gameCore->addScoreSession(1000);
        m_scene->updateScore(m_gameCore->getScore());

        processFallAndMatch(); // 复用消除后的下落逻辑
        });
}

// 洗牌技能：重新打乱当前盘面
void GameController::onSkillShuffle() {
    if (m_isPaused || m_isProcessing) return;

    m_remainShuffle--;
    updateSkillButtons();
    m_scene->stopHintAnimation();

    m_gameCore->shuffleBoard();
    m_scene->renderBoard(m_gameCore->getBoard());

    int comboSize = 0;
    // 检测并标记 Exploding 状态
    if (m_gameCore->findAndMarkMatches(&comboSize)) {

        m_isProcessing = true; // 锁定输入，因为要开始播动画了

        // 播放音效
        m_soundShuffle->play(); // 洗牌音效
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

    m_soundIce->play(); // 冻结音效
    m_scene->stopHintAnimation();

    m_isTimeFrozen = true;
    m_freezeCounter = 5; // 冻结 5 个 Tick

    // 可选：给 UI 一个视觉反馈，比如时间变蓝
    m_scene->updateTime(m_remainingTime, true);
}

//万能
void GameController::onSkillAll() {
    // 1. 检查条件
    if (m_isPaused || m_isProcessing || m_remainAll <= 0) return;

    m_remainAll--;
    updateSkillButtons();
    m_scene->stopHintAnimation();

    // 2. 随机找一个非空的普通位置变成万能宝石
    // 更好的体验是：把它变在棋盘中央，或者变在玩家选中的位置（如果选了的话）
    int r, c;
    do {
        r = rand() % BOARD_ROWS;
        c = rand() % BOARD_COLS;
    } while (m_gameCore->getBoard().getGem(r, c).type == GemType::Empty);

    // 3. 修改 Model
    Gem g = m_gameCore->getBoard().getGem(r, c);
    g.type = GemType::Universal; 
    g.state = GemState::Static;
    m_gameCore->getBoardPtr()->setGem(r, c, g);

    // 4. 播放特效音效
    // m_soundMagic->play(); 

    // 5. 刷新界面----paint宝石图片调用的时机,会根据宝石的类型以及坐标对应绘图显示宝石(addItm)
    m_scene->renderBoard(m_gameCore->getBoard());
}

void GameController::updateSkillButtons() {
    // 调用 View 的接口刷新界面
    m_scene->updateSkillButtonText(m_remainBomb, m_remainShuffle, m_remainTime, m_remainAll);
}

void GameController::onHintClicked() {
    if (m_isPaused || m_isProcessing) return;

    // 1. 问 Model 要提示
    std::vector<QPoint> hint = m_gameCore->findHint();

    if (!hint.empty()) {
        // 2. 找到了，告诉 View 播放动画
        // 扣除一点分数作为惩罚（可选）
        // m_gameCore->addScoreSession(-50); 
        // m_scene->updateScore(m_gameCore->getScore());

        m_scene->showHintAnimation(hint[0], hint[1]);
    }
    else {
        // 3. 没找到（说明是死局，通常应该自动洗牌，或者弹窗提示）
        //qDebug() << "没有可移动的步数！";
        QMessageBox::information(m_mainWindow, "提示", "当前没有可消除的宝石，建议使用洗牌技能！");
    }
}

void GameController::startGame(int difficultyLevel) {
    m_isTerminated = false;
    // 记录当前难度
    m_currentDifficulty = difficultyLevel;

    // 【新增】确保清理掉上一次的破纪录图片
    m_scene->hideRecordAnimation();

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
    m_remainAll = MAX_ALL_COUNT;
    updateSkillButtons();

    m_remainingTime = GAME_DURATION;
    m_scene->updateTime(m_remainingTime, false); // 初始调用
    m_gameTimer->start(1000); // 每 1000ms (1秒) 触发一次
    m_isTimeFrozen = false;
    m_isPaused = false;
    m_scene->setPauseButtonText("assets/images/暂停游戏.png");
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
        int recordType = 0; // 0=无纪录, 1=个人, 2=全服

        // 1. 更新数据库并获取纪录状态
        if (!UserManager::instance().getCurrentUser().isEmpty()) {
            // 注意：这里假设您已经修改了 UserManager::updateScore 让它返回 int
            // 如果您的 updateScore 还是 void，这里会报错，请先修改 UserManager
            recordType = UserManager::instance().updateScore(finalScore, m_currentDifficulty);

            qDebug() << "游戏结束 - 难度:" << m_currentDifficulty
                << " 分数:" << finalScore
                << " 纪录状态(recordType):" << recordType;
        }

        // 定义结束流程：延迟显示弹窗
        auto showGameOverDialog = [this, finalScore]() {
            emit gameOver(finalScore);
            endGame();
            };

        // 2. 判断是否播放动画
        if (recordType > 0) {
            qDebug() << "触发破纪录动画！类型：" << recordType;

            // 调用 SceneGame 的动画接口
            // 请确保 SceneGame::playNewRecordAnimation 已经按之前的建议修改好
            m_scene->playNewRecordAnimation(recordType, [this, showGameOverDialog]() {
                // 动画播放完毕（图片落地）后，再等 3 秒，然后弹窗
                QTimer::singleShot(3000, this, [showGameOverDialog]() {
                    showGameOverDialog();
                    });
                });
        }
        else {
            // 没有破纪录，直接弹窗（稍作延迟体验更好）
            qDebug() << "未破纪录，直接结算";
            QTimer::singleShot(500, this, showGameOverDialog);
        }
        //endGame();
        //// ✅ 修改：发出 gameOver 信号，而不是直接显示 MessageBox
        //emit gameOver(finalScore);

        
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
    m_isPaused = true;
    m_isTerminated = true;
    m_gameTimer->stop();
    stopAllSounds();

    if (m_scene) {
        m_scene->stopHintAnimation();
    }
}

void GameController::onGemClicked(int row, int col) {
    // 1. 守卫：如果正在处理动画，忽略所有点击
    if (m_isPaused || m_isProcessing) return;

    m_soundClick->play();
    m_scene->stopHintAnimation();

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
        if (m_isTerminated) return;
        // --- 动画结束后的回调 (Callback) ---

        //判断是否为万能宝石交换
        const Board& board = m_gameCore->getBoard();
        GemType type1 = board.getGem(p1.x(), p1.y()).type;
        GemType type2 = board.getGem(p2.x(), p2.y()).type;

        // ============================================================
        // 【新增逻辑】万能宝石特殊处理 (Magic Gem Logic)
        // ============================================================
        bool isUniversal1 = (type1 == GemType::Universal);
        bool isUniversal2 = (type2 == GemType::Universal);

        // 只要其中一个是万能宝石，就触发特殊消除
        if (isUniversal1 || isUniversal2) {

            // 1. 确定要消除的目标颜色
            // 如果两个都是万能宝石(双彩虹)，逻辑比较特殊，这里暂时默认消除其中一方的颜色
            // 如果 p1 是万能，那么目标颜色就是 p2 的颜色，反之亦然
            GemType targetColor = isUniversal1 ? type2 : type1;

            // 2. 如果目标也是万能宝石（即两个万能互换），或者目标是空的，则不处理或全屏清除
            // 这里简单处理：如果是两个万能互换，我们假设消除所有红色（或者你可以写一个 explodeAllBoard）
            if (targetColor == GemType::Universal) targetColor = GemType::Red; // 兜底逻辑
            if (targetColor == GemType::Empty) return; // 交换了个寂寞

            // 3. 在数据层执行交换
            // 我们必须先让它们在数据上换位置，这样万能宝石才会跑到目标位置去爆炸
            m_gameCore->getBoardPtr()->swapGem(p1.x(), p1.y(), p2.x(), p2.y());

            // 4. 调用核心算法：消除所有该颜色的宝石
            // 注意：explodeAllColor 需要你在 GameCore 中实现
            int count = m_gameCore->explodeAllColor(targetColor);

            // 5. 播放音效与加分
			m_soundAll->play(); // 万能宝石消除音效
            m_comboLevel = 1;
            m_gameCore->addScoreSession(count * 10);
            m_scene->updateScore(m_gameCore->getScore());

            // 6. 搜集所有状态为 Exploding 的点（用于播放动画）
            std::vector<QPoint> explodePoints;
            const Board& currentBoard = m_gameCore->getBoard(); // 重新获取最新的引用
            for (int r = 0; r < BOARD_ROWS; ++r) {
                for (int c = 0; c < BOARD_COLS; ++c) {
                    if (currentBoard.getGem(r, c).state == GemState::Exploding) {
                        explodePoints.push_back(QPoint(r, c));
                    }
                }
            }

            // 7. 播放消除动画 -> 并在结束后进入下落流程
            m_scene->animateExplosion(explodePoints, [=]() {
                processFallAndMatch();
                });

            // 【非常重要】直接 return，不执行下面的常规三连检测逻辑
            return;
        }

        // 【步骤 2】调用 Model 进行逻辑判断
        SwapResult result = m_gameCore->trySwap(p1.x(), p1.y(), p2.x(), p2.y());

        if (result == SwapResult::Success) {
            // 播放消除音效
            if (m_gameCore->isSpecialMatch()) {
                m_soundLaser->play();
            }
            else {
                m_soundMouth->play(); 
            }

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
    if (m_isTerminated) return;
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
            if (m_gameCore->isSpecialMatch()) {
                m_soundLaser->play();
            }
            else {
                m_soundMouth->play();
            }

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

void GameController::stopAllSounds() {
    if (m_soundClick) m_soundClick->stop();
    if (m_soundMouth) m_soundMouth->stop();
    if (m_soundBoom) m_soundBoom->stop();
    if (m_soundShuffle) m_soundShuffle->stop();
    if (m_soundIce) m_soundIce->stop();
    if (m_soundAll) m_soundAll->stop();
    if (m_soundLaser) m_soundLaser->stop();
}


