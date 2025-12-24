#include "PageSettings.h"
#include "MainWindow.h"
#include <QVBoxLayout>
#include <QGraphicsScene>

PageSettings::PageSettings(MainWindow* mainWin)
    : QWidget(mainWin)
    , m_mainWin(mainWin)
    , m_boxProxy(nullptr)   // ✅ 初始化为空指针
    , m_logoProxy(nullptr)  // ✅ 初始化为空指针
    , m_player(nullptr)
    , m_logo(nullptr)
{
    setupUI();
}

void PageSettings::setupUI() {
    // 1. 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 2. Graphics View & Scene
    m_view = new QGraphicsView(this);
    m_view->setStyleSheet("border: none; background: transparent;");
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QGraphicsScene* scene = new QGraphicsScene(this);
    m_view->setScene(scene);

    // 3. 视频背景 (Z=0)
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(0.0f);

    m_videoItem = new QGraphicsVideoItem();
    m_videoItem->setSize(QSizeF(1280, 800));
    m_videoItem->setZValue(0);
    scene->addItem(m_videoItem);

    m_player->setVideoOutput(m_videoItem);
    m_videoPath = "assets/videos/2.mp4"; // 保持原视频
    m_player->setLoops(QMediaPlayer::Infinite);

    // =========================================================
    // 4. 设置框容器 (Z=1) —— 模仿 PageLogin 的毛玻璃风格
    // =========================================================
    QWidget* container = new QWidget();
    container->setFixedSize(500, 450); // 调整大小
    container->setStyleSheet(
        "QWidget {"
        "   background-color: rgba(255, 255, 255, 100);" /* 白色半透明 */
        "   border-radius: 20px;"
        "   border: 1px solid rgba(255, 255, 255, 200);"
        "}"
        "QLabel {"
        "   color: #044BB7;" /* 深蓝灰色字体，适应浅色背景 */
        "   font-size: 25px;"
        "   font-weight: bold;"
        "   background: transparent;"
        "}"
    );

    QVBoxLayout* form = new QVBoxLayout(container);
    form->setContentsMargins(40, 40, 40, 40);
    form->setSpacing(25);
    form->setAlignment(Qt::AlignTop);

    // --- 内容控件 ---
    // 标题 (可选，如果有了Logo，这里可以写"系统设置")
    m_labelTitle = new QLabel("系统设置");
    m_labelTitle->setAlignment(Qt::AlignCenter);
    m_labelTitle->setStyleSheet("font-size: 30px; color: #044BB7; margin-bottom: 10px;");
    form->addWidget(m_labelTitle);

    // 音乐滑块
    m_labelMusic = new QLabel("音乐音量");
    form->addWidget(m_labelMusic);
    QSlider* musicSlider = new QSlider(Qt::Horizontal);
    musicSlider->setRange(0, 100);
    musicSlider->setValue(50);
    // 美化滑块 (可选)
    musicSlider->setStyleSheet(
        "QSlider::groove:horizontal { height: 8px; background: rgba(0,0,0,0.2); border-radius: 4px; }"
        "QSlider::handle:horizontal { background: #00BFFF; width: 20px; margin: -6px 0; border-radius: 10px; }"
    );
    form->addWidget(musicSlider);

    // 亮度滑块
    m_labelBrightness = new QLabel("屏幕亮度");
    form->addWidget(m_labelBrightness);
    QSlider* brightSlider = new QSlider(Qt::Horizontal);
    brightSlider->setRange(10, 100);
    brightSlider->setValue(100);
    brightSlider->setStyleSheet(musicSlider->styleSheet());
    form->addWidget(brightSlider);
    connect(brightSlider, &QSlider::valueChanged, [this](int v) {
        m_mainWin->setGlobalBrightness(v);
        });

    form->addStretch();

    // 1. 创建“返回”按钮 (图片按钮)
        // 请确保 assets/images/ 下有对应的图片，如果没有请更换为实际路径
    m_btnBack = new GameButton("assets/images/返回主菜单.png");
    connect(m_btnBack, &QPushButton::clicked, [this]() {
        m_mainWin->switchPage(1); // 返回主菜单 (Index 1)
        });

    // 2. 创建“重新登录”按钮 (图片按钮)
    m_btnReLogin = new GameButton("assets/images/重新登录.png");
    connect(m_btnReLogin, &QPushButton::clicked, [this]() {
        m_mainWin->switchPage(0); // 返回登录页 (Index 0)
        });

    // 3. 添加到布局 (垂直布局会自动上下排列)
    // Qt::AlignHCenter 确保按钮水平居中
    form->addWidget(m_btnBack, 0, Qt::AlignHCenter);

    // 添加一点间距
    form->addSpacing(15);

    form->addWidget(m_btnReLogin, 0, Qt::AlignHCenter);

    // 底部再加一点弹簧，让按钮不要死贴着底边
    form->addSpacing(20);

    m_boxProxy = scene->addWidget(container); // <--- 加上这一行！
    m_boxProxy->setZValue(1);                 // 确保它在视频之上，Logo之下

    // =========================================================
    // 5. Logo (Z=2)
    // =========================================================
    // 这里可以使用 "logo_settings.png" 或者复用主 Logo
    m_logo = new GameLogo("assets/images/logo_宝石迷阵.png");
    m_logoProxy = scene->addWidget(m_logo);
    m_logoProxy->setZValue(2); // 浮在上方

    // 6. 完成布局
    mainLayout->addWidget(m_view);
}

void PageSettings::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

    if (m_videoItem && m_view) {
        m_videoItem->setSize(QSizeF(this->size()));
        m_view->setSceneRect(0, 0, this->width(), this->height());

        // 1. 定位设置框：屏幕中心稍偏下
        if (m_boxProxy) {
            QWidget* widget = m_boxProxy->widget();
            if (widget) {
                m_boxProxy->setPos((this->width() - widget->width()) / 2,
                    (this->height() - widget->height()) / 2 + 60);
            }
        }

        // 2. 定位 Logo：屏幕中心偏上
        if (m_logoProxy) {
            QWidget* widget = m_logoProxy->widget();
            if (widget) {
                m_logoProxy->setPos((this->width() - widget->width()) / 2,
                    this->height() * 0.05);
            }
        }
    }
}

void PageSettings::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    if (m_player) {
        m_player->setSource(QUrl::fromLocalFile(m_videoPath));
        m_player->play();
    }
    // ✅ 触发 Logo 动画
    if (m_logo) {
        m_logo->startEntrance();
    }
}

void PageSettings::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    if (m_player) {
        m_player->stop();
        m_player->setSource(QUrl());
    }
}