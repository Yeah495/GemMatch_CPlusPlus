#include "SceneRank.h"
#include "MainWindow.h"
#include "UserManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

SceneRank::SceneRank(MainWindow* mainWin) : QWidget(mainWin), m_mainWin(mainWin) {
    setupUI();
}

void SceneRank::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

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
    m_videoPath = "assets/videos/7.mp4";
    m_player->setLoops(QMediaPlayer::Infinite);

    // =========================================================
    // 容器 (白色磨砂)
    // =========================================================
    QWidget* container = new QWidget();
    container->setFixedSize(800, 600);
    container->setStyleSheet(
        "QWidget {"
        "   background-color: rgba(255, 255, 255, 100);"
        "   border-radius: 20px;"
        "   border: 1px solid rgba(255, 255, 255, 200);"
        "}"
    );

    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setContentsMargins(30, 40, 30, 30);
    layout->setSpacing(15);

    // 难度切换按钮区
    QHBoxLayout* modeLayout = new QHBoxLayout();
    modeLayout->addStretch();
    m_btnEasy = new GameButton("assets/images/难度通用.png");
    m_btnHard = new GameButton("assets/images/难度通用.png");
    m_btnExtreme = new GameButton("assets/images/难度通用.png");
    modeLayout->addWidget(m_btnEasy);
    modeLayout->addWidget(m_btnHard);
    modeLayout->addWidget(m_btnExtreme);
    modeLayout->addStretch();
    layout->addLayout(modeLayout);

    // 表格
    m_table = new QTableWidget();
    m_table->setColumnCount(3);
    m_table->setHorizontalHeaderLabels({ "排名", "用户", "得分" });
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);

    // 表格样式适配浅色背景
    m_table->setStyleSheet(
        "QTableWidget {"
        "   background: rgba(255, 255, 255, 150);" // 稍微更不透明一点
        "   gridline-color: #ccc;"
        "   color: #333;" // 深色字体
        "   font-size: 16px;"
        "   border: none;"
        "   border-radius: 10px;"
        "}"
        "QHeaderView::section {"
        "   background: #044BB7;" // 表头深蓝
        "   color: white;"
        "   padding: 8px;"
        "   font-weight: bold;"
        "   border: none;"
        "}"
        "QTableWidget::item { padding: 5px; }"
    );
    layout->addWidget(m_table);

    // 返回按钮
    m_btnBack = new GameButton("assets/images/按键通用.png");
    QHBoxLayout* backLayout = new QHBoxLayout();
    backLayout->addStretch();
    backLayout->addWidget(m_btnBack);
    backLayout->addStretch();
    layout->addLayout(backLayout);

    connect(m_btnBack, &QPushButton::clicked, [this]() { m_mainWin->switchPage(1); });

    // 模拟点击切换 (实际逻辑需配合数据库查询条件)
    connect(m_btnEasy, &QPushButton::clicked, [this]() { loadRankData(); });

    m_containerProxy = scene->addWidget(container);
    m_containerProxy->setZValue(1);

    // Logo
    m_logo = new GameLogo("assets/images/logo_宝石迷阵.png");
    m_logoProxy = scene->addWidget(m_logo);
    m_logoProxy->setZValue(2);

    mainLayout->addWidget(m_view);
}

void SceneRank::loadRankData() {
    // 检查数据库连接状态
    if (!UserManager::instance().isDatabaseConnected()) {
        // 显示错误信息
        m_table->setRowCount(1);
        QTableWidgetItem* errorItem = new QTableWidgetItem("数据库连接失败");
        errorItem->setTextAlignment(Qt::AlignCenter);
        errorItem->setForeground(QBrush(Qt::red));
        m_table->setItem(0, 0, errorItem);

        // 合并单元格显示完整错误信息
        m_table->setSpan(0, 0, 1, 3);
        return;
    }

    // 获取排行榜数据
    // 这里依然只加载 Top10，实际项目可根据难度参数过滤
    QList<UserData> list = UserManager::instance().getTop10();

    if (list.isEmpty()) {
        // 如果没有数据，显示提示
        m_table->setRowCount(1);
        QTableWidgetItem* emptyItem = new QTableWidgetItem("暂无排行榜数据");
        emptyItem->setTextAlignment(Qt::AlignCenter);
        emptyItem->setForeground(QBrush(Qt::gray));
        m_table->setItem(0, 0, emptyItem);
        m_table->setSpan(0, 0, 1, 3);
        return;
    }

    // 设置表格行数
    m_table->setRowCount(list.size());

    // 使用不同颜色标记前三名
    QList<QColor> rankColors = {
        QColor(255, 215, 0),   // 金牌色
        QColor(192, 192, 192), // 银牌色
        QColor(205, 127, 50),  // 铜牌色
        QColor(255, 255, 255)  // 白色（其他名次）
    };

    for (int i = 0; i < list.size(); ++i) {
        const UserData& user = list[i];

        // 确定颜色
        QColor textColor = (i < 3) ? rankColors[i] : rankColors[3];

        // 排名
        QTableWidgetItem* itemRank = new QTableWidgetItem(QString::number(i + 1));
        itemRank->setTextAlignment(Qt::AlignCenter);
        itemRank->setForeground(QBrush(textColor));

        // 为前三名添加特殊图标或标记
        if (i == 0) {
            itemRank->setIcon(QIcon(":/icons/gold_medal.png")); // 如果有图标资源
        }
        else if (i == 1) {
            itemRank->setIcon(QIcon(":/icons/silver_medal.png"));
        }
        else if (i == 2) {
            itemRank->setIcon(QIcon(":/icons/bronze_medal.png"));
        }

        m_table->setItem(i, 0, itemRank);

        // 用户名
        QTableWidgetItem* itemName = new QTableWidgetItem(user.username);
        itemName->setTextAlignment(Qt::AlignCenter);
        itemName->setForeground(QBrush(textColor));
        m_table->setItem(i, 1, itemName);

        // 分数
        QTableWidgetItem* itemScore = new QTableWidgetItem(QString::number(user.highScore));
        itemScore->setTextAlignment(Qt::AlignCenter);
        itemScore->setForeground(QBrush(textColor));

        // 为高分添加特殊样式
        if (user.highScore >= 1000) {
            itemScore->setFont(QFont("Arial", 14, QFont::Bold));
        }

        // 分数颜色
        itemScore->setForeground(QBrush(QColor("#044BB7")));
        itemScore->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
        m_table->setItem(i, 2, itemScore);
    }

    // 调整列宽，使排名列稍窄
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->setColumnWidth(0, 80);  // 排名列
    m_table->setColumnWidth(1, 300); // 用户名列
    m_table->setColumnWidth(2, 150); // 分数列

    // 添加当前用户高亮显示（如果当前用户在排行榜中）
    QString currentUser = UserManager::instance().getCurrentUser();
    if (!currentUser.isEmpty()) {
        for (int i = 0; i < list.size(); ++i) {
            if (list[i].username == currentUser) {
                // 高亮显示当前用户的行
                for (int col = 0; col < 3; ++col) {
                    QTableWidgetItem* item = m_table->item(i, col);
                    if (item) {
                        item->setBackground(QBrush(QColor(30, 60, 90))); // 深蓝色背景
                        item->setFont(QFont("Arial", 14, QFont::Bold));
                    }
                }
                break;
            }
        }
    }
}

void SceneRank::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (m_videoItem && m_view) {
        m_videoItem->setSize(QSizeF(this->size()));
        m_view->setSceneRect(0, 0, this->width(), this->height());

        if (m_containerProxy) {
            QWidget* w = m_containerProxy->widget();
            if (w) m_containerProxy->setPos((width() - w->width()) / 2, (height() - w->height()) / 2 + 60);
        }
        if (m_logoProxy) {
            QWidget* w = m_logoProxy->widget();
            if (w) m_logoProxy->setPos((width() - w->width()) / 2, height() * 0.05);
        }
    }
}

void SceneRank::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    loadRankData();
    if (m_player) { m_player->setSource(QUrl::fromLocalFile(m_videoPath)); m_player->play(); }
    if (m_logo) m_logo->startEntrance();
}

void SceneRank::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    if (m_player) { m_player->stop(); m_player->setSource(QUrl()); }
}