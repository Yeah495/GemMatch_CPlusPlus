#include "SceneGame.h"
#include "MainWindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QDebug>
#include "ResourceLoader.h"
#include <QStandardPaths>
#include <QBuffer>
#include <QPainter>
#include <QStyleOption>
#include <QGraphicsProxyWidget>
#include <QUrl>

SceneGame::SceneGame(MainWindow* mainWin)
    : QWidget(mainWin), m_mainWin(mainWin), m_boardProxy(nullptr), m_rightPanelProxy(nullptr)
{
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            m_items[i][j] = nullptr;

    setupUI();
}

SceneGame::~SceneGame() {}

void SceneGame::paintEvent(QPaintEvent* event) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void SceneGame::setupUI() {
    this->setObjectName("SceneGame");

    // 1. 根布局
    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    // 2. 背景 View
    m_bgView = new QGraphicsView(this);
    m_bgView->setStyleSheet("border: none; background: transparent;");
    m_bgView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_bgView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_bgScene = new QGraphicsScene(this);
    m_bgView->setScene(m_bgScene);
    rootLayout->addWidget(m_bgView);

    // 3. 视频背景
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(0.0f);

    m_videoItem = new QGraphicsVideoItem();
    m_videoItem->setSize(QSizeF(1280, 800));
    m_videoItem->setZValue(0);
    m_bgScene->addItem(m_videoItem);

    m_player->setVideoOutput(m_videoItem);
    m_videoPath = "assets/videos/4.mp4";
    m_player->setLoops(QMediaPlayer::Infinite);

    // =========================================================
    // 4. 左侧：棋盘部分 (独立)
    // =========================================================
    m_scene = new QGraphicsScene(this);
    m_scene->setSceneRect(0, 0, 580, 580);

    m_view = new QGraphicsView(m_scene);
    m_view->setFixedSize(600, 600); // 固定大小
    m_view->setStyleSheet("background: transparent; border: 2px solid rgba(255,255,255,0.6); border-radius: 15px;");
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setRenderHint(QPainter::SmoothPixmapTransform);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 直接将棋盘 View 添加到背景场景中
    m_boardProxy = m_bgScene->addWidget(m_view);
    m_boardProxy->setZValue(1);

    // =========================================================
    // 5. 右侧：控制面板 (独立白色磨砂框)
    // =========================================================
    QWidget* rightPanel = new QWidget();
    rightPanel->setFixedSize(350, 600); // 设置右侧面板的大小
    rightPanel->setStyleSheet(
        "QWidget {"
        "   background-color: rgba(255, 255, 255, 100);" /* 白色半透明 */
        "   border-radius: 20px;"
        "   border: 1px solid rgba(255, 255, 255, 200);"
        "}"
    );

    QVBoxLayout* sideLayout = new QVBoxLayout(rightPanel);
    sideLayout->setContentsMargins(30, 40, 30, 40);
    sideLayout->setSpacing(20);

    // --- 状态区 ---
    QWidget* statusBox = new QWidget();
    statusBox->setStyleSheet("background: transparent; border: none;");
    QVBoxLayout* statusLayout = new QVBoxLayout(statusBox);
    statusLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* lblTimeTitle = new QLabel("剩余时间");
    lblTimeTitle->setAlignment(Qt::AlignCenter);
    lblTimeTitle->setStyleSheet("font-size: 16px; color: #555; font-weight: bold; background: transparent; border: none;");

    m_timeLabel = new QLabel("60");
    m_timeLabel->setAlignment(Qt::AlignCenter);
    m_timeLabel->setStyleSheet("font-size: 56px; font-weight: bold; color: #044BB7; background: transparent; border: none; font-family: Arial;");

    statusLayout->addWidget(m_timeLabel);
    statusLayout->addWidget(lblTimeTitle);

    QLabel* lblScoreTitle = new QLabel("得分");
    lblScoreTitle->setAlignment(Qt::AlignCenter);
    lblScoreTitle->setStyleSheet("font-size: 16px; color: #555; font-weight: bold; margin-top: 20px; background: transparent; border: none;");

    m_scoreDisplay = new QLCDNumber();
    m_scoreDisplay->setDigitCount(6);
    m_scoreDisplay->setSegmentStyle(QLCDNumber::Flat);
    m_scoreDisplay->setStyleSheet("border: none; color: #FF4500; background: rgba(0,0,0,0.05); border-radius: 10px; height: 50px;");

    statusLayout->addWidget(m_scoreDisplay);
    statusLayout->addWidget(lblScoreTitle);
    sideLayout->addWidget(statusBox);

    // --- 技能区 ---
    QGridLayout* skillGrid = new QGridLayout();
    skillGrid->setSpacing(15);

    // 创建按钮时指定父对象防止内存泄漏，虽然 layout 会接管
    m_btnSkillBomb = new GameButton("assets/images/技能通用.png");
    m_btnSkillShuffle = new GameButton("assets/images/技能通用.png");
    m_btnSkillTime = new GameButton("assets/images/技能通用.png");
    m_btnSkill4 = new GameButton("assets/images/技能通用.png");

    m_btnSkillBomb->setText("炸弹 (3)");
    m_btnSkillShuffle->setText("洗牌 (1)");
    m_btnSkillTime->setText("时间冻结 (2)");
    QFont font("Microsoft YaHei", 12, QFont::Bold);
    m_btnSkillBomb->setFont(font);
    m_btnSkillShuffle->setFont(font);
    m_btnSkillTime->setFont(font);

    skillGrid->addWidget(m_btnSkillBomb, 0, 0);
    skillGrid->addWidget(m_btnSkillShuffle, 0, 1);
    skillGrid->addWidget(m_btnSkillTime, 1, 0);
    skillGrid->addWidget(m_btnSkill4, 1, 1);
    sideLayout->addLayout(skillGrid);

    sideLayout->addStretch(); // 弹簧

    // --- 功能按钮 ---
    m_btnPause = new GameButton("assets/images/按键通用.png");
    m_btnExit = new GameButton("assets/images/按键通用.png");
    m_btnExit->setText("返回主菜单");

    QHBoxLayout* funcLayout = new QHBoxLayout();
    funcLayout->setSpacing(15);
    funcLayout->addWidget(m_btnPause);
    funcLayout->addWidget(m_btnExit);
    sideLayout->addLayout(funcLayout);

    // 将右侧面板添加到背景场景
    m_rightPanelProxy = m_bgScene->addWidget(rightPanel);
    m_rightPanelProxy->setZValue(1);

    // 信号连接
    connect(m_btnExit, &QPushButton::clicked, [this]() {
        emit backToMenu();
        });

    //连接功能按钮到信号
    connect(m_btnPause, &QPushButton::clicked, this, &SceneGame::pauseGame);
    connect(m_btnSkillBomb, &QPushButton::clicked, this, &SceneGame::skillBomb);
    connect(m_btnSkillShuffle, &QPushButton::clicked, this, &SceneGame::skillShuffle);
    connect(m_btnSkillTime, &QPushButton::clicked, this, &SceneGame::skillTime);
}

// 关键：在 resizeEvent 中分别计算两个组件的位置
void SceneGame::setPauseButtonText(const QString& text) {
    if (m_btnPause) {
        m_btnPause->setText(text);
    }
}

void SceneGame::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

    if (m_videoItem && m_bgView) {
        m_videoItem->setSize(QSizeF(this->size()));
        m_bgView->setSceneRect(0, 0, this->width(), this->height());

        // 计算布局参数
        qreal boardWidth = 600.0;
        qreal rightPanelWidth = 350.0;
        qreal spacing = 40.0; // 两个面板之间的间距
        qreal totalWidth = boardWidth + spacing + rightPanelWidth;

        // 计算起始 X 坐标，使整体居中
        qreal startX = (this->width() - totalWidth) / 2.0;
        qreal startY = (this->height() - 600.0) / 2.0; // 假设高度也是600左右居中

        // 1. 定位棋盘
        if (m_boardProxy) {
            m_boardProxy->setPos(startX, startY);
        }

        // 2. 定位右侧面板
        if (m_rightPanelProxy) {
            m_rightPanelProxy->setPos(startX + boardWidth + spacing, startY);
        }
    }
}

// 其余函数 (getScreenPos, renderBoard, animate..., update...) 保持不变
// ...
// 务必保留 animateSwap, animateExplosion 等所有动画逻辑
// ...

QPointF SceneGame::getScreenPos(int row, int col) const {
    int offsetX = 30;
    int offsetY = 30;
    return QPointF(offsetX + col * CELL_SIZE, offsetY + row * CELL_SIZE);
}

void SceneGame::renderBoard(const Board& board) {
    m_scene->clear();
    for (int i = 0; i < 8; ++i) for (int j = 0; j < 8; ++j) m_items[i][j] = nullptr;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            Gem g = board.getGem(r, c);
            if (g.type == GemType::Empty) continue;

            GemItem* item = new GemItem(r, c, g.type);
            item->setPos(getScreenPos(r, c));
            m_scene->addItem(item);
            m_items[r][c] = item;
            connect(item, &GemItem::clicked, this, &SceneGame::gemClicked);
        }
    }
}

void SceneGame::updateScore(int score) {
    m_scoreDisplay->display(score);
}

void SceneGame::updateSkillButtonText(int bombCount, int shuffleCount, int timeCount) {
    if (m_btnSkillBomb) {
        m_btnSkillBomb->setText(QString("炸弹 (%1)").arg(bombCount));
        m_btnSkillBomb->setEnabled(bombCount > 0); // 次数耗尽则禁用按钮
    }
    if (m_btnSkillShuffle) {
        m_btnSkillShuffle->setText(QString("洗牌 (%1)").arg(shuffleCount));
        m_btnSkillShuffle->setEnabled(shuffleCount > 0);
    }
    if (m_btnSkillTime) {
        m_btnSkillTime->setText(QString("冻结 (%1)").arg(timeCount));
        m_btnSkillTime->setEnabled(timeCount > 0);
    }
}

void SceneGame::updateTime(int seconds, bool isFrozen) {

    // 基础样式：字号和边距
    QString baseStyle = "font-size: 20px; margin-top: 10px; font-weight: bold;";

    if (isFrozen) {
        // --- ❄️ 冻结状态：冰蓝色 + 特殊文字 ---
        m_timeLabel->setText(QString("❄️时间冻结: %1s").arg(seconds));
        // DeepSkyBlue 是很好看的冰蓝色，或者用 #00BFFF
        m_timeLabel->setStyleSheet(baseStyle + "color: #00BFFF;");
    }
    else {
        // --- 正常计时状态 ---
        m_timeLabel->setText(QString("剩余时间: %1s").arg(seconds));

        if (seconds <= 10) {
            // ⚠️ 倒计时警报：红色
            m_timeLabel->setStyleSheet(baseStyle + "color: red;");
        }
        else {
            // ✅ 正常：青色
            m_timeLabel->setStyleSheet(baseStyle + "color: cyan;");
        }
    }
}
void SceneGame::setGemSelected(int r, int c, bool selected) {
    if (r >= 0 && r < 8 && c >= 0 && c < 8 && m_items[r][c]) {
        m_items[r][c]->setSelected(selected);
    }
}

// --- 动画实现部分 ---

void SceneGame::animateSwap(int r1, int c1, int r2, int c2, std::function<void()> finishedCallback) {
    GemItem* item1 = m_items[r1][c1];
    GemItem* item2 = m_items[r2][c2];
    if (!item1 || !item2) { if (finishedCallback) finishedCallback(); return; }
    std::swap(m_items[r1][c1], m_items[r2][c2]);
    item1->setGridPos(r2, c2); item2->setGridPos(r1, c1);
    QParallelAnimationGroup* group = new QParallelAnimationGroup;
    QPropertyAnimation* anim1 = new QPropertyAnimation(item1, "pos");
    anim1->setDuration(250); anim1->setStartValue(item1->pos()); anim1->setEndValue(getScreenPos(r2, c2)); anim1->setEasingCurve(QEasingCurve::InOutQuad);
    QPropertyAnimation* anim2 = new QPropertyAnimation(item2, "pos");
    anim2->setDuration(250); anim2->setStartValue(item2->pos()); anim2->setEndValue(getScreenPos(r1, c1)); anim2->setEasingCurve(QEasingCurve::InOutQuad);
    group->addAnimation(anim1); group->addAnimation(anim2);
    connect(group, &QAbstractAnimation::finished, this, [group, finishedCallback]() { if (finishedCallback) finishedCallback(); group->deleteLater(); });
    group->start();
}

void SceneGame::animateExplosion(const std::vector<QPoint>& points, std::function<void()> finishedCallback) {
    QParallelAnimationGroup* group = new QParallelAnimationGroup;
    for (const auto& p : points) {
        if (m_items[p.x()][p.y()]) {
            GemItem* item = m_items[p.x()][p.y()];
            QPropertyAnimation* animScale = new QPropertyAnimation(item, "scale");
            animScale->setDuration(200); animScale->setEndValue(0.1);
            QPropertyAnimation* animOpacity = new QPropertyAnimation(item, "opacity");
            animOpacity->setDuration(200); animOpacity->setEndValue(0.0);
            group->addAnimation(animScale); group->addAnimation(animOpacity);
        }
    }
    connect(group, &QAbstractAnimation::finished, this, [this, points, group, finishedCallback]() {
        for (const auto& p : points) {
            GemItem* item = m_items[p.x()][p.y()];
            if (item) { m_scene->removeItem(item); delete item; m_items[p.x()][p.y()] = nullptr; }
        }
        if (finishedCallback) finishedCallback(); group->deleteLater();
        });
    group->start();
}

void SceneGame::animateFall(const Board& newBoard, std::function<void()> finishedCallback) {
    QParallelAnimationGroup* group = new QParallelAnimationGroup;
    bool anyMove = false;
    m_scene->clear();
    for (int i = 0; i < 8; ++i) for (int j = 0; j < 8; ++j) m_items[i][j] = nullptr;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            Gem g = newBoard.getGem(r, c);
            if (g.type == GemType::Empty) continue;
            GemItem* item = new GemItem(r, c, g.type);
            m_scene->addItem(item); m_items[r][c] = item;
            connect(item, &GemItem::clicked, this, &SceneGame::gemClicked);
            QPointF targetPos = getScreenPos(r, c);
            if (g.state == GemState::Falling) {
                QPointF startPos = targetPos - QPointF(0, CELL_SIZE);
                if (r == 0) startPos = targetPos - QPointF(0, CELL_SIZE * 1.5);
                item->setPos(startPos);
                QPropertyAnimation* anim = new QPropertyAnimation(item, "pos");
                anim->setDuration(400); anim->setStartValue(startPos); anim->setEndValue(targetPos);
                anim->setEasingCurve(QEasingCurve::OutBounce);
                group->addAnimation(anim); anyMove = true;
            }
            else { item->setPos(targetPos); }
        }
    }
    if (anyMove) {
        connect(group, &QAbstractAnimation::finished, this, [group, finishedCallback]() { if (finishedCallback) finishedCallback(); group->deleteLater(); });
        group->start();
    }
    else { delete group; if (finishedCallback) finishedCallback(); }
}

//void SceneGame::updateScore(int score) {
//    m_scoreDisplay->display(score);
//}
//
//void SceneGame::updateTime(int seconds) {
//    m_timeLabel->setText(QString::number(seconds));
//    if (seconds <= 10) m_timeLabel->setStyleSheet("font-size: 56px; font-weight: bold; color: red; font-family: Arial; border: none; background: transparent;");
//    else m_timeLabel->setStyleSheet("font-size: 56px; font-weight: bold; color: #044BB7; font-family: Arial; border: none; background: transparent;");
//}
//
//void SceneGame::setGemSelected(int r, int c, bool selected) {
//    if (r >= 0 && r < 8 && c >= 0 && c < 8 && m_items[r][c]) {
//        m_items[r][c]->setSelected(selected);
//    }
//}

void SceneGame::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    if (m_player) { m_player->setSource(QUrl::fromLocalFile(m_videoPath)); m_player->play(); }
}

void SceneGame::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    if (m_player) { m_player->stop(); m_player->setSource(QUrl()); }
}