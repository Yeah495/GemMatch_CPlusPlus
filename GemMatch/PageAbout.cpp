#include "PageAbout.h"
#include "MainWindow.h"
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QPushButton>
#include <QLabel>                    // ✅ 新增
#include <QGraphicsProxyWidget>      // ✅ 新增
#include <QUrl>                      // ✅ 新增：QUrl::fromLocalFile 需要

PageAbout::PageAbout(MainWindow* mainWin) : QWidget(mainWin), m_mainWin(mainWin) {
    setupUI();
}
void PageAbout::setupUI() {
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
    m_videoPath = "assets/videos/2.mp4";
    m_player->setLoops(QMediaPlayer::Infinite);

    // ========== 步骤 4: 创建 UI 容器（顶层）==========
    // ❌ 删除原来的 this->setStyleSheet(...) 背景设置

    QWidget* container = new QWidget();  // 注意：不传父对象
    container->setFixedSize(400, 450);  // 可根据需要调整大小
    container->setStyleSheet(
        "QWidget { background: rgba(0,0,0,0.8); border: 2px solid #555; border-radius: 10px; padding: 20px; }"
    );

    // ========== 步骤 5: 构建容器内容（保持原有布局逻辑）==========
    QVBoxLayout* contentLayout = new QVBoxLayout(container);
    contentLayout->setContentsMargins(50, 40, 50, 40);
    contentLayout->setSpacing(20);

    // 标题
    QLabel* title = new QLabel("宝石迷阵");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color: gold; font-size: 36px; font-weight: bold;");

    QLabel* version = new QLabel("Version 1.0");
    version->setAlignment(Qt::AlignCenter);
    version->setStyleSheet("color: white; font-size: 24px;");

    // 内容区域（用QTextBrowser）
    QTextBrowser* browser = new QTextBrowser();
    browser->setHtml(
        "<h3 style='color:white; text-align:center;'>开发团队</h3>"
        "<p style='color:#ccc; font-size:18px; text-align:center;'>"
        "开发者：张涵玮、刘智童、周秉龙、闵世宇<br><br>"
        "感谢您的游玩！"
        "</p>"
    );
    browser->setStyleSheet(
        "background: rgba(255,255,255,0.05); border: 1px solid #444; "
        "border-radius: 5px; padding: 15px; color: white;"
    );

    // 返回按钮
    QPushButton* btnBack = new QPushButton("返回主菜单 (BACK)");
    btnBack->setStyleSheet(
        "QPushButton { font-size: 18px; padding: 12px; background: gold; "
        "color: black; border-radius: 8px; font-weight: bold; } "
        "QPushButton:hover { background: white; }"
    );
    connect(btnBack, &QPushButton::clicked, [this]() { m_mainWin->switchPage(1); });

    // 添加到容器布局
    contentLayout->addWidget(title);
    contentLayout->addWidget(version);
    contentLayout->addWidget(browser);
    contentLayout->addStretch();
    contentLayout->addWidget(btnBack);

    // ========== 步骤 6: 将容器添加到场景（Z=1，在视频上方）==========
    QGraphicsProxyWidget* proxy = scene->addWidget(container);
    proxy->setZValue(1);  // 顶层
    proxy->setPos((2560 - 800) / 2, (1600 - 900) / 2);  // 居中

    // ========== 步骤 7: 添加到主布局 ==========
    mainLayout->addWidget(m_view);
}

void PageAbout::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

    if (m_videoItem && m_view) {
        // 调整视频大小
        m_videoItem->setSize(QSizeF(this->size()));
        m_view->setSceneRect(0, 0, this->width(), this->height());

        // 重新居中 UI 容器
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
void PageAbout::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    if (m_player) {
        m_player->setSource(QUrl::fromLocalFile(m_videoPath));
        m_player->play();
    }
}

// 【新增】实现 hideEvent
void PageAbout::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    if (m_player) {
        m_player->stop();
        m_player->setSource(QUrl()); // ✅ 释放内存
    }
}