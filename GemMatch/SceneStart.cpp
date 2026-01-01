#include "SceneStart.h"
#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsScene>
#include <QApplication>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>

SceneStart::SceneStart(MainWindow* mainWin) : QWidget(mainWin), m_mainWin(mainWin) {
    m_currentDifficulty = 0;
    setupUI();
}

void SceneStart::selectDifficulty(int level) {
    m_currentDifficulty = level;
    
    //设置选中高亮
    if (m_btnEasy) m_btnEasy->setSelected(level == 3);
    if (m_btnHard) m_btnHard->setSelected(level == 5);
    if (m_btnExtreme) m_btnExtreme->setSelected(level == 7);
}

void SceneStart::setupUI() {
    //主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_view = new QGraphicsView(this);
    m_view->setStyleSheet("border: none; background: transparent;");
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QGraphicsScene* scene = new QGraphicsScene(this);
    m_view->setScene(scene);

    //视频背景
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

    //中央菜单容器
    QWidget* menuContainer = new QWidget();
    menuContainer->setFixedSize(500, 400); // 调整大小以适应按钮
    menuContainer->setStyleSheet(
        "QWidget {"
        "   background-color: rgba(255, 255, 255, 100);" 
        "   border-radius: 20px;"
        "   border: 1px solid rgba(255, 255, 255, 200);"
        "}"
    );

    QVBoxLayout* menuLayout = new QVBoxLayout(menuContainer);
    menuLayout->setContentsMargins(20, 40, 30, 40);
    menuLayout->setSpacing(20);
    menuLayout->setAlignment(Qt::AlignCenter);

    //三个难度按键
    QHBoxLayout* diffLayout = new QHBoxLayout();
    diffLayout->setSpacing(20);
    m_btnEasy = new GameButton("assets/images/简单1.png");
    m_btnHard = new GameButton("assets/images/困难1.png");
    m_btnExtreme = new GameButton("assets/images/极限1.png");

    diffLayout->addWidget(m_btnEasy);
    diffLayout->addWidget(m_btnHard);
    diffLayout->addWidget(m_btnExtreme);
    menuLayout->addLayout(diffLayout);

    //开始游戏
    m_btnStart = new GameButton("assets/images/开始游戏.png");
    // 如果图片不够大，可以通过 setFixedSize 强制调整，但建议直接用大图
    menuLayout->addWidget(m_btnStart, 0, Qt::AlignCenter);

    //排行榜
    m_btnRank = new GameButton("assets/images/排行榜.png");
    menuLayout->addWidget(m_btnRank, 0, Qt::AlignCenter);


    //个人战绩
    m_btnHistory = new GameButton("assets/images/个人战绩.png");
    menuLayout->addWidget(m_btnHistory, 0, Qt::AlignCenter);

    //添加到场景
    m_menuProxy = scene->addWidget(menuContainer);
    m_menuProxy->setZValue(1);

    //Logo
    m_logo = new GameLogo("assets/images/logo_宝石迷阵.png");
    m_logoProxy = scene->addWidget(m_logo);
    m_logoProxy->setZValue(2);

    //关于
    m_btnAbout = new GameButton("assets/images/关于.png");
    m_aboutProxy = scene->addWidget(m_btnAbout);
    m_aboutProxy->setZValue(2);

    //设置
    m_btnSettings = new GameButton("assets/images/设置.png");
    m_settingProxy = scene->addWidget(m_btnSettings);
    m_settingProxy->setZValue(2);

    //头像
    m_avatar = new QLabel();
    m_avatar->setFixedSize(100, 100);
    m_avatar->setStyleSheet("background: transparent;");
    m_avatarProxy = scene->addWidget(m_avatar);
    m_avatarProxy->setZValue(2);

    //信号连接
    connect(m_btnRank, &QPushButton::clicked, [this]() { m_mainWin->switchPage(5); });
    connect(m_btnAbout, &QPushButton::clicked, [this]() { m_mainWin->switchPage(4); }); 
    connect(m_btnSettings, &QPushButton::clicked, [this]() { m_mainWin->switchPage(3); }); 
    
    //难度按钮,不直接开始游戏
    connect(m_btnEasy, &QPushButton::clicked, [this]() {
        selectDifficulty(3);
        });

    connect(m_btnHard, &QPushButton::clicked, [this]() {
        selectDifficulty(5);
        });

    connect(m_btnExtreme, &QPushButton::clicked, [this]() {
        selectDifficulty(7); 
        });

    connect(m_btnStart, &QPushButton::clicked, [this]() {
        if (m_currentDifficulty == 0) {
            // 如果还没选难度，弹窗提示
            QMessageBox::warning(this, "提示", "请先选择游戏难度！");
            return; // 直接返回，不开始游戏
        }

        // 选了才能进
        m_mainWin->startNewGame(m_currentDifficulty);
        });

    connect(m_btnHistory, &QPushButton::clicked, [this]() {
        if (m_mainWin) {
            m_mainWin->switchPage(7); //切换到个人战绩页面
        }
        });

    mainLayout->addWidget(m_view);
}

void SceneStart::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (m_videoItem && m_view) {
        m_videoItem->setSize(QSizeF(this->size()));
        m_view->setSceneRect(0, 0, this->width(), this->height());

        //中央菜单
        if (m_menuProxy) {
            QWidget* w = m_menuProxy->widget();
            if (w) m_menuProxy->setPos((width() - w->width()) / 2, (height() - w->height()) / 2 + 50);
        }

        //Logo
        if (m_logoProxy) {
            QWidget* w = m_logoProxy->widget();
            if (w) m_logoProxy->setPos((width() - w->width()) / 2, height() * 0.10);
        }
        
        //关于
        if (m_aboutProxy) {
            QWidget* w = m_aboutProxy->widget();
            if (w) m_aboutProxy->setPos(30, height() - w->height() - 30);
        }

        //设置按钮
        if (m_settingProxy) {
            QWidget* w = m_settingProxy->widget();
            if (w) m_settingProxy->setPos(width() - w->width() - 30, height() - w->height() - 30);
        }

        //头像
        if (m_avatarProxy) {
            QWidget* w = m_avatarProxy->widget();
            if (w) m_avatarProxy->setPos( 50, 50);
        }
    }
}

void SceneStart::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);

    selectDifficulty(0);

    //每次进入主菜单时，从 MainWindow 同步头像
    if (m_mainWin && m_avatar) {
        const QPixmap& pix = m_mainWin->getAvatarPixmap();
        if (!pix.isNull()) {
            m_avatar->setPixmap(pix);
        }
    }

    if (m_player) {
        m_player->setSource(QUrl::fromLocalFile(m_videoPath));
        m_player->play();
    }
    if (m_logo) m_logo->startEntrance();
}

void SceneStart::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    if (m_player) {
        m_player->stop();
        m_player->setSource(QUrl());
    }
}