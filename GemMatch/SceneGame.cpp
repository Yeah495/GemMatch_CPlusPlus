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

SceneGame::SceneGame(MainWindow* mainWin)
    : QWidget(mainWin), m_mainWin(mainWin)
{
    // 初始化指针数组为空
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            m_items[i][j] = nullptr;

    setupUI();
}

SceneGame::~SceneGame() {
    // Qt 的对象树机制会自动清理 m_view, m_scene 和 UI 控件
}

void SceneGame::setupUI() {
    // 1. 设置整个窗口的背景：优先使用 qrc 资源路径
    QString qrcBg = ":/assets/images/bg_login.jpg";
    if (QFile::exists(qrcBg)) {
        this->setStyleSheet(QString("SceneGame { border-image: url(%1); } QLabel { color: blue; font-weight: bold; font-family: 'Microsoft YaHei'; } QPushButton { background-color: rgba(255, 255, 255, 0.9); border: 2px solid #aaa; border-radius: 8px; padding: 5px; font-weight: bold; font-size: 14px; } QPushButton:hover { background-color: white; border-color: gold; }") .arg(qrcBg));
    }
    else {
        // fallback to ResourceLoader pixmap
        QPixmap bg = ResourceLoader::instance().getBackground();
        if (!bg.isNull()) {
            QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
            if (cacheDir.isEmpty()) cacheDir = QCoreApplication::applicationDirPath();
            QString outPath = cacheDir + "/bg_game_temp.png";
            bg.save(outPath);
            this->setStyleSheet(QString("SceneGame { border-image: url(%1); } QLabel { color: white; font-weight: bold; font-family: 'Microsoft YaHei'; } QPushButton { background-color: rgba(255, 255, 255, 0.9); border: 2px solid #aaa; border-radius: 8px; padding: 5px; font-weight: bold; font-size: 14px; } QPushButton:hover { background-color: white; border-color: gold; }") .arg(outPath));
        }
        else {
            this->setStyleSheet("SceneGame { border-image: url(:/assets/images/bg_game.png); } QLabel { color: white; font-weight: bold; font-family: 'Microsoft YaHei'; } QPushButton { background-color: rgba(255, 255, 255, 0.9); border: 2px solid #aaa; border-radius: 8px; padding: 5px; font-weight: bold; font-size: 14px; } QPushButton:hover { background-color: white; border-color: gold; }");
        }
    }

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    // --- 左侧：图形视图 (Graphics View) ---
    m_scene = new QGraphicsScene(this);
    m_scene->setSceneRect(0, 0, 580, 580); // 设置场景逻辑大小

    // 使用资源背景作为场景背景
    QString sceneQrc = ":/assets/images/bg_login.jpg";
    if (QFile::exists(sceneQrc)) {
        QPixmap sceneBg(sceneQrc);
        m_scene->setBackgroundBrush(QBrush(sceneBg.scaled(580, 580, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
    }
    else {
        QPixmap sceneBg = ResourceLoader::instance().getBackground();
        if (!sceneBg.isNull()) {
            m_scene->setBackgroundBrush(QBrush(sceneBg.scaled(580, 580, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
        }
    }

    m_view = new QGraphicsView(m_scene);
    m_view->setFixedSize(600, 600); // 视图固定大小
    // 视图样式：透明背景，无边框
    m_view->setStyleSheet("background: transparent; border: 2px solid rgba(255,255,255,0.3); border-radius: 15px;");
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setRenderHint(QPainter::SmoothPixmapTransform);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    mainLayout->addWidget(m_view);

    // --- 右侧：控制面板 (UI) ---
    QVBoxLayout* sideLayout = new QVBoxLayout();

    // 1. 状态区
    QGroupBox* statusBox = new QGroupBox("当前状态");
    statusBox->setStyleSheet("QGroupBox { color: gold; border: 1px solid rgba(255,255,255,0.5); border-radius: 5px; margin-top: 20px; font-size: 16px; font-weight: bold; } "
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; }");
    QVBoxLayout* statusLayout = new QVBoxLayout(statusBox);
    statusLayout->setSpacing(10);

    // 分数
    QLabel* lblScoreTitle = new QLabel("得分 ");
    lblScoreTitle->setAlignment(Qt::AlignCenter);

    m_scoreDisplay = new QLCDNumber();
    m_scoreDisplay->setDigitCount(6);
    m_scoreDisplay->setSegmentStyle(QLCDNumber::Flat);
    m_scoreDisplay->setStyleSheet("border: none; color: gold; background-color: rgba(0,0,0,0.6); border-radius: 5px; height: 40px;");

    // 时间
    m_timeLabel = new QLabel("剩余时间: 60s");
    m_timeLabel->setAlignment(Qt::AlignCenter);
    m_timeLabel->setStyleSheet("font-size: 20px; color: cyan; margin-top: 10px;");

    statusLayout->addWidget(lblScoreTitle);
    statusLayout->addWidget(m_scoreDisplay);
    statusLayout->addWidget(m_timeLabel);
    sideLayout->addWidget(statusBox);

    // 2. 技能栏
    QGroupBox* skillBox = new QGroupBox("技能栏");
    QGridLayout* skillLayout = new QGridLayout(skillBox);

    m_btnSkillBomb = new QPushButton("炸弹道具");
    m_btnSkillShuffle = new QPushButton("洗牌道具");
    m_btnSkillTime = new QPushButton("时间冻结");

    skillLayout->addWidget(m_btnSkillBomb, 0, 0);
    skillLayout->addWidget(m_btnSkillShuffle, 0, 1);
    skillLayout->addWidget(m_btnSkillTime, 1, 0, 1, 2);
    sideLayout->addWidget(skillBox);

    sideLayout->addStretch(); // 弹簧占位

    // 3. 系统按钮
    m_btnPause = new QPushButton("暂停游戏");
    m_btnExit = new QPushButton("返回主菜单");
    m_btnExit->setStyleSheet("QPushButton { background-color: rgba(255, 80, 80, 0.8); color: red; } QPushButton:hover { background-color: red; }");

    sideLayout->addWidget(m_btnPause);
    sideLayout->addWidget(m_btnExit);

    mainLayout->addLayout(sideLayout);

    // --- 信号连接 ---
    connect(m_btnExit, &QPushButton::clicked, [this]() {
        emit backToMenu(); // 发送信号给 MainWindow
        });
}

// 计算屏幕坐标：基于 8x8 网格居中
QPointF SceneGame::getScreenPos(int row, int col) const {
    // 假设场景 580x580，格子 65x65，8个格子共 520
    // 边距 = (580 - 520) / 2 = 30
    int offsetX = 30;
    int offsetY = 30;
    return QPointF(offsetX + col * CELL_SIZE, offsetY + row * CELL_SIZE);
}

void SceneGame::renderBoard(const Board& board) {
    m_scene->clear(); // 清空旧图元

    // 重置指针数组
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            m_items[i][j] = nullptr;

    // 遍历生成新宝石
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            Gem g = board.getGem(r, c);
            if (g.type == GemType::Empty) continue;

            GemItem* item = new GemItem(r, c, g.type);
            item->setPos(getScreenPos(r, c));
            m_scene->addItem(item);
            m_items[r][c] = item;

            // 关键：连接宝石的点击信号到本类的信号
            connect(item, &GemItem::clicked, this, &SceneGame::gemClicked);
        }
    }
}

void SceneGame::updateScore(int score) {
    m_scoreDisplay->display(score);
}

void SceneGame::updateTime(int seconds) {
    m_timeLabel->setText(QString("剩余时间: %1s").arg(seconds));
    if (seconds <= 10) {
        m_timeLabel->setStyleSheet("font-size: 20px; color: red; margin-top: 10px; font-weight: bold;");
    }
    else {
        m_timeLabel->setStyleSheet("font-size: 20px; color: cyan; margin-top: 10px;");
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

    if (!item1 || !item2) {
        if (finishedCallback) finishedCallback();
        return;
    }

    // 交换内存中的指针，保证逻辑一致性
    std::swap(m_items[r1][c1], m_items[r2][c2]);
    item1->setGridPos(r2, c2);
    item2->setGridPos(r1, c1);

    // 动画组
    QParallelAnimationGroup* group = new QParallelAnimationGroup;

    QPropertyAnimation* anim1 = new QPropertyAnimation(item1, "pos");
    anim1->setDuration(250);
    anim1->setStartValue(item1->pos());
    anim1->setEndValue(getScreenPos(r2, c2));
    anim1->setEasingCurve(QEasingCurve::InOutQuad);

    QPropertyAnimation* anim2 = new QPropertyAnimation(item2, "pos");
    anim2->setDuration(250);
    anim2->setStartValue(item2->pos());
    anim2->setEndValue(getScreenPos(r1, c1));
    anim2->setEasingCurve(QEasingCurve::InOutQuad);

    group->addAnimation(anim1);
    group->addAnimation(anim2);

    connect(group, &QAbstractAnimation::finished, this, [group, finishedCallback]() {
        if (finishedCallback) finishedCallback();
        group->deleteLater();
        });
    group->start();
}

void SceneGame::animateExplosion(const std::vector<QPoint>& points, std::function<void()> finishedCallback) {
    QParallelAnimationGroup* group = new QParallelAnimationGroup;

    for (const auto& p : points) {
        if (m_items[p.x()][p.y()]) {
            GemItem* item = m_items[p.x()][p.y()];

            // 缩放+透明度淡出
            QPropertyAnimation* animScale = new QPropertyAnimation(item, "scale");
            animScale->setDuration(200);
            animScale->setEndValue(0.1);

            QPropertyAnimation* animOpacity = new QPropertyAnimation(item, "opacity");
            animOpacity->setDuration(200);
            animOpacity->setEndValue(0.0);

            group->addAnimation(animScale);
            group->addAnimation(animOpacity);
        }
    }

    connect(group, &QAbstractAnimation::finished, this, [this, points, group, finishedCallback]() {
        // 动画结束，移除图元
        for (const auto& p : points) {
            GemItem* item = m_items[p.x()][p.y()];
            if (item) {
                m_scene->removeItem(item);
                delete item;
                m_items[p.x()][p.y()] = nullptr;
            }
        }
        if (finishedCallback) finishedCallback();
        group->deleteLater();
        });
    group->start();
}

void SceneGame::animateFall(const Board& newBoard, std::function<void()> finishedCallback) {
    // 简单实现：清空并根据新状态重绘，并对新生成的/位置变化的做掉落动画
    // 这种方式最稳健，不会出现图元错位

    QParallelAnimationGroup* group = new QParallelAnimationGroup;
    bool anyMove = false;

    // 1. 先记录旧的图元，准备复用或移除
    // 为了简化代码，VC环境下我们直接采用“重建法”配合动画
    m_scene->clear();
    // 重置数组
    for (int i = 0; i < 8; ++i) for (int j = 0; j < 8; ++j) m_items[i][j] = nullptr;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            Gem g = newBoard.getGem(r, c);
            if (g.type == GemType::Empty) continue;

            GemItem* item = new GemItem(r, c, g.type);
            m_scene->addItem(item);
            m_items[r][c] = item;
            connect(item, &GemItem::clicked, this, &SceneGame::gemClicked);

            QPointF targetPos = getScreenPos(r, c);

            if (g.state == GemState::Falling) {
                // 如果标记为下落，从上面飞下来
                QPointF startPos = targetPos - QPointF(0, 100);
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