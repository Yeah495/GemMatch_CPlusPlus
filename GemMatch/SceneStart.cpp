#include "SceneStart.h"
#include "MainWindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QApplication>
#include <QPainter>
#include <QStyleOption>
#include <QGraphicsProxyWidget>  // ✅ 新增
#include <QUrl>                  // ✅ 新增

SceneStart::SceneStart(MainWindow* mainWin) : QWidget(mainWin), m_mainWin(mainWin) {
    setupUI();
}

void SceneStart::paintEvent(QPaintEvent* event) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void SceneStart::setupUI() {
    // ========== 步骤 1: 创建主布局 ==========
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ========== 步骤 2: 创建 Graphics View ==========
    m_view = new QGraphicsView(this);
    m_view->setStyleSheet("border: none; background: transparent;");
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QGraphicsScene* scene = new QGraphicsScene(this);
    m_view->setScene(scene);

    // ========== 步骤 3: 添加视频层（底层，Z=0）==========
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(0.0f);

    m_videoItem = new QGraphicsVideoItem();
    m_videoItem->setSize(QSizeF(1280, 800));
    m_videoItem->setZValue(0);  // 底层
    scene->addItem(m_videoItem);

    m_player->setVideoOutput(m_videoItem);
    
    m_videoPath = "assets/videos/6.mp4"; // 注意：PageLogin用1.mp4, SceneGame用4.mp4
    m_player->setLoops(QMediaPlayer::Infinite);


    m_player->setLoops(QMediaPlayer::Infinite);
 

    // ========== 步骤 4: 创建 UI 容器（顶层）==========
    QWidget* container = new QWidget();
    container->setFixedSize(600, 700);
    container->setStyleSheet("background: rgba(0,0,0,0.3); border-radius: 20px;");

    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);
    layout->setContentsMargins(40, 40, 40, 40);

    // 标题
    QLabel* title = new QLabel("宝石迷阵");
    title->setStyleSheet("font-size: 60px; font-weight: bold; color: white; margin-bottom: 50px;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    // 通用按钮样式
    QString btnStyle = "QPushButton { font-size: 24px; color: white; background-color: rgba(0,0,0,0.6); "
        "border: 2px solid gold; border-radius: 25px; padding: 10px 40px; min-width: 200px; }"
        "QPushButton:hover { background-color: gold; color: black; }";

    QPushButton* btnStart = new QPushButton("开始游戏");
    btnStart->setStyleSheet(btnStyle);

    QPushButton* btnRank = new QPushButton("排行榜");
    btnRank->setStyleSheet(btnStyle);

    QPushButton* btnSettings = new QPushButton("设置");
    btnSettings->setStyleSheet(btnStyle);

    QPushButton* btnAbout = new QPushButton("关于");
    btnAbout->setStyleSheet(btnStyle);

    QPushButton* btnExit = new QPushButton("退出游戏");
    btnExit->setStyleSheet(btnStyle);

    layout->addWidget(btnStart);
    layout->addWidget(btnRank);
    layout->addWidget(btnSettings);
    layout->addWidget(btnAbout);
    layout->addWidget(btnExit);

    // 导航连接
    connect(btnStart, &QPushButton::clicked, [this]() { m_mainWin->switchPage(2); });
    connect(btnRank, &QPushButton::clicked, [this]() { m_mainWin->switchPage(5); });
    connect(btnSettings, &QPushButton::clicked, [this]() { m_mainWin->switchPage(3); });
    connect(btnAbout, &QPushButton::clicked, [this]() { m_mainWin->switchPage(4); });
    connect(btnExit, &QPushButton::clicked, []() { QApplication::quit(); });

    // ========== 步骤 5: 将容器添加到场景（Z=1，在视频上方）==========
    QGraphicsProxyWidget* proxy = scene->addWidget(container);
    proxy->setZValue(1);  // 顶层
    proxy->setPos((2560 - 600) / 2, (1600 - 700) / 2);  // 居中

    // ========== 步骤 6: 添加到主布局 ==========
    mainLayout->addWidget(m_view);
}

void SceneStart::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

    if (m_videoItem && m_view) {
        m_videoItem->setSize(QSizeF(this->size()));
        m_view->setSceneRect(0, 0, this->width(), this->height());

        QList<QGraphicsItem*> items = m_view->scene()->items();
        for (auto* item : items) {
            if (QGraphicsProxyWidget* proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(item)) {
                QWidget* widget = proxy->widget();
                if (widget) {
                    proxy->setPos((this->width() - widget->width()) / 2,
                        (this->height() - widget->height()) / 2);
                }
            }
        }
    }
}

void SceneStart::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    // 页面显示时，开始播放
    if (m_player) {
        m_player->setSource(QUrl::fromLocalFile(m_videoPath)); // ✅ 此时才加载进内存
        m_player->play();
    }
}

void SceneStart::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    // 页面隐藏时，暂停播放以释放CPU/GPU资源
    if (m_player) {
        m_player->stop();
        m_player->setSource(QUrl()); // ✅ 关键！设为空，强制释放视频占用的内存

    }
}