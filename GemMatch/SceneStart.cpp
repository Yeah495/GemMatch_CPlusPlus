#include "SceneStart.h"
#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsScene>
#include <QApplication>
#include <QPainterPath> // 需要引入
#include <QGraphicsDropShadowEffect>

SceneStart::SceneStart(MainWindow* mainWin) : QWidget(mainWin), m_mainWin(mainWin) {
    m_currentDifficulty = 0;
    setupUI();
}

void SceneStart::selectDifficulty(int level) {
    m_currentDifficulty = level;

    // 如果 level 是 0，所有判断都会是 false，所有按钮都会取消高亮
    if (m_btnEasy) m_btnEasy->setSelected(level == 3);
    if (m_btnHard) m_btnHard->setSelected(level == 5);
    if (m_btnExtreme) m_btnExtreme->setSelected(level == 7);
}

void SceneStart::setupUI() {
    // 1. 主布局与视图初始化
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_view = new QGraphicsView(this);
    m_view->setStyleSheet("border: none; background: transparent;");
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QGraphicsScene* scene = new QGraphicsScene(this);
    m_view->setScene(scene);

    // 2. 视频背景
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

    // =========================================================
    // 3. 中央菜单容器 (白色磨砂)
    // =========================================================
    QWidget* menuContainer = new QWidget();
    menuContainer->setFixedSize(500, 400); // 调整大小以适应按钮
    menuContainer->setStyleSheet(
        "QWidget {"
        "   background-color: rgba(255, 255, 255, 100);" /* 白色半透明 */
        "   border-radius: 20px;"
        "   border: 1px solid rgba(255, 255, 255, 200);"
        "}"
    );

    QVBoxLayout* menuLayout = new QVBoxLayout(menuContainer);
    menuLayout->setContentsMargins(20, 40, 30, 40);
    menuLayout->setSpacing(20);
    menuLayout->setAlignment(Qt::AlignCenter);

    // -- 第一行：三个难度按键 --
    QHBoxLayout* diffLayout = new QHBoxLayout();
    diffLayout->setSpacing(20);
    // 请确保有对应图片，没有则用文字图或临时图代替
    m_btnEasy = new GameButton("assets/images/简单1.png");
    m_btnHard = new GameButton("assets/images/困难1.png");
    m_btnExtreme = new GameButton("assets/images/极限1.png");

    diffLayout->addWidget(m_btnEasy);
    diffLayout->addWidget(m_btnHard);
    diffLayout->addWidget(m_btnExtreme);
    menuLayout->addLayout(diffLayout);

    // -- 第二行：开始游戏 (大) --
    m_btnStart = new GameButton("assets/images/开始游戏.png");
    // 如果图片不够大，可以通过 setFixedSize 强制调整，但建议直接用大图
    menuLayout->addWidget(m_btnStart, 0, Qt::AlignCenter);

    // -- 第三行：排行榜 --
    m_btnRank = new GameButton("assets/images/排行榜.png");
    menuLayout->addWidget(m_btnRank, 0, Qt::AlignCenter);


    // 【新增】 -- 第四行：个人战绩 --
    m_btnHistory = new GameButton("assets/images/个人战绩.png");
    menuLayout->addWidget(m_btnHistory, 0, Qt::AlignCenter);

    // 添加到场景
    m_menuProxy = scene->addWidget(menuContainer);
    m_menuProxy->setZValue(1);

    // =========================================================
    // 4. Logo (独立，位于菜单上方)
    // =========================================================
    m_logo = new GameLogo("assets/images/logo_宝石迷阵.png");
    m_logoProxy = scene->addWidget(m_logo);
    m_logoProxy->setZValue(2);

    // =========================================================
    // 5. 角落按钮 (独立)
    // =========================================================
    // 左上：关于
    m_btnAbout = new GameButton("assets/images/关于.png");
    m_aboutProxy = scene->addWidget(m_btnAbout);
    m_aboutProxy->setZValue(2);

    // 右上：设置
    m_btnSettings = new GameButton("assets/images/设置.png");
    m_settingProxy = scene->addWidget(m_btnSettings);
    m_settingProxy->setZValue(2);



    // =========================================================
        // 【修改】 6. 右上角圆形头像 (带白色描边优化)
        // =========================================================
    m_avatar = new QLabel();
    m_avatar->setFixedSize(100, 100); // 稍微调小一点，显得更精致
    m_avatar->setStyleSheet("background: transparent;");

    // 处理圆形图片 + 描边
    QPixmap rawPix("assets/images/1.png"); // 读取你的头像图片
    if (!rawPix.isNull()) {
        // 创建一个透明画布
        QPixmap circularPix(100, 100);
        circularPix.fill(Qt::transparent);

        QPainter painter(&circularPix);
        painter.setRenderHint(QPainter::Antialiasing);           // 抗锯齿
        painter.setRenderHint(QPainter::SmoothPixmapTransform);  // 平滑变换

        // 1. 绘制圆形路径（作为裁剪区域）
        QPainterPath path;
        // 留出 2px 边距给描边，防止边缘被切掉
        path.addEllipse(2, 2, 96, 96);
        painter.setClipPath(path);

        // 2. 绘制图片
        painter.drawPixmap(0, 0, 100, 100, rawPix);

        // 3. 绘制白色描边 (重置裁剪，否则描边会被切掉一半)
        painter.setClipping(false);
        QPen pen(Qt::white, 4); // 白色，4像素宽度
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(2, 2, 96, 96); // 这里的圆要和上面的路径一致

        m_avatar->setPixmap(circularPix);
    }

    m_avatarProxy = scene->addWidget(m_avatar);
    m_avatarProxy->setZValue(2);

    // --- 信号连接 ---
    connect(m_btnRank, &QPushButton::clicked, [this]() { m_mainWin->switchPage(5); }); // 假设 Rank 是 Page 5
    connect(m_btnAbout, &QPushButton::clicked, [this]() { m_mainWin->switchPage(4); }); // 假设 About 是 Page 4
    connect(m_btnSettings, &QPushButton::clicked, [this]() { m_mainWin->switchPage(3); }); // 假设 Settings 是 Page 3
    
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
            m_mainWin->switchPage(7); // 切换到PageStatistics页面
        }
        });

    mainLayout->addWidget(m_view);
}

void SceneStart::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (m_videoItem && m_view) {
        m_videoItem->setSize(QSizeF(this->size()));
        m_view->setSceneRect(0, 0, this->width(), this->height());

        // 1. 中央菜单居中偏下
        if (m_menuProxy) {
            QWidget* w = m_menuProxy->widget();
            if (w) m_menuProxy->setPos((width() - w->width()) / 2, (height() - w->height()) / 2 + 50);
        }

        // 2. Logo 居中偏上
        if (m_logoProxy) {
            QWidget* w = m_logoProxy->widget();
            if (w) m_logoProxy->setPos((width() - w->width()) / 2, height() * 0.10);
        }

        if (m_aboutProxy) {
            QWidget* w = m_aboutProxy->widget();
            if (w) m_aboutProxy->setPos(30, height() - w->height() - 30);
        }

        // 【修改】 4. 设置按钮 -> 移到右下角
        if (m_settingProxy) {
            QWidget* w = m_settingProxy->widget();
            if (w) m_settingProxy->setPos(width() - w->width() - 30, height() - w->height() - 30);
        }

        // 【新增】 5. 头像 -> 移到右上角
        if (m_avatarProxy) {
            QWidget* w = m_avatarProxy->widget();
            // 距离右边30，距离顶部30
            if (w) m_avatarProxy->setPos( 50, 50);
        }
    }
}

//只要切换到这个页面,就会自动调用一次showEvent
void SceneStart::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);

    selectDifficulty(0);

    if (m_player) {
        m_player->setSource(QUrl::fromLocalFile(m_videoPath));
        m_player->play();
    }
    if (m_logo) m_logo->startEntrance();
}

//只要离开这个页面,就会自动调用一次hideEvent
void SceneStart::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    if (m_player) {
        m_player->stop();
        m_player->setSource(QUrl());
    }
}