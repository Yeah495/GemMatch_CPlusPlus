#include "GameController.h"
#include <QDebug>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include "UserManager.h"
#include "SpeechManager.h" 

GameController::GameController(MainWindow* view, QObject* parent)
    : QObject(parent), m_mainWindow(view), m_isProcessing(false)
{
    //初始化 Model
    m_gameCore = new GameCore();

    //获取 View 引用
    m_scene = m_mainWindow->getGamePage();

    //连接点击信号
    connect(m_scene, &SceneGame::gemClicked, this, &GameController::onGemClicked);

    //连接按钮信号
    connect(m_scene, &SceneGame::pauseGame, this, &GameController::onPauseClicked);
    connect(m_scene, &SceneGame::skillBomb, this, &GameController::onSkillBomb);
    connect(m_scene, &SceneGame::skillShuffle, this, &GameController::onSkillShuffle);
    connect(m_scene, &SceneGame::skillTime, this, &GameController::onSkillTime);
    connect(m_scene, &SceneGame::skillAll, this, &GameController::onSkillAll);
    connect(m_scene, &SceneGame::hintRequested, this, &GameController::onHintClicked);

    //初始化选中状态
    m_selectedPos = QPoint(-1, -1);

    //初始化计时器
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
    m_currentDifficulty = 3;
}

//获取难度名称
QString GameController::getDifficultyName(int difficulty) {
    switch (difficulty) {
    case 3: return "简单";
    case 5: return "普通";
    case 7: return "困难";
    default: return "未知";
    }
}

//暂停功能
void GameController::onPauseClicked() {
    if (m_isProcessing) {
		return; // 正在处理动画时不允许暂停
    }
    if (m_isPaused) {
        //恢复游戏
        m_isPaused = false;
        m_gameTimer->start();
        m_scene->setPauseButtonText("assets/images/暂停游戏.png");
    }
    else {
        //暂停游戏
        m_isPaused = true;
        m_gameTimer->stop();
        m_scene->setPauseButtonText("assets/images/继续游戏.png");
    }
}

//炸弹技能：随机炸掉 40 个宝石
void GameController::onSkillBomb() {
    if (m_isPaused || m_isProcessing) return;

    m_remainBomb--;
    updateSkillButtons();
    m_scene->stopHintAnimation();

    m_isProcessing = true; //设置正在进行动画处理

    //收集所有非空的宝石坐标作为候选名单
    std::vector<QPoint> candidates;
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            if (m_gameCore->getBoard().getGem(r, c).type != GemType::Empty) {
                candidates.push_back(QPoint(r, c));
            }
        }
    }
    int bombCount = std::min((int)candidates.size(), 40);

    //打乱顺序
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(candidates.begin(), candidates.end(), std::default_random_engine(seed));

    //取前40个作为目标
    std::vector<QPoint> targets;
    for (int i = 0; i < bombCount; ++i) {
        targets.push_back(candidates[i]);
    }

    //修改 Model 状态
    for (const auto& p : targets) {
        Gem g = m_gameCore->getBoard().getGem(p.x(), p.y());
        g.state = GemState::Exploding;
        m_gameCore->getBoardPtr()->setGem(p.x(), p.y(), g);
    }

    m_scene->startShakeAnimation();  //震动
    m_soundBoom->play(); // 炸弹音效

    //播放动画并进入下落流程
    m_scene->animateExplosion(targets, [=]() {
        //增加分数
        m_gameCore->addScoreSession(1000);
        m_scene->updateScore(m_gameCore->getScore());

        processFallAndMatch();//递归下落
        });
}

//洗牌技能：重新打乱当前盘面
void GameController::onSkillShuffle() {
    if (m_isPaused || m_isProcessing) return;

    m_remainShuffle--;
    updateSkillButtons();
    m_scene->stopHintAnimation();

    m_gameCore->shuffleBoard();
    m_scene->renderBoard(m_gameCore->getBoard());

    int comboSize = 0;
    //检测并标记 Exploding 状态
    if (m_gameCore->findAndMarkMatches(&comboSize)) {

        m_isProcessing = true; //锁定输入，因为要开始播动画了

        m_soundShuffle->play(); // 洗牌音效
        m_comboLevel = 1;

        //加分
        m_gameCore->addScoreSession(comboSize * 10);
        m_scene->updateScore(m_gameCore->getScore());

        //收集爆炸点
        std::vector<QPoint> explodePoints;
        const Board& board = m_gameCore->getBoard();
        for (int r = 0; r < BOARD_ROWS; ++r) {
            for (int c = 0; c < BOARD_COLS; ++c) {
                if (board.getGem(r, c).state == GemState::Exploding) {
                    explodePoints.push_back(QPoint(r, c));
                }
            }
        }

        //消除下落
        m_scene->animateExplosion(explodePoints, [=]() {
            processFallAndMatch();
            });
    }
}

//时间冻结：冻结时间 5 秒
void GameController::onSkillTime() {
    if (m_isPaused || m_isTimeFrozen) return; // 避免重复使用

    m_remainTime--;
    updateSkillButtons();

    m_soundIce->play(); // 冻结音效
    m_scene->stopHintAnimation();

    m_isTimeFrozen = true;
    m_freezeCounter = 5;

    //时间变蓝
    m_scene->updateTime(m_remainingTime, true);
}

//万能
void GameController::onSkillAll() {
    if (m_isPaused || m_isProcessing || m_remainAll <= 0) return;

    m_remainAll--;
    updateSkillButtons();
    m_scene->stopHintAnimation();

    // 随机找一个非空的普通位置变成万能宝石
    int r, c;
    do {
        r = rand() % BOARD_ROWS;
        c = rand() % BOARD_COLS;
    } while (m_gameCore->getBoard().getGem(r, c).type == GemType::Empty);

    //修改 Model
    Gem g = m_gameCore->getBoard().getGem(r, c);
    g.type = GemType::Universal; 
    g.state = GemState::Static;
    m_gameCore->getBoardPtr()->setGem(r, c, g);

    // 刷新界面
    m_scene->renderBoard(m_gameCore->getBoard());
}

void GameController::updateSkillButtons() {
    m_scene->updateSkillButtonText(m_remainBomb, m_remainShuffle, m_remainTime, m_remainAll);
}

void GameController::onHintClicked() {
    if (m_isPaused || m_isProcessing) return;

    //问 Model 要提示
    std::vector<QPoint> hint = m_gameCore->findHint();

    if (!hint.empty()) {
        m_scene->showHintAnimation(hint[0], hint[1]);
    }
    else {
        QMessageBox::information(m_mainWindow, "提示", "当前没有可消除的宝石，建议使用洗牌技能！");
    }
}

void GameController::startGame(int difficultyLevel) {
    m_isTerminated = false;
    //记录当前难度
    m_currentDifficulty = difficultyLevel;

    //清理掉上一次的破纪录图片
    m_scene->hideRecordAnimation();

    //Model 初始化数据
    m_gameCore->initGame(difficultyLevel);

    //View 渲染初始画面
    m_scene->renderBoard(m_gameCore->getBoard());

    //重置状态
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
    m_gameTimer->start(1000);
    m_isTimeFrozen = false;
    m_isPaused = false;
    m_scene->setPauseButtonEnabled(true);
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

        //获取最终得分
        int finalScore = m_gameCore->getScore();
        int recordType = 0; //0=无纪录, 1=个人, 2=全服
        endGame();

        //更新数据库并获取纪录状态
        if (!UserManager::instance().getCurrentUser().isEmpty()) {
            recordType = UserManager::instance().updateScore(finalScore, m_currentDifficulty);
        }
        QString recordVoice;
        if (recordType == 2) {
            recordVoice = QStringLiteral("恭喜！你打破了全国纪录！");
        }
        else if (recordType == 1) {
            recordVoice = QStringLiteral("恭喜你，刷新了个人最好成绩！");
        }

        if (!recordVoice.isEmpty()) {
            SpeechManager::instance().speak(recordVoice);
        }

        //定义结束流程：延迟显示弹窗
        auto showGameOverDialog = [this, finalScore]() {
            emit gameOver(finalScore);
            endGame();
            };

        //判断是否播放动画
        if (recordType > 0) {
            //调用 SceneGame 的动画接口
            m_scene->playNewRecordAnimation(recordType, [this, showGameOverDialog]() {
                //等 3 秒，然后弹窗
                QTimer::singleShot(5000, this, [showGameOverDialog]() {
                    showGameOverDialog();
                    });
                });
        }
        else {
            // 没有破纪录，直接弹窗
            QTimer::singleShot(500, this, showGameOverDialog);
        } 
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

        m_scene->setPauseButtonEnabled(false);
    }
}

void GameController::onGemClicked(int row, int col) {
    //如果正在处理动画，忽略所有点击
    if (m_isPaused || m_isProcessing) return;

    m_soundClick->play();
    m_scene->stopHintAnimation();

    QPoint currentClick(row, col);

    //情况A：当前没有选中任何宝石
    if (m_selectedPos.x() == -1) {
        m_selectedPos = currentClick;
        m_scene->setGemSelected(row, col, true); //通知 View 高亮
        return;
    }

    //情况B：点击了同一个宝石 -> 取消选中
    if (m_selectedPos == currentClick) {
        m_scene->setGemSelected(m_selectedPos.x(), m_selectedPos.y(), false);
        m_selectedPos = QPoint(-1, -1);
        return;
    }

    //情况C：点击了相邻的宝石 -> 尝试交换
    if (abs(m_selectedPos.x() - row) + abs(m_selectedPos.y() - col) == 1) {
        //取消高亮
        m_scene->setGemSelected(m_selectedPos.x(), m_selectedPos.y(), false);

        //锁定输入
        m_isProcessing = true;

        //执行交换逻辑
        attemptSwap(m_selectedPos, currentClick);

        //重置选中
        m_selectedPos = QPoint(-1, -1);
    }
    //情况D：点击了不相邻的宝石 -> 切换选中目标
    else {
        m_scene->setGemSelected(m_selectedPos.x(), m_selectedPos.y(), false);
        m_selectedPos = currentClick;
        m_scene->setGemSelected(row, col, true);
    }
}

void GameController::attemptSwap(const QPoint& p1, const QPoint& p2) {
    //先让 View 播放交换动画（Model 还没动）
    m_scene->animateSwap(p1.x(), p1.y(), p2.x(), p2.y(), [=]() {
        if (m_isTerminated) return;
        //判断是否为万能宝石交换
        const Board& board = m_gameCore->getBoard();
        GemType type1 = board.getGem(p1.x(), p1.y()).type;
        GemType type2 = board.getGem(p2.x(), p2.y()).type;
        bool isUniversal1 = (type1 == GemType::Universal);
        bool isUniversal2 = (type2 == GemType::Universal);

        //只要其中一个是万能宝石，就触发特殊消除
        if (isUniversal1 || isUniversal2) {
            GemType targetColor = isUniversal1 ? type2 : type1;

            if (targetColor == GemType::Universal) targetColor = GemType::Red;
            if (targetColor == GemType::Empty) return;

            //在数据层执行交换
            m_gameCore->getBoardPtr()->swapGem(p1.x(), p1.y(), p2.x(), p2.y());

            //消除所有该颜色的宝石
            int count = m_gameCore->explodeAllColor(targetColor);

            //播放音效与加分
			m_soundAll->play(); // 万能宝石消除音效
            m_comboLevel = 1;
            m_gameCore->addScoreSession(count * 10);
            m_scene->updateScore(m_gameCore->getScore());

            //搜集所有状态为 Exploding 的点
            std::vector<QPoint> explodePoints;
            const Board& currentBoard = m_gameCore->getBoard(); // 重新获取最新的引用
            for (int r = 0; r < BOARD_ROWS; ++r) {
                for (int c = 0; c < BOARD_COLS; ++c) {
                    if (currentBoard.getGem(r, c).state == GemState::Exploding) {
                        explodePoints.push_back(QPoint(r, c));
                    }
                }
            }

            //播放消除动画并在结束后进入下落流程
            m_scene->animateExplosion(explodePoints, [=]() {
                processFallAndMatch();
                });

            return;
        }

        //调用 Model 进行逻辑判断(非万能消除)
        SwapResult result = m_gameCore->trySwap(p1.x(), p1.y(), p2.x(), p2.y());

        if (result == SwapResult::Success) {
            // 播放消除音效
            if (m_gameCore->isSpecialMatch()) {
                m_soundLaser->play();
            }
            else {
                m_soundMouth->play(); 
            }

            //交换成功且消除了
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

            //播放消除动画
            m_scene->animateExplosion(explodePoints, [=]() {
                // 消除动画结束，开始处理下落
                processFallAndMatch();
                });
        }
        else {
            //交换无效（没有形成三连）
            m_scene->animateSwap(p2.x(), p2.y(), p1.x(), p1.y(), [=]() {
                //解锁输入
                m_isProcessing = false;
                });
        }
        });
}

void GameController::processFallAndMatch() {
    if (m_isTerminated) return;
    //把上一轮消除掉的宝石在数据层变为空
    m_gameCore->clearMatches();

    //执行下落和生成新宝石
    m_gameCore->applyGravityOnly(m_currentDifficulty);

    //播放下落动画
    m_scene->animateFall(m_gameCore->getBoard(), [=]() {
        m_gameCore->resetGemStates(); //全部重置为Static状态

        //扫描新盘面
        int comboSize = 0;
        bool hasNewMatches = m_gameCore->findAndMarkMatches(&comboSize);

        //判断是否有连击
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

            //搜集新的爆炸点
            std::vector<QPoint> explodePoints;
            const Board& board = m_gameCore->getBoard();
            for (int r = 0; r < BOARD_ROWS; ++r) {
                for (int c = 0; c < BOARD_COLS; ++c) {
                    if (board.getGem(r, c).state == GemState::Exploding) {
                        explodePoints.push_back(QPoint(r, c));
                    }
                }
            }

            //播放新一轮消除，再递归下落
            m_scene->animateExplosion(explodePoints, [=]() {
                processFallAndMatch(); // 递归进入下一轮
                });
        }
        else {
            //解锁输入
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