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
    // 这里依然只加载 Top10，实际项目可根据难度参数过滤
    QList<UserData> list = UserManager::instance().getTop10();
    m_table->setRowCount(list.size());
    for (int i = 0; i < list.size(); ++i) {
        QTableWidgetItem* itemRank = new QTableWidgetItem(QString::number(i + 1));
        itemRank->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(i, 0, itemRank);

        QTableWidgetItem* itemName = new QTableWidgetItem(list[i].username);
        itemName->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(i, 1, itemName);

        QTableWidgetItem* itemScore = new QTableWidgetItem(QString::number(list[i].highScore));
        itemScore->setTextAlignment(Qt::AlignCenter);
        // 分数颜色
        itemScore->setForeground(QBrush(QColor("#044BB7")));
        itemScore->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
        m_table->setItem(i, 2, itemScore);
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