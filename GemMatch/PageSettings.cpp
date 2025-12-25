#include "PageSettings.h"
#include "MainWindow.h"
#include <QVBoxLayout>
#include <QGraphicsScene>
#include <QComboBox>
#include <QDebug>
#include <QEvent>

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
    container->setFixedSize(500, 600); // 调整大小
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
    form->setContentsMargins(40, 30, 40, 50);
    form->setSpacing(20);
    form->setAlignment(Qt::AlignTop);



    // =================================================
        // 1. 头像显示 (可点击)
        // =================================================
    m_btnAvatar = new QPushButton();
    m_btnAvatar->setFixedSize(100, 100);
    m_btnAvatar->setStyleSheet("border: none; background: transparent;"); // 去掉默认按钮样式

    // 生成圆形头像 (同 SceneStart 逻辑)
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

        m_btnAvatar->setIcon(QIcon(circularPix));
        m_btnAvatar->setIconSize(QSize(100, 100));
    }
    // 连接点击信号
    connect(m_btnAvatar, &QPushButton::clicked, this, &PageSettings::onChangeAvatar);

    // 将头像居中添加到布局
    form->addWidget(m_btnAvatar, 0, Qt::AlignHCenter);

    // =================================================
    // 2. 标题
    // =================================================
    m_labelTitle = new QLabel("系统设置");
    m_labelTitle->setAlignment(Qt::AlignCenter);
    m_labelTitle->setStyleSheet("font-size: 26px; color: #044BB7; font-weight: bold;");
    form->addWidget(m_labelTitle);

    // ---------------------------------------------------------
        // 【模块 2】语言切换 (改为按钮)
        // ---------------------------------------------------------
    QHBoxLayout* langLayout = new QHBoxLayout();
    QLabel* lblLang = new QLabel("中英文切换");

    m_btnLang = new QPushButton("简体中文");
    m_btnLang->setFixedSize(140, 40);
    m_btnLang->setCursor(Qt::PointingHandCursor);
    // 美化按钮样式：蓝色边框，白色背景，鼠标悬停变色
    m_btnLang->setStyleSheet(
        "QPushButton {"
        "   border: 2px solid #00BFFF;"
        "   border-radius: 10px;"
        "   background-color: rgba(255, 255, 255, 0.8);"
        "   color: #044BB7;"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #00BFFF;"
        "   color: white;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #009ACD;"
        "}"
    );
    // 连接切换信号
    connect(m_btnLang, &QPushButton::clicked, this, &PageSettings::onToggleLanguage);

    langLayout->addWidget(lblLang);
    langLayout->addWidget(m_btnLang);
    form->addLayout(langLayout);

    // =================================================
    // 4. 音量与亮度 (原有代码)
    // =================================================
    m_labelMusic = new QLabel("音乐音量");
    form->addWidget(m_labelMusic);
    m_musicSlider = new QSlider(Qt::Horizontal); // 修改为成员变量
    m_musicSlider->setRange(0, 100);
    m_musicSlider->setValue(50);
    m_musicSlider->setTracking(true); // 确保拖动时持续发送valueChanged
    m_musicSlider->setEnabled(true);
    m_musicSlider->setStyleSheet(
        "QSlider::groove:horizontal { height: 8px; background: rgba(0,0,0,0.2); border-radius: 4px; }"
        "QSlider::handle:horizontal { background: #00BFFF; width: 20px; margin: -6px 0; border-radius: 10px; }"
    );
    qDebug() << "[Settings] musicSlider created:" << m_musicSlider;
    // 移除调试用的eventFilter
    // m_musicSlider->installEventFilter(this); 
    form->addWidget(m_musicSlider);
    // 连接音量滑块信号到槽函数
    connect(m_musicSlider, &QSlider::valueChanged, this, &PageSettings::onMusicVolumeChanged);
    // 直接联动：拖动过程中实时设置BGM音量
    connect(m_musicSlider, &QSlider::sliderMoved, this, [this](int v){
        if (m_mainWin) {
            float vol = qBound(0, v, 100) / 100.0f;
            m_mainWin->setBGMVolume(vol);
        }
    });
    connect(m_musicSlider, &QSlider::sliderReleased, this, [this](){
        if (m_mainWin) {
            float vol = qBound(0, m_musicSlider->value(), 100) / 100.0f;
            m_mainWin->setBGMVolume(vol);
        }
    });

    m_labelBrightness = new QLabel("屏幕亮度");
    form->addWidget(m_labelBrightness);
    QSlider* brightSlider = new QSlider(Qt::Horizontal);
    brightSlider->setRange(10, 100);
    brightSlider->setValue(100);
    brightSlider->setStyleSheet(m_musicSlider->styleSheet());
    form->addWidget(brightSlider);
    connect(brightSlider, &QSlider::valueChanged, [this](int v) {
        m_mainWin->setGlobalBrightness(v);
        });

    form->addStretch();

    // =================================================
    // 5. 底部按钮
    // =================================================
    QVBoxLayout* bottomLayout = new QVBoxLayout();
    m_btnBack = new GameButton("assets/images/返回主菜单.png");
    m_btnReLogin = new GameButton("assets/images/重新登录.png");



    connect(m_btnBack, &QPushButton::clicked, [this]() { m_mainWin->switchPage(1); });
    connect(m_btnReLogin, &QPushButton::clicked, [this]() { m_mainWin->switchPage(0); });

    bottomLayout->addWidget(m_btnReLogin,0,Qt::AlignHCenter);
    bottomLayout->addSpacing(10);
    bottomLayout->addWidget(m_btnBack, 0, Qt::AlignHCenter);
    form->addLayout(bottomLayout);

    // 添加到 Scene
    m_boxProxy = scene->addWidget(container);
    m_boxProxy->setZValue(1);

    // Logo 
    m_logo = new GameLogo("assets/images/logo_宝石迷阵.png");
    m_logoProxy = scene->addWidget(m_logo);
    m_logoProxy->setZValue(2);

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
    // ✅ 同步滑块到当前BGM音量（0.0~1.0 -> 0~100）
    if (m_mainWin && m_musicSlider) {
        float vol = m_mainWin->getBGMVolume();
        int sliderVal = static_cast<int>(vol * 100.0f + 0.5f);
        m_musicSlider->blockSignals(true); // 避免触发回调
        m_musicSlider->setValue(sliderVal);
        m_musicSlider->blockSignals(false);
        qDebug() << "[Settings] sync slider to BGM:" << vol << "->" << sliderVal;
    }
}

void PageSettings::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    if (m_player) {
        m_player->stop();
        m_player->setSource(QUrl());
    }
}



// 实现空函数
void PageSettings::onChangeAvatar() {
    // TODO: 后续实现选择图片逻辑
    qDebug() << "点击了更换头像";
}

// 切换语言的逻辑
void PageSettings::onToggleLanguage() {
    // 1. 获取当前按钮文字，判断状态
    QString currentTxt = m_btnLang->text();

    if (currentTxt == "简体中文") {
        // 切换到英文状态
        m_btnLang->setText("English");
        // TODO: 这里可以调用 MainWindow 的翻译函数，或者直接发信号
        // m_mainWin->toggleLanguage(); 
        qDebug() << "Language switched to English";
    }
    else {
        // 切换回中文状态
        m_btnLang->setText("简体中文");
        // m_mainWin->toggleLanguage(); 
        qDebug() << "Language switched to Chinese";
    }

    //// 调用主窗口的切换接口（如果有的话）
    //if (m_mainWin) {
    //    m_mainWin->toggleLanguage();
    //}
}

// 新增：音量滑块槽函数实现
void PageSettings::onMusicVolumeChanged(int value) {
    qDebug() << "[Settings] slider value:" << value;
    if (m_mainWin) {
        float volume = qBound(0, value, 100) / 100.0f;
        qDebug() << "[Settings] mapped volume:" << volume;
        m_mainWin->setBGMVolume(volume);
    } else {
        qDebug() << "[Settings] m_mainWin is null";
    }
}