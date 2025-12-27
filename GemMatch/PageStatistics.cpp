#include "PageStatistics.h"
#include "MainWindow.h"
#include "UserManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include <QListWidgetItem>
#include <QDebug>

// 样式定义
const QString STATS_LABEL_STYLE =
"QLabel {"
"   background-color: rgba(255, 255, 255, 150);"
"   border-radius: 10px;"
"   border: 1px solid #044BB7;"
"   padding: 10px;"
"   color: #044BB7;"
"   font-weight: bold;"
"   font-size: 14px;"
"   min-height: 40px;"
"}";

PageStatistics::PageStatistics(MainWindow* mainWin)
    : QWidget(mainWin), m_mainWin(mainWin), m_currentDifficulty(3) {
    setupUI();
    loadStatisticsData();
}

PageStatistics::~PageStatistics() {
    if (m_player) m_player->stop();
}

void PageStatistics::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 1. 创建GraphicsView和视频背景
    m_view = new QGraphicsView(this);
    m_view->setStyleSheet("border: none; background: transparent;");
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QGraphicsScene* scene = new QGraphicsScene(this);
    m_view->setScene(scene);

    // 视频背景
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(0.0f);

    m_videoItem = new QGraphicsVideoItem();
    m_videoItem->setSize(QSizeF(1280, 800));
    m_videoItem->setZValue(0);
    scene->addItem(m_videoItem);
    m_player->setVideoOutput(m_videoItem);
    m_videoPath = "assets/videos/8.mp4";
    m_player->setLoops(QMediaPlayer::Infinite);

    // 2. 创建主容器（白色磨砂效果）
    QWidget* mainContainer = new QWidget();
    mainContainer->setFixedSize(1200, 700);
    mainContainer->setStyleSheet(
        "QWidget {"
        "   background-color: rgba(255, 255, 255, 100);"
        "   border-radius: 25px;"
        "   border: 2px solid rgba(255, 255, 255, 200);"
        "}"
    );

    QHBoxLayout* containerLayout = new QHBoxLayout(mainContainer);
    containerLayout->setContentsMargins(20, 20, 20, 20);
    containerLayout->setSpacing(20);

    // ========== 左侧：统计图表区 ==========
    QWidget* leftPanel = new QWidget();
    leftPanel->setMinimumWidth(700);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(15);

    // 难度选择按钮
    QHBoxLayout* difficultyLayout = new QHBoxLayout();
    difficultyLayout->setSpacing(30);  // 增加按钮之间的间距

    // 创建按钮并设置固定大小
    m_btnEasy = new GameButton("assets/images/简单1.png");
    m_btnNormal = new GameButton("assets/images/困难1.png");
    m_btnHard = new GameButton("assets/images/极限1.png");

    // 设置按钮大小
    m_btnEasy->setFixedSize(150, 60);
    m_btnNormal->setFixedSize(150, 60);
    m_btnHard->setFixedSize(150, 60);

    // 为按钮添加悬停效果（如果需要）
    QString buttonStyle =
        "QPushButton {"
        "   border: 2px solid #044BB7;"
        "   border-radius: 15px;"
        "   background-color: rgba(255, 255, 255, 200);"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(4, 75, 183, 150);"
        "   border: 2px solid #FFD700;"
        "}";

    m_btnEasy->setStyleSheet(buttonStyle);
    m_btnNormal->setStyleSheet(buttonStyle);
    m_btnHard->setStyleSheet(buttonStyle);

    // 添加按钮到布局，使用比例控制
    difficultyLayout->addStretch(1);
    difficultyLayout->addWidget(m_btnEasy);
    difficultyLayout->addSpacing(20);  // 添加间距
    difficultyLayout->addWidget(m_btnNormal);
    difficultyLayout->addSpacing(20);  // 添加间距
    difficultyLayout->addWidget(m_btnHard);
    difficultyLayout->addStretch(1);

    // 创建一个容器来包装难度按钮，确保它们不会被压缩
    QWidget* difficultyContainer = new QWidget();
    difficultyContainer->setLayout(difficultyLayout);
    difficultyContainer->setFixedHeight(80);  // 固定高度

    leftLayout->addWidget(difficultyContainer);

    // 连接难度切换信号
    connect(m_btnEasy, &QPushButton::clicked, [this]() { onDifficultyChanged(3); });
    connect(m_btnNormal, &QPushButton::clicked, [this]() { onDifficultyChanged(5); });
    connect(m_btnHard, &QPushButton::clicked, [this]() { onDifficultyChanged(7); });

    // 创建图表
    m_chartWidget = new ScoreChartWidget();
    m_chartWidget->setMinimumHeight(350);
    leftLayout->addWidget(m_chartWidget);

    // 统计数据面板
    QWidget* statsPanel = new QWidget();
    QGridLayout* statsLayout = new QGridLayout(statsPanel);
    statsLayout->setSpacing(10);

    m_lblTotalGames = new QLabel("总游戏场次: 0");
    m_lblAverageScore = new QLabel("平均得分: 0");
    m_lblMaxScore = new QLabel("最高得分: 0");
    m_lblWinRate = new QLabel("胜率: 0%");

    QLabel* statsLabels[] = { m_lblTotalGames, m_lblAverageScore, m_lblMaxScore, m_lblWinRate };
    for (QLabel* lbl : statsLabels) {
        lbl->setStyleSheet(STATS_LABEL_STYLE);
        lbl->setAlignment(Qt::AlignCenter);
    }

    statsLayout->addWidget(m_lblTotalGames, 0, 0);
    statsLayout->addWidget(m_lblAverageScore, 0, 1);
    statsLayout->addWidget(m_lblMaxScore, 1, 0);
    statsLayout->addWidget(m_lblWinRate, 1, 1);

    leftLayout->addWidget(statsPanel);

    // ========== 右侧：成就系统 ==========
    QWidget* rightPanel = new QWidget();
    rightPanel->setMinimumWidth(400);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(15);

    // 成就标题
    QLabel* achTitle = new QLabel("🏆 成就系统");
    achTitle->setStyleSheet(
        "QLabel {"
        "   color: #2a5493;"
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   background-color: rgba(255, 255, 255, 150);"
        "   border-radius: 15px;"
        "   padding: 10px;"
        "   border: 2px solid #FFD700;"
        "}"
    );
    achTitle->setAlignment(Qt::AlignCenter);
    rightLayout->addWidget(achTitle);

    // 成就列表
    m_achievementsList = new QListWidget();
    m_achievementsList->setStyleSheet(
        "QListWidget {"
        "   background-color: rgba(255, 255, 255, 150);"
        "   border-radius: 15px;"
        "   border: 2px solid #044BB7;"
        "}"
        "QListWidget::item {"
        "   border-bottom: 1px solid #ddd;"
        "   padding: 10px;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: rgba(4, 75, 183, 100);"
        "}"
    );
    rightLayout->addWidget(m_achievementsList);

    // 将左右面板添加到主容器
    containerLayout->addWidget(leftPanel);
    containerLayout->addWidget(rightPanel);

    // 将主容器添加到场景
    m_containerProxy = scene->addWidget(mainContainer);
    m_containerProxy->setZValue(1);


    // 3. 返回按钮（放在底部）
    m_btnBack = new GameButton("assets/images/返回主菜单.png");
    m_btnBack->setFixedSize(200, 60);

    QGraphicsProxyWidget* backProxy = scene->addWidget(m_btnBack);
    backProxy->setZValue(3);

    connect(m_btnBack, &QPushButton::clicked, this, &PageStatistics::onBackClicked);

    mainLayout->addWidget(m_view);
}

void PageStatistics::loadStatisticsData() {
    // 初始化成就列表
    m_achievements.clear();

    // 定义成就
    Achievement ach;

    // 基础成就
    ach = { "first_game", "初出茅庐", "完成第一局游戏", false, 0, 1 };
    m_achievements.append(ach);

    ach = { "play_10_games", "持之以恒", "累计完成10局游戏", false, 0, 10 };
    m_achievements.append(ach);

    ach = { "play_50_games", "游戏达人", "累计完成50局游戏", false, 0, 50 };
    m_achievements.append(ach);

    // 得分成就
    ach = { "score_100", "百分达人", "单局得分超过100分", false, 0, 1 };
    m_achievements.append(ach);

    ach = { "score_500", "五百分高手", "单局得分超过500分", false, 0, 1 };
    m_achievements.append(ach);

    ach = { "score_1000", "千分大师", "单局得分超过1000分", false, 0, 1 };
    m_achievements.append(ach);

    // 难度成就
    ach = { "easy_master", "简单模式大师", "简单模式最高分达到300分", false, 0, 300 };
    m_achievements.append(ach);

    ach = { "normal_master", "普通模式大师", "普通模式最高分达到500分", false, 0, 500 };
    m_achievements.append(ach);

    ach = { "hard_master", "困难模式大师", "困难模式最高分达到800分", false, 0, 800 };
    m_achievements.append(ach);

    // 连击成就
    ach = { "combo_3", "三连击", "达成3连击", false, 0, 1 };
    m_achievements.append(ach);

    ach = { "combo_5", "五连击", "达成5连击", false, 0, 1 };
    m_achievements.append(ach);

    // 道具成就
    ach = { "use_bomb", "炸弹专家", "使用炸弹道具", false, 0, 1 };
    m_achievements.append(ach);

    ach = { "use_all_skills", "道具大师", "使用所有类型的道具", false, 0, 4 };
    m_achievements.append(ach);

    ach = { "full_combo", "完美连击", "一局游戏中无失误交换", false, 0, 1 };
    m_achievements.append(ach);

    // 从数据库或文件加载成就解锁状态
    loadAchievementProgress();

    // 默认显示简单难度的数据
    updateChart(3);
    updateAchievements();
}

void PageStatistics::updateChart(int difficultyLevel) {
    m_currentDifficulty = difficultyLevel;

    // 从UserManager获取最近得分数据
    QList<int> recentScores = UserManager::instance().getCurrentUserRecentScores(difficultyLevel);

    // 设置图表数据
    QString difficultyName;
    switch (difficultyLevel) {
    case 3: difficultyName = "简单"; break;
    case 5: difficultyName = "普通"; break;
    case 7: difficultyName = "困难"; break;
    default: difficultyName = "未知";
    }

    m_chartWidget->setScores(recentScores, difficultyName + "模式 - 最近游戏得分趋势");
    m_chartWidget->setDifficulty(difficultyLevel);

    if (recentScores.isEmpty()) {
        // 没有数据，显示提示
        m_lblTotalGames->setText("总游戏场次: 0");
        m_lblAverageScore->setText("平均得分: 0");
        m_lblMaxScore->setText("最高得分: 0");
        m_lblWinRate->setText("胜率: 0%");

        return;
    }

    // 计算统计数据
    int totalGames = recentScores.size();
    int totalScore = 0;
    int maxScore = 0;
    int winCount = 0;

    for (int score : recentScores) {
        totalScore += score;
        if (score > maxScore) maxScore = score;
        if (score > 50) winCount++; // 假设得分超过50分算胜利
    }

    int averageScore = totalScore / totalGames;
    int winRate = (winCount * 100) / totalGames;
    int highScore = UserManager::instance().getCurrentUserHighScore(difficultyLevel);

    // 更新统计标签
    m_lblTotalGames->setText(QString("总游戏场次: %1").arg(totalGames));
    m_lblAverageScore->setText(QString("平均得分: %1").arg(averageScore));
    m_lblMaxScore->setText(QString("最高得分: %1").arg(highScore));
    m_lblWinRate->setText(QString("胜率: %1%").arg(winRate));

    // 重新检查成就
    checkBasicAchievements();
    checkScoreAchievements();
    updateAchievements();
}

void PageStatistics::updateAchievements() {
    m_achievementsList->clear();

    // 计算已解锁成就数量
    int unlockedCount = 0;
    for (const auto& ach : m_achievements) {
        if (ach.unlocked) unlockedCount++;
    }

    // 添加成就标题（带进度）
    QListWidgetItem* titleItem = new QListWidgetItem(QString("成就进度: %1/%2").arg(unlockedCount).arg(m_achievements.size()));
    titleItem->setTextAlignment(Qt::AlignCenter);
    titleItem->setFlags(Qt::NoItemFlags);
    titleItem->setForeground(QBrush(QColor("#2a5493"))); // 添加这行，设置字体颜色为 #2a5493
    titleItem->setFont(QFont("Microsoft YaHei", 12, QFont::Bold)); // 可以设置字体
    m_achievementsList->addItem(titleItem);

    // 添加分割线
    QListWidgetItem* separator = new QListWidgetItem();
    separator->setFlags(Qt::NoItemFlags);
    separator->setBackground(QBrush(QColor(200, 200, 200)));
    separator->setSizeHint(QSize(0, 2));
    m_achievementsList->addItem(separator);

    // 添加每个成就项
    for (const auto& ach : m_achievements) {
        QString displayText;
        if (ach.target > 1) {
            displayText = QString("%1\n%2 [%3/%4]")
                .arg(ach.name)
                .arg(ach.description)
                .arg(ach.progress)
                .arg(ach.target);
        }
        else {
            displayText = QString("%1\n%2").arg(ach.name).arg(ach.description);
        }

        QListWidgetItem* item = new QListWidgetItem(displayText);

        if (ach.unlocked) {
            item->setForeground(QBrush(QColor("#2a5493"))); // 修改为 #2a5493
            item->setIcon(QIcon(":/icons/achievement_unlocked.png"));
            // 已解锁成就可以加粗显示
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);
        }
        else {
            item->setForeground(QBrush(QColor("#2a5493"))); // 修改为 #2a5493
            item->setIcon(QIcon(":/icons/achievement_locked.png"));
        }

        m_achievementsList->addItem(item);
    }
}

void PageStatistics::checkBasicAchievements() {
    QString currentUser = UserManager::instance().getCurrentUser();
    if (currentUser.isEmpty()) return;

    // 计算总游戏场次
    int totalGames = 0;
    for (int difficulty : {3, 5, 7}) {
        QList<int> scores = UserManager::instance().getCurrentUserRecentScores(difficulty);
        totalGames += scores.size();
    }

    // 检查"第一局游戏"成就
    if (totalGames >= 1 && !isAchievementUnlocked("first_game")) {
        unlockAchievement("first_game", "初出茅庐", "完成第一局游戏");
    }

    // 检查"10局游戏"成就
    if (totalGames >= 10 && !isAchievementUnlocked("play_10_games")) {
        unlockAchievement("play_10_games", "持之以恒", "累计完成10局游戏");
    }

    // 更新进度
    for (auto& ach : m_achievements) {
        if (ach.id == "play_10_games") {
            ach.progress = std::min(totalGames, ach.target);
        }
        if (ach.id == "play_50_games") {
            ach.progress = std::min(totalGames, ach.target);
            if (totalGames >= 50 && !ach.unlocked) {
                unlockAchievement("play_50_games", "游戏达人", "累计完成50局游戏");
            }
        }
    }
}

void PageStatistics::checkScoreAchievements() {
    // 检查各难度最高分成就
    for (int difficulty : {3, 5, 7}) {
        int highScore = UserManager::instance().getCurrentUserHighScore(difficulty);

        QString achId;
        if (difficulty == 3) achId = "easy_master";
        else if (difficulty == 5) achId = "normal_master";
        else achId = "hard_master";

        // 更新进度
        for (auto& ach : m_achievements) {
            if (ach.id == achId) {
                ach.progress = std::min(highScore, ach.target);
                if (highScore >= ach.target && !ach.unlocked) {
                    QString achName;
                    if (difficulty == 3) achName = "简单模式大师";
                    else if (difficulty == 5) achName = "普通模式大师";
                    else achName = "困难模式大师";

                    unlockAchievement(achId, achName, ach.description);
                }
            }
        }
    }

    // 检查单局得分成就
    for (int difficulty : {3, 5, 7}) {
        QList<int> scores = UserManager::instance().getCurrentUserRecentScores(difficulty);
        for (int score : scores) {
            if (score >= 100 && !isAchievementUnlocked("score_100")) {
                unlockAchievement("score_100", "百分达人", "单局得分超过100分");
            }
            if (score >= 500 && !isAchievementUnlocked("score_500")) {
                unlockAchievement("score_500", "五百分高手", "单局得分超过500分");
            }
            if (score >= 1000 && !isAchievementUnlocked("score_1000")) {
                unlockAchievement("score_1000", "千分大师", "单局得分超过1000分");
            }
        }
    }
}

void PageStatistics::unlockAchievement(const QString& id, const QString& name, const QString& desc) {
    for (auto& ach : m_achievements) {
        if (ach.id == id) {
            ach.unlocked = true;
            qDebug() << "成就解锁:" << name;

            // 保存到文件
            saveAchievementProgress();

            // 显示解锁通知（可选）
            QMessageBox::information(this, "🎉 成就解锁",
                QString("恭喜解锁成就：%1\n%2").arg(name).arg(desc));

            break;
        }
    }
}

bool PageStatistics::isAchievementUnlocked(const QString& id) {
    for (const auto& ach : m_achievements) {
        if (ach.id == id && ach.unlocked) {
            return true;
        }
    }
    return false;
}

void PageStatistics::loadAchievementProgress() {
    QString currentUser = UserManager::instance().getCurrentUser();
    if (currentUser.isEmpty()) return;

    QString filePath = QDir::currentPath() + "/achievements_" + currentUser + ".json";

    QFile file(filePath);
    if (!file.exists()) return;

    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();

        if (!doc.isObject()) return;

        QJsonObject obj = doc.object();
        for (auto& ach : m_achievements) {
            if (obj.contains(ach.id)) {
                QJsonObject achObj = obj[ach.id].toObject();
                ach.unlocked = achObj["unlocked"].toBool();
                ach.progress = achObj["progress"].toInt();
            }
        }
    }
}

void PageStatistics::saveAchievementProgress() {
    QString currentUser = UserManager::instance().getCurrentUser();
    if (currentUser.isEmpty()) return;

    QJsonObject root;
    for (const auto& ach : m_achievements) {
        QJsonObject achObj;
        achObj["unlocked"] = ach.unlocked;
        achObj["progress"] = ach.progress;

        root[ach.id] = achObj;
    }

    QJsonDocument doc(root);
    QString filePath = QDir::currentPath() + "/achievements_" + currentUser + ".json";

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void PageStatistics::onDifficultyChanged(int difficulty) {
    updateChart(difficulty);
}

void PageStatistics::onBackClicked() {
    m_mainWin->switchPage(1); // 返回主菜单
}

void PageStatistics::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (m_videoItem && m_view) {
        m_videoItem->setSize(QSizeF(this->size()));
        m_view->setSceneRect(0, 0, this->width(), this->height());

        // 定位主容器
        if (m_containerProxy) {
            QWidget* w = m_containerProxy->widget();
            if (w) m_containerProxy->setPos((width() - w->width()) / 2, (height() - w->height()) / 2);
        }

        // 定位Logo
        if (m_logoProxy) {
            QWidget* w = m_logoProxy->widget();
            if (w) m_logoProxy->setPos((width() - w->width()) / 2, height() * 0.02);
        }

        // 定位返回按钮
        QList<QGraphicsItem*> items = m_view->scene()->items();
        for (auto item : items) {
            if (QGraphicsProxyWidget* proxy = dynamic_cast<QGraphicsProxyWidget*>(item)) {
                if (proxy->widget() && proxy->widget() == m_btnBack) {
                    proxy->setPos(width() - m_btnBack->width() - 20, height() - m_btnBack->height() - 20);
                    break;
                }
            }
        }
    }
}

void PageStatistics::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);

    // 播放背景视频
    if (m_player) {
        m_player->setSource(QUrl::fromLocalFile(m_videoPath));
        m_player->play();
    }

    // 启动Logo动画
    if (m_logo) {
        m_logo->startEntrance();
    }

    // 刷新数据
    loadStatisticsData();
    updateChart(m_currentDifficulty);
}

void PageStatistics::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);

    // 停止背景视频
    if (m_player) {
        m_player->stop();
        m_player->setSource(QUrl());
    }
}