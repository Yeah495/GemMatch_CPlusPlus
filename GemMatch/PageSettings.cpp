#include "PageSettings.h"
#include "MainWindow.h"
#include <QVBoxLayout>
#include <QPushButton>

PageSettings::PageSettings(MainWindow* mainWin) : QWidget(mainWin), m_mainWin(mainWin) {
    setupUI();
}

void PageSettings::setupUI() {
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
    m_videoItem->setZValue(0);
    scene->addItem(m_videoItem);

    m_player->setVideoOutput(m_videoItem);
    m_videoPath = "assets/videos/2.mp4";
    m_player->setLoops(QMediaPlayer::Infinite);

    // ========== 步骤 4: 创建 UI 容器 ==========
    // ❌ 删除原来的 this->setStyleSheet(...)

    QWidget* container = new QWidget();
    container->setFixedSize(600, 700);
    container->setStyleSheet(
        "QWidget { background: rgba(0,0,0,0.8); border-radius: 20px; } "
        "QLabel { color: white; font-size: 18px; font-weight: bold; }"
    );

    // ========== 步骤 5: 构建表单内容（保持原有逻辑）==========
    QVBoxLayout* form = new QVBoxLayout(container);
    form->setContentsMargins(30, 30, 30, 30);
    form->setSpacing(20);

    m_labelTitle = new QLabel("设置 / Settings");
    m_labelTitle->setAlignment(Qt::AlignCenter);
    m_labelTitle->setStyleSheet("font-size: 24px; color: gold;");
    form->addWidget(m_labelTitle);

    // --- 音乐控制 ---
    m_labelMusic = new QLabel("背景音乐音量 (Music Volume):");
    form->addWidget(m_labelMusic);
    QSlider* musicSlider = new QSlider(Qt::Horizontal);
    musicSlider->setRange(0, 100);
    musicSlider->setValue(50);
    form->addWidget(musicSlider);
    connect(musicSlider, &QSlider::valueChanged, [this](int v) {
        // m_mainWin->setGlobalVolume(v); 
        });

    // --- 亮度控制 ---
    m_labelBrightness = new QLabel("屏幕亮度 (Brightness):");
    form->addWidget(m_labelBrightness);
    QSlider* brightSlider = new QSlider(Qt::Horizontal);
    brightSlider->setRange(10, 100);
    brightSlider->setValue(100);
    form->addWidget(brightSlider);
    connect(brightSlider, &QSlider::valueChanged, [this](int v) {
        m_mainWin->setGlobalBrightness(v);
        });

    form->addStretch();

    // 返回按钮
    QPushButton* btnBack = new QPushButton("保存并返回 (Save & Back)");
    btnBack->setStyleSheet(
        "QPushButton { background: gold; color: black; padding: 10px; "
        "border-radius: 5px; font-weight: bold; } "
        "QPushButton:hover { background: white; }"
    );
    connect(btnBack, &QPushButton::clicked, [this]() { m_mainWin->switchPage(0); });
    form->addWidget(btnBack);

    // ========== 步骤 6: 将容器添加到场景 ==========
    QGraphicsProxyWidget* proxy = scene->addWidget(container);
    proxy->setZValue(1);
    proxy->setPos((2560 - 600) / 2, (1600 - 700) / 2);

    // ========== 步骤 7: 添加到主布局 ==========
    mainLayout->addWidget(m_view);
}



void PageSettings::resizeEvent(QResizeEvent* event) {
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



// 【新增】实现 showEvent
void PageSettings::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    if (m_player) {
        m_player->setSource(QUrl::fromLocalFile(m_videoPath));
        m_player->play();
    }
}

// 【新增】实现 hideEvent
void PageSettings::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    if (m_player) {
        m_player->stop();
        m_player->setSource(QUrl()); // ✅ 释放内存
    }
}