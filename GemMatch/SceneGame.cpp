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
    rightPanel->setFixedSize(350, 650); // 设置右侧面板的大小
    rightPanel->setStyleSheet(
        "QWidget {"
        "   background-color: rgba(255, 255, 255, 100);" /* 白色半透明 */
        "   border-radius: 20px;"
        "   border: 1px solid rgba(255, 255, 255, 200);"
        "}"
    );

    QVBoxLayout* sideLayout = new QVBoxLayout(rightPanel);
    sideLayout->setContentsMargins(20, 20, 20, 20);
    sideLayout->setSpacing(2);

    // -------------------------------------------------
    // 【新增】 1. 顶部显示头像
    // -------------------------------------------------
    m_avatarLabel = new QLabel();
    m_avatarLabel->setFixedSize(100, 100);
    m_avatarLabel->setStyleSheet("background: transparent; border: none;");

    // 生成圆形头像 (同上)
    QPixmap rawPix("assets/images/1.png");
    if (!rawPix.isNull()) {
        QPixmap circularPix(100, 100);
        circularPix.fill(Qt::transparent);
        QPainter painter(&circularPix);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        QPainterPath path;
        path.addEllipse(2, 2, 96, 96);
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, 100, 100, rawPix);
        painter.setClipping(false);
        QPen pen(Qt::white, 4);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(2, 2, 96, 96);
        m_avatarLabel->setPixmap(circularPix);
    }

    // 居中添加到布局顶部
    sideLayout->addWidget(m_avatarLabel, 0, Qt::AlignHCenter);

    // --- 状态区 ---
    QWidget* statusBox = new QWidget();
    statusBox->setStyleSheet("background: transparent; border: none;");
    QVBoxLayout* statusLayout = new QVBoxLayout(statusBox);
    statusLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* lblTimeTitle = new QLabel("得分：");
    lblTimeTitle->setAlignment(Qt::AlignCenter);
    lblTimeTitle->setStyleSheet("font-size: 16px; color: #555; font-weight: bold; background: transparent; border: none;");

    m_timeLabel = new QLabel("60");
    m_timeLabel->setAlignment(Qt::AlignCenter);
    m_timeLabel->setStyleSheet("font-size: 56px; font-weight: bold; color: #044BB7; background: transparent; border: none; font-family: Arial;");

    statusLayout->addWidget(m_timeLabel);
    statusLayout->addWidget(lblTimeTitle);



    m_scoreDisplay = new QLCDNumber();
    m_scoreDisplay->setDigitCount(6);
    m_scoreDisplay->setSegmentStyle(QLCDNumber::Flat);
    m_scoreDisplay->setStyleSheet("border: none; color: #FF4500; background: rgba(0,0,0,0.05); border-radius: 10px; height: 50px;");

    statusLayout->addWidget(m_scoreDisplay);
    sideLayout->addWidget(statusBox);

    // --- 技能区 ---
    QGridLayout* skillGrid = new QGridLayout();
    skillGrid->setSpacing(5);

    // 创建按钮时指定父对象防止内存泄漏，虽然 layout 会接管
    m_btnSkillBomb = new GameButton("assets/images/炸弹.png");
    m_btnSkillShuffle = new GameButton("assets/images/洗牌.png");
    m_btnSkillTime = new GameButton("assets/images/冻结.png");
    m_btnSkillAll = new GameButton("assets/images/万能.png");


    QFont font("Microsoft YaHei", 20, QFont::Bold);
    m_btnSkillBomb->setFont(font);
    m_btnSkillShuffle->setFont(font);
    m_btnSkillTime->setFont(font);
    m_btnSkillAll-> setFont(font);

    skillGrid->addWidget(m_btnSkillBomb, 0, 0);
    skillGrid->addWidget(m_btnSkillShuffle, 0, 1);
    skillGrid->addWidget(m_btnSkillTime, 1, 0);
    skillGrid->addWidget(m_btnSkillAll, 1, 1);
    sideLayout->addLayout(skillGrid);

    sideLayout->addStretch(); // 弹簧

    // --- 功能按钮 ---
    m_btnHint = new GameButton("assets/images/提示.png");
    m_btnPause = new GameButton("assets/images/暂停游戏.png");
    m_btnExit = new GameButton("assets/images/返回主菜单.png");

    QVBoxLayout* funcLayout = new QVBoxLayout();
    funcLayout->setSpacing(15);
    funcLayout->setAlignment(Qt::AlignHCenter);
    funcLayout->addWidget(m_btnHint);
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
    connect(m_btnSkillAll, &QPushButton::clicked, this, &SceneGame::skillAll);
    connect(m_btnHint, &QPushButton::clicked, this, &SceneGame::hintRequested);
    connect(m_btnExit, &QPushButton::clicked, this, &SceneGame::backToMenu);
}

// 关键：在 resizeEvent 中分别计算两个组件的位置
void SceneGame::setPauseButtonText(const QString& path) {
    if (!m_btnPause) return;

    m_btnPause->setPixmap(path);
}

void SceneGame::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

    if (m_videoItem && m_bgView) {
        m_videoItem->setSize(QSizeF(this->size()));
        m_bgView->setSceneRect(0, 0, this->width(), this->height());

        // 计算布局参数
        qreal boardWidth = 600.0;
        qreal rightPanelWidth = 350.0;
        qreal spacing = 50.0; // 两个面板之间的间距
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

void SceneGame::updateSkillButtonText(int bombCount, int shuffleCount, int timeCount, int allCount) {
    if (m_btnSkillBomb) {
        m_btnSkillBomb->setText(QString("(%1)").arg(bombCount));
        m_btnSkillBomb->setEnabled(bombCount > 0); // 次数耗尽则禁用按钮
    }
    if (m_btnSkillShuffle) {
        m_btnSkillShuffle->setText(QString("(%1)").arg(shuffleCount));
        m_btnSkillShuffle->setEnabled(shuffleCount > 0);
    }
    if (m_btnSkillTime) {
        m_btnSkillTime->setText(QString("(%1)").arg(timeCount));
        m_btnSkillTime->setEnabled(timeCount > 0);
    }
    if (m_btnSkillAll) {
        m_btnSkillAll->setText(QString("(%1)").arg(allCount));
        m_btnSkillAll->setEnabled(allCount > 0);
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

//震动动画实现
void SceneGame::startShakeAnimation() {
    // 1. 安全检查
    if (!m_boardProxy) return;

    // 2. 获取棋盘当前的位置 (由 resizeEvent 计算出来的那个位置)
    QPointF originalPos = m_boardProxy->pos();

    // 3. 创建串行动画组
    QSequentialAnimationGroup* group = new QSequentialAnimationGroup(this);

    // 4. 设置震动参数
    int intensity = 15; // 震动幅度 (像素)
    int duration = 30;  // 每次位移耗时 (毫秒)

    // --- 关键：定义一组乱序的位移路径 ---

    // 向右下
    QPropertyAnimation* anim1 = new QPropertyAnimation(m_boardProxy, "pos");
    anim1->setDuration(duration);
    anim1->setEndValue(originalPos + QPointF(intensity, intensity));
    group->addAnimation(anim1);

    // 向左上 (大幅反向拉扯)
    QPropertyAnimation* anim2 = new QPropertyAnimation(m_boardProxy, "pos");
    anim2->setDuration(duration);
    anim2->setEndValue(originalPos + QPointF(-intensity, -intensity));
    group->addAnimation(anim2);

    // 向左下
    QPropertyAnimation* anim3 = new QPropertyAnimation(m_boardProxy, "pos");
    anim3->setDuration(duration);
    anim3->setEndValue(originalPos + QPointF(-intensity, intensity));
    group->addAnimation(anim3);

    // 向右上
    QPropertyAnimation* anim4 = new QPropertyAnimation(m_boardProxy, "pos");
    anim4->setDuration(duration);
    anim4->setEndValue(originalPos + QPointF(intensity, -intensity));
    group->addAnimation(anim4);

    // --- 5. 最后必须归位！ ---
    QPropertyAnimation* animEnd = new QPropertyAnimation(m_boardProxy, "pos");
    animEnd->setDuration(duration);
    animEnd->setEndValue(originalPos); // 回到原点
    animEnd->setEasingCurve(QEasingCurve::OutBounce); // 加一点回弹效果
    group->addAnimation(animEnd);

    // 6. 播放并自动删除
    group->start(QAbstractAnimation::DeleteWhenStopped);
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

void SceneGame::stopHintAnimation() {
    for (auto anim : m_hintAnims) {
        if (anim) {
            anim->stop();
            delete anim;
        }
    }
    m_hintAnims.clear();

    // 确保所有宝石恢复原状 (scale 1.0)
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (m_items[i][j]) m_items[i][j]->setScale(1.0);
        }
    }
}

void SceneGame::showHintAnimation(const QPoint& p1, const QPoint& p2) {
    stopHintAnimation(); // 先停止之前的

    GemItem* item1 = m_items[p1.x()][p1.y()];
    GemItem* item2 = m_items[p2.x()][p2.y()];

    if (!item1 || !item2) return;

    // 创建一个让宝石“呼吸”的缩放动画
    auto createPulseAnim = [this](GemItem* item) {
        QPropertyAnimation* anim = new QPropertyAnimation(item, "scale");
        anim->setDuration(600);
        anim->setStartValue(1.0);
        anim->setKeyValueAt(0.5, 1.2); // 放大到 1.2 倍
        anim->setEndValue(1.0);
        anim->setLoopCount(-1); // 无限循环
        anim->setEasingCurve(QEasingCurve::InOutQuad);
        m_hintAnims.append(anim); // 存起来方便管理
        anim->start();
        };

    createPulseAnim(item1);
    createPulseAnim(item2);
}

//void SceneGame::playNewRecordAnimation(int recordType, std::function<void()> callback) {
//    // 1. 准备图片 (确保 assets/images/破纪录.png 存在)
//    QString imagePath;
//    if (recordType == 2) {
//        imagePath = "assets/images/全国新纪录.png"; // 全服纪录图片
//    }
//    else {
//        imagePath = "assets/images/破纪录.png"; // 个人纪录图片
//    }
//
//    QPixmap pix(imagePath);
//    if (pix.isNull()) {
//        qDebug() << "错误：找不到破纪录图片 -> " << imagePath;
//        if (callback) callback();
//        return;
//    }
//
//    // 2. 创建或重置图元
//    if (!m_recordItem) {
//        m_recordItem = new QGraphicsPixmapItem(pix);
//        m_bgScene->addItem(m_recordItem); // 添加到背景 Scene 最上层
//        m_recordItem->setZValue(100);     // 确保在最前面
//    }
//    else {
//        m_recordItem->setPixmap(pix);
//        m_recordItem->setVisible(true);
//    }
//
//    // 3. 设置初始位置 (屏幕正上方外部)
//    // 假设窗口宽度 this->width()，让图片水平居中
//    qreal targetX = (this->width() - pix.width()) / 2.0;
//    qreal targetY = (this->height() - pix.height()) / 2.0; // 最终停在屏幕中央
//    qreal startY = -pix.height() - 100; // 从屏幕上方看不到的地方开始
//
//    m_recordItem->setPos(targetX, startY);
//
//    // 4. ✅ 【关键修复】使用 QVariantAnimation 代替 QPropertyAnimation
//        // 因为 QGraphicsPixmapItem 不是 QObject，不能直接用 QPropertyAnimation
//    QVariantAnimation* anim = new QVariantAnimation(this);
//    anim->setDuration(5000); // 1秒
//    anim->setStartValue(QPointF(targetX, startY));
//    anim->setEndValue(QPointF(targetX, targetY));
//    anim->setEasingCurve(QEasingCurve::OutBounce); // 弹跳效果
//
//    // 5. 动画结束后调用回调
//    connect(anim, &QAbstractAnimation::finished, this, [anim, callback]() {
//        anim->deleteLater();
//        if (callback) callback();
//        });
//
//    anim->start();
//}



void SceneGame::playNewRecordAnimation(int recordType, std::function<void()> callback) {
    QString imageName = (recordType == 2) ? "全国新纪录.png" : "破纪录.png";
    QString imagePath = "assets/images/" + imageName;

    // 尝试加载图片
    QPixmap pix(imagePath);

    // 安全检查：如果图片加载失败，直接执行回调，避免卡住流程
    if (pix.isNull()) {
        if (callback) callback();
        return;
    }

    // 2. 创建或重置图元
    if (!m_recordItem) {
        m_recordItem = new QGraphicsPixmapItem(pix);
        m_bgScene->addItem(m_recordItem);
        m_recordItem->setZValue(9999); // 确保在最前面
    }
    else {
        m_recordItem->setPixmap(pix);
        m_recordItem->setVisible(true);
        m_recordItem->setZValue(9999);
    }

    // 3. 计算位置
    qreal targetX = (this->width() - pix.width()) / 2.0;
    qreal targetY = (this->height() - pix.height()) / 2.0;
    qreal startY = -pix.height() - 100;

    // 初始位置
    m_recordItem->setPos(targetX, startY);

    // 4. 使用 QVariantAnimation 驱动
    QVariantAnimation* anim = new QVariantAnimation(this);
    anim->setDuration(1000);
    anim->setStartValue(QPointF(targetX, startY));
    anim->setEndValue(QPointF(targetX, targetY));
    anim->setEasingCurve(QEasingCurve::OutBounce);

    // 关键：监听每一帧的变化，手动更新图元位置
    connect(anim, &QVariantAnimation::valueChanged, this, [this](const QVariant& val) {
        if (m_recordItem) {
            m_recordItem->setPos(val.toPointF());
        }
        });

    // 动画结束
    connect(anim, &QAbstractAnimation::finished, this, [anim, callback]() {
        anim->deleteLater();
        if (callback) callback();
        });

    anim->start();
}