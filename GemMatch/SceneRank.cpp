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

    // 容器 
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
    m_btnEasy = new GameButton("assets/images/简单1.png");
    m_btnHard = new GameButton("assets/images/困难1.png");
    m_btnExtreme = new GameButton("assets/images/极限1.png");
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
    m_table->setShowGrid(true); 

    // 表格样式
    m_table->setStyleSheet(
        "QTableWidget {"
        "   background: rgba(255, 255, 255, 180);" 
        "   gridline-color: #aaa;" 
        "   color: #333;" 
        "   font-size: 16px;"
        "   border: 1px solid #ccc;"
        "   border-radius: 10px;"
        "}"
        "QHeaderView::section {"
        "   background: #044BB7;"
        "   color: white;"
        "   padding: 8px;"
        "   font-weight: bold;"
        "   border: 1px solid #033E8C;"
        "   border-radius: 5px;"
        "}"
        "QTableWidget::item { "
        "   padding: 5px; "
        "   border-bottom: 1px solid #ddd;"
        "}"
    );
    layout->addWidget(m_table);

    // 返回按钮
    m_btnBack = new GameButton("assets/images/返回主菜单.png");
    QHBoxLayout* backLayout = new QHBoxLayout();
    backLayout->addStretch();
    backLayout->addWidget(m_btnBack);
    backLayout->addStretch();
    layout->addLayout(backLayout);

    connect(m_btnBack, &QPushButton::clicked, [this]() { m_mainWin->switchPage(1); });

    // 添加难度参数
    connect(m_btnEasy, &QPushButton::clicked, [this]() {
        qDebug() << "点击简单难度按钮";
        loadRankData(3);
        });
    connect(m_btnHard, &QPushButton::clicked, [this]() {
        qDebug() << "点击普通难度按钮";
        loadRankData(5);
        });
    connect(m_btnExtreme, &QPushButton::clicked, [this]() {
        qDebug() << "点击困难难度按钮";
        loadRankData(7);
        });

    m_containerProxy = scene->addWidget(container);
    m_containerProxy->setZValue(1);

    // Logo
    m_logo = new GameLogo("assets/images/logo_宝石迷阵.png");
    m_logoProxy = scene->addWidget(m_logo);
    m_logoProxy->setZValue(2);

    mainLayout->addWidget(m_view);
}

// 加载排行榜数据
void SceneRank::loadRankData(int difficultyLevel) {
    qDebug() << "正在加载排行榜数据，难度：" << difficultyLevel;

    m_table->clear(); 
    m_table->setRowCount(0);
    m_table->setColumnCount(3);
    m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); 

    QString difficultyName;
    switch (difficultyLevel) {
    case 3: difficultyName = "简单难度"; break;
    case 5: difficultyName = "普通难度"; break;
    case 7: difficultyName = "困难难度"; break;
    default: difficultyName = "未知难度";
    }

    m_table->setHorizontalHeaderLabels({ "排名", "用户", difficultyName + "最高分" });

    // 检查数据库连接状态
    if (!UserManager::instance().isDatabaseConnected()) {
        qDebug() << "数据库连接失败！";

        m_table->setRowCount(1);
        m_table->setColumnCount(3);

        QTableWidgetItem* errorItem = new QTableWidgetItem("数据库连接失败");
        errorItem->setTextAlignment(Qt::AlignCenter);
        errorItem->setForeground(QBrush(Qt::red));

        m_table->setItem(0, 0, errorItem);
        m_table->setSpan(0, 0, 1, 3); 

        return;
    }

    // 获取指定难度的排行榜数据
    QList<UserData> list = UserManager::instance().getRanking(difficultyLevel);
    qDebug() << "获取到" << list.size() << "条排行榜数据";

    if (list.isEmpty()) {
        qDebug() << "排行榜数据为空";

        m_table->setRowCount(1);
        m_table->setColumnCount(3);

        QTableWidgetItem* emptyItem = new QTableWidgetItem("暂无" + difficultyName + "排行榜数据");
        emptyItem->setTextAlignment(Qt::AlignCenter);
        emptyItem->setForeground(QBrush(Qt::gray));

        m_table->setItem(0, 0, emptyItem);
        m_table->setSpan(0, 0, 1, 3);

        return;
    }

    m_table->setRowCount(list.size());
    //填充数据
    qDebug() << "开始填充表格数据...";
    for (int i = 0; i < list.size(); ++i) {
        const UserData& user = list[i];
        qDebug() << "第" << i + 1 << "名: 用户=" << user.username
            << ", 分数=" << (difficultyLevel == 3 ? user.easyHighScore :
                difficultyLevel == 5 ? user.normalHighScore :
                user.hardHighScore);

        QColor textColor = QColor("#044BB7");
        //排名
        QTableWidgetItem* itemRank = new QTableWidgetItem(QString::number(i + 1));
        itemRank->setTextAlignment(Qt::AlignCenter);
        itemRank->setForeground(QBrush(textColor));
        itemRank->setFlags(itemRank->flags() & ~Qt::ItemIsEditable);

        itemRank->setFont(QFont("Microsoft YaHei", 11, QFont::Bold));

        m_table->setItem(i, 0, itemRank);

        // 用户名
        QTableWidgetItem* itemName = new QTableWidgetItem(user.username);
        itemName->setTextAlignment(Qt::AlignCenter);
        itemName->setForeground(QBrush(textColor));
        itemName->setFlags(itemName->flags() & ~Qt::ItemIsEditable);

        itemName->setFont(QFont("Microsoft YaHei", 11, QFont::Bold));
        m_table->setItem(i, 1, itemName);


        // 分数
        int score = 0;
        switch (difficultyLevel) {
        case 3: score = user.easyHighScore; break;
        case 5: score = user.normalHighScore; break;
        case 7: score = user.hardHighScore; break;
        }

        QTableWidgetItem* itemScore = new QTableWidgetItem(QString::number(score));
        itemScore->setTextAlignment(Qt::AlignCenter);

        itemScore->setForeground(QBrush(QColor("#044BB7"))); 
        itemScore->setFont(QFont("Microsoft YaHei", 11, QFont::Bold)); 

        itemScore->setFlags(itemScore->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 2, itemScore);
    }

    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->setColumnWidth(0, 100);  
    m_table->setColumnWidth(1, 250);  
    m_table->setColumnWidth(2, 200);  

    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    // 添加当前用户高亮显示
    QString currentUser = UserManager::instance().getCurrentUser();
    qDebug() << "当前用户：" << currentUser;
    if (!currentUser.isEmpty()) {
        bool foundCurrentUser = false;
        for (int i = 0; i < list.size(); ++i) {
            if (list[i].username == currentUser) {
                qDebug() << "找到当前用户，高亮显示第" << i + 1 << "行";
                foundCurrentUser = true;

                for (int col = 0; col < 3; ++col) {
                    QTableWidgetItem* item = m_table->item(i, col);
                    if (item) {
                        item->setBackground(QBrush(QColor(173, 216, 230, 150)));
                        item->setFont(QFont("Arial", 12, QFont::Bold));
                        item->setForeground(QBrush(QColor(0, 0, 139)));
                    }
                }
                break;
            }
        }

        if (!foundCurrentUser) {
            qDebug() << "当前用户不在排行榜前10名中";
        }
    }

    // 更新表格显示
    m_table->update();
    qDebug() << "表格数据填充完成，共" << m_table->rowCount() << "行";
}


// 调整排行榜大小
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

//显示排行榜
void SceneRank::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    loadRankData(3);
    if (m_player) { m_player->setSource(QUrl::fromLocalFile(m_videoPath)); m_player->play(); }
    if (m_logo) m_logo->startEntrance();
}


// 隐藏排行榜
void SceneRank::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    if (m_player) { m_player->stop(); m_player->setSource(QUrl()); }
}