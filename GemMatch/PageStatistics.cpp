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
"   font-size: 20px;"
"   min-height: 40px;"
"}";

PageStatistics::PageStatistics(MainWindow* mainWin)
    : QWidget(mainWin), m_mainWin(mainWin), m_currentDifficulty(3) {
    setupUI();
    loadStatisticsData();

    // 初始化成就显示
    initAchievementDisplay();
}

PageStatistics::~PageStatistics() {
    if (m_player) m_player->stop();
}

void PageStatistics::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 创建视频背景
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

    // 创建主容器
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

    //统计图表区
    QWidget* leftPanel = new QWidget();
    leftPanel->setMinimumWidth(700);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(15);

    // 难度选择按钮
    QHBoxLayout* difficultyLayout = new QHBoxLayout();
    difficultyLayout->setSpacing(30);  // 增加按钮之间的间距

    // 创建按钮
    m_btnEasy = new GameButton("assets/images/简单1.png");
    m_btnNormal = new GameButton("assets/images/困难1.png");
    m_btnHard = new GameButton("assets/images/极限1.png");

    // 设置按钮大小
    m_btnEasy->setFixedSize(150, 60);
    m_btnNormal->setFixedSize(150, 60);
    m_btnHard->setFixedSize(150, 60);

    // 为按钮添加悬停效果
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

    // 添加按钮到布局
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

    //成就系统
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


    // 返回按钮
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

    ach = { "play_100_games", "百战不殆", "累计完成100局游戏", false, 0, 100 };
    m_achievements.append(ach);

    // 得分成就
    ach = { "score_100", "百分达人", "单局得分超过100分", false, 0, 1 };
    m_achievements.append(ach);

    ach = { "score_500", "五百分高手", "单局得分超过500分", false, 0, 1 };
    m_achievements.append(ach);

    ach = { "score_1000", "千分大师", "单局得分超过1000分", false, 0, 1 };
    m_achievements.append(ach);

    ach = { "score_2000", "两千分精英", "单局得分超过2000分", false, 0, 1 };
    m_achievements.append(ach);

    ach = { "score_5000", "五千分宗师", "单局得分超过5000分", false, 0, 1 };
    m_achievements.append(ach);

    ach = { "score_10000", "万分王者", "单局得分超过10000分", false, 0, 1 };
    m_achievements.append(ach);

    ach = { "score_20000", "两万分传奇", "单局得分超过20000分", false, 0, 1 };
    m_achievements.append(ach);

    // 难度成就
    ach = { "easy_master", "简单模式大师", "简单模式最高分达到300分", false, 0, 300 };
    m_achievements.append(ach);

    ach = { "normal_master", "普通模式大师", "普通模式最高分达到500分", false, 0, 500 };
    m_achievements.append(ach);

    ach = { "hard_master", "困难模式大师", "困难模式最高分达到800分", false, 0, 800 };
    m_achievements.append(ach);

    // 从数据库或文件加载成就解锁状态
    loadAchievementProgress();

    // 默认显示简单难度的数据
    updateChart(3);
    updateAchievements();
}

// 改变难度时更新图表和统计数据
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
        m_lblAverageScore->setText("最近10场平均得分: 0");
        m_lblMaxScore->setText("最高得分: 0");
        m_lblWinRate->setText("胜率: 0%");

        return;
    }

    // 计算统计数据
    int totalGamesInChart = recentScores.size();
    int totalScore = 0;
    int maxScore = 0;
    int winCount = 0;

    for (int score : recentScores) {
        totalScore += score;
        if (score > maxScore) maxScore = score;
        if (score > 50) winCount++;
    }

    int averageScore = totalGamesInChart > 0 ? totalScore / totalGamesInChart : 0;
    int winRate = totalGamesInChart > 0 ? (winCount * 100) / totalGamesInChart : 0;
    int highScore = UserManager::instance().getCurrentUserHighScore(difficultyLevel);

    // 获取该难度的真实总游戏场次
    int realTotalGames = UserManager::instance().getCurrentUserTotalGames(difficultyLevel);

    // 更新统计标签
    m_lblTotalGames->setText(QString("总游戏场次: %1").arg(realTotalGames));  // 显示真实总场次
    m_lblAverageScore->setText(QString("最近10场平均得分: %1").arg(averageScore));
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

    // 计算总游戏场次
    int totalGames = UserManager::instance().getCurrentUserAllTotalGames();

    // 添加成就标题
    QListWidgetItem* titleItem = new QListWidgetItem(
        QString("成就进度: %1/%2 (总游戏场次: %3)")
        .arg(unlockedCount)
        .arg(m_achievements.size())
        .arg(totalGames));  
    titleItem->setTextAlignment(Qt::AlignCenter);
    titleItem->setFlags(Qt::NoItemFlags);
    titleItem->setForeground(QBrush(QColor("#2a5493")));
    titleItem->setFont(QFont("Microsoft YaHei", 18, QFont::Bold));
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

        // 对于游戏次数成就，显示当前进度
        if (ach.id == "play_10_games" || ach.id == "play_50_games" || ach.id == "play_100_games") {
            // 获取真实总场次作为进度
            int totalGames = UserManager::instance().getCurrentUserAllTotalGames();
            displayText = QString("%1\n%2 [%3/%4]")
                .arg(ach.name)
                .arg(ach.description)
                .arg(std::min(totalGames, ach.target))  
                .arg(ach.target);
        }
        // 对于分数成就，显示最高分进度
        else if (ach.id == "easy_master" || ach.id == "normal_master" || ach.id == "hard_master") {
            // 获取当前难度
            int difficulty = 3;
            if (ach.id == "normal_master") difficulty = 5;
            else if (ach.id == "hard_master") difficulty = 7;

            int highScore = UserManager::instance().getCurrentUserHighScore(difficulty);
            displayText = QString("%1\n%2 [%3/%4]")
                .arg(ach.name)
                .arg(ach.description)
                .arg(highScore)
                .arg(ach.target);
        }
        // 其他成就
        else if (ach.target > 1) {
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
            item->setForeground(QBrush(QColor("#2a5493")));
            item->setIcon(QIcon(":/icons/achievement_unlocked.png"));

            // 已解锁成就可以加粗显示
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);

            // 对于100场游戏成就，可以显示特殊图标
            if (ach.id == "play_100_games") {
                item->setForeground(QBrush(QColor("#FFD700"))); 
            }
        }
        else {
            item->setForeground(QBrush(QColor("#666666"))); // 灰色表示未解锁
            item->setIcon(QIcon(":/icons/achievement_locked.png"));

            // 对于100场游戏成就，可以显示进度条效果
            if (ach.id == "play_100_games" && ach.progress > 0) {
                QString progressPercent = QString::number((ach.progress * 100) / ach.target);
                item->setText(displayText + QString(" (%1%)").arg(progressPercent));
            }
        }

        m_achievementsList->addItem(item);
    }
}

void PageStatistics::checkBasicAchievements() {
    QString currentUser = UserManager::instance().getCurrentUser();
    if (currentUser.isEmpty()) return;

    // 计算总游戏场次
    int totalGames = UserManager::instance().getCurrentUserAllTotalGames();

    qDebug() << "检查基础成就，总游戏场次:" << totalGames;

    // 检查并更新所有基础游戏次数成就
    for (auto& ach : m_achievements) {
        if (ach.id == "first_game") {
            // 只有当有真实的游戏记录时才解锁
            if (totalGames >= 1 && !ach.unlocked) {
                // 确保用户真的有游戏记录
                bool hasGameRecord = false;
                for (int difficulty : {3, 5, 7}) {
                    QList<int> recentScores = UserManager::instance().getCurrentUserRecentScores(difficulty);
                    if (!recentScores.isEmpty()) {
                        hasGameRecord = true;
                        break;
                    }
                }

                if (hasGameRecord) {
                    unlockAchievement("first_game", "初出茅庐", "完成第1局游戏");
                }
            }
        }
        else if (ach.id == "play_10_games") {
            ach.progress = std::min(totalGames, ach.target);
            if (totalGames >= 10 && !ach.unlocked) {
                unlockAchievement("play_10_games", "持之以恒", "累计完成10局游戏");
            }
        }
        else if (ach.id == "play_50_games") {
            ach.progress = std::min(totalGames, ach.target);
            if (totalGames >= 50 && !ach.unlocked) {
                unlockAchievement("play_50_games", "游戏达人", "累计完成50局游戏");
            }
        }
        else if (ach.id == "play_100_games") {
            ach.progress = std::min(totalGames, ach.target);
            if (totalGames >= 100 && !ach.unlocked) {
                unlockAchievement("play_100_games", "百战不殆", "累计完成100局游戏");
            }
        }
    }
}

void PageStatistics::checkScoreAchievements() {
    // 检查各难度最高分成就
    for (int difficulty : {3, 5, 7}) {
        int highScore = UserManager::instance().getCurrentUserHighScore(difficulty);

        qDebug() << "难度" << difficulty << "最高分:" << highScore;

        QString achId;
        if (difficulty == 3) achId = "easy_master";
        else if (difficulty == 5) achId = "normal_master";
        else achId = "hard_master";

        // 更新进度
        for (auto& ach : m_achievements) {
            if (ach.id == achId) {
                ach.progress = std::min(highScore, ach.target);
                // 只有当最高分确实大于0且达到目标时才解锁
                if (highScore > 0 && highScore >= ach.target && !ach.unlocked) {
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

        qDebug() << "难度" << difficulty << "最近得分记录:" << scores;

        for (int score : scores) {
            if (score <= 0) continue;

            // 检查得分成就
            if (score >= 100 && !isAchievementUnlocked("score_100")) {
                unlockAchievement("score_100", "百分达人", "单局得分超过100分");
            }
            if (score >= 500 && !isAchievementUnlocked("score_500")) {
                unlockAchievement("score_500", "五百分高手", "单局得分超过500分");
            }
            if (score >= 1000 && !isAchievementUnlocked("score_1000")) {
                unlockAchievement("score_1000", "千分大师", "单局得分超过1000分");
            }
            if (score >= 2000 && !isAchievementUnlocked("score_2000")) {
                unlockAchievement("score_2000", "两千分精英", "单局得分超过2000分");
            }
            if (score >= 5000 && !isAchievementUnlocked("score_5000")) {
                unlockAchievement("score_5000", "五千分宗师", "单局得分超过5000分");
            }
            if (score >= 10000 && !isAchievementUnlocked("score_10000")) {
                unlockAchievement("score_10000", "万分王者", "单局得分超过10000分");
            }
            if (score >= 20000 && !isAchievementUnlocked("score_20000")) {
                unlockAchievement("score_20000", "两万分传奇", "单局得分超过20000分");
            }
        }
    }
}

void PageStatistics::unlockAchievement(const QString& id, const QString& name, const QString& desc) {
    for (auto& ach : m_achievements) {
        if (ach.id == id && !ach.unlocked) {
            ach.unlocked = true;
            ach.progress = ach.target; // 设置进度为完成
            qDebug() << "成就解锁:" << name << "(" << id << ")";

            // 保存到文件
            saveAchievementProgress();

            // 将成就添加到队列
            m_achievementQueue.append(qMakePair(id, name));

            // 如果没有正在显示成就，则立即显示
            if (!m_achievementTimer->isActive() && m_achievementDisplay->isHidden()) {
                showNextAchievement();
            }


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

    // 首先检查用户是否真的有游戏记录
    bool hasGameRecord = false;
    int totalGames = UserManager::instance().getCurrentUserAllTotalGames();

    if (totalGames == 0) {
        // 如果没有游戏记录，强制重置所有成就状态
        qDebug() << "用户没有游戏记录，重置成就状态";
        for (auto& ach : m_achievements) {
            ach.unlocked = false;
            ach.progress = 0;
        }

        // 删除可能存在的成就文件
        QString filePath = QDir::currentPath() + "/achievements_" + currentUser + ".json";
        QFile file(filePath);
        if (file.exists()) {
            file.remove();
            qDebug() << "已删除旧的成就文件";
        }
        return;
    }

    // 如果有游戏记录，才从文件加载成就状态
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
                bool wasUnlocked = achObj["unlocked"].toBool();
                int savedProgress = achObj["progress"].toInt();

                // 对于"第一局游戏"成就，需要额外检查
                if (ach.id == "first_game") {
                    if (totalGames >= 1) {
                        ach.unlocked = true;
                        ach.progress = 1;
                    }
                    else {
                        ach.unlocked = false;
                        ach.progress = 0;
                    }
                }
                else {
                    ach.unlocked = wasUnlocked;
                    ach.progress = savedProgress;
                }
            }
        }
    }
}

// 保存成就进度到文件
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

            // 定位成就显示
            if (m_achievementDisplay && m_achievementDisplay->isVisible()) {
                m_achievementDisplay->move(m_containerProxy->pos().x() + (1200 - 400) / 2,
                    m_containerProxy->pos().y() + 20);
            }
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

    // 刷新数据
    loadStatisticsData();
    updateChart(m_currentDifficulty);

    // 显示队列中的成就
    if (!m_achievementQueue.isEmpty()) {
        showNextAchievement();
    }
}

void PageStatistics::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);

    // 停止背景视频
    if (m_player) {
        m_player->stop();
        m_player->setSource(QUrl());
    }

    // 隐藏成就显示
    if (m_achievementDisplay) {
        m_achievementDisplay->hide();
    }

    // 停止定时器
    if (m_achievementTimer) {
        m_achievementTimer->stop();
    }
}

void PageStatistics::initAchievementDisplay() {
    // 创建成就显示标签
    m_achievementDisplay = new QLabel(this);
    m_achievementDisplay->setFixedSize(450, 120); 
    m_achievementDisplay->setAlignment(Qt::AlignCenter);
    m_achievementDisplay->setStyleSheet(
        "QLabel {"
        "   background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0,"
        "       stop:0 rgba(74, 144, 226, 220),"
        "       stop:1 rgba(42, 84, 147, 220));"
        "   border-radius: 15px;"
        "   border: 3px solid #FFD700;"
        "   color: white;"
        "   font-size: 100px;"
        "   font-weight: bold;"
        "}"
    );
    m_achievementDisplay->setWordWrap(true);
    m_achievementDisplay->hide();

    // 添加图标显示
    m_achievementDisplay->setScaledContents(false);
    m_achievementDisplay->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    // 创建定时器
    m_achievementTimer = new QTimer(this);
    m_achievementTimer->setSingleShot(true);
    connect(m_achievementTimer, &QTimer::timeout, [this]() {
        m_achievementDisplay->hide();
        showNextAchievement();
        });
}

void PageStatistics::showNextAchievement() {
    if (m_achievementQueue.isEmpty()) {
        return;
    }

    // 获取第一个成就
    auto achievement = m_achievementQueue.takeFirst();
    QString id = achievement.first;  
    QString name = achievement.second; 

    // 根据成就ID选择不同的图片
    QString imagePath = ":/icons/achievement_unlocked.png"; 

    // 根据成就的重要程度设置不同的图片
    if (id.contains("10000") || id.contains("20000") || id.contains("宗师") || id.contains("王者")) {
        imagePath = "assets/achievement/achievement_gold.png"; // 高级成就
    }
    else if (id.contains("1000") || id.contains("2000") || id.contains("5000")) {
        imagePath = "assets/achievement/achievement_silver.png"; // 中级成就
    }
    else {
        imagePath = "assets/achievement/achievement_bronze.png"; // 基础成就
    }

    // 设置显示内容
    QString displayHtml = QString(
        "<html>"
        "<div align='center'>"
        "<img src='%1' width='64' height='64'><br>"
        "<b style='font-size:18px; color:#FFD700;'>🎉 成就解锁 🎉</b><br>"
        "<span style='font-size:16px;'>%2</span><br>"
        "<span style='font-size:14px;'>%3</span>"
        "</div>"
        "</html>"
    ).arg(imagePath).arg(name).arg(getAchievementDescriptionById(id));

    m_achievementDisplay->setText(displayHtml);

    // 显示位置
    if (m_containerProxy) {
        QPointF containerPos = m_containerProxy->pos();
        m_achievementDisplay->move(containerPos.x() + (1200 - 450) / 2, containerPos.y() + 20);
    }

    // 显示
    m_achievementDisplay->show();
    m_achievementDisplay->raise();

    // 1.5秒后自动关闭
    m_achievementTimer->start(1500);
}

// 根据成就ID获取描述
QString PageStatistics::getAchievementDescriptionById(const QString& id) {
    for (const auto& ach : m_achievements) {
        if (ach.id == id) {
            return ach.description;
        }
    }
    return "";
}

