#include "PageAbout.h"
#include "MainWindow.h"
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QGraphicsScene>

PageAbout::PageAbout(MainWindow* mainWin) : QWidget(mainWin), m_mainWin(mainWin) {
    setupUI();
}

void PageAbout::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 1. 视图初始化
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
    m_videoPath = "assets/videos/2.mp4";
    m_player->setLoops(QMediaPlayer::Infinite);

    // =========================================================
    // 3. 内容容器 (Z=1) —— 白色毛玻璃风格
    // =========================================================
    QWidget* container = new QWidget();
    container->setFixedSize(600, 450);
    container->setStyleSheet(
        "QWidget {"
        "   background-color: rgba(255, 255, 255, 100);" /* 白色半透明 */
        "   border-radius: 20px;"
        "   border: 1px solid rgba(255, 255, 255, 200);"
        "}"
    );

    QVBoxLayout* contentLayout = new QVBoxLayout(container);
    contentLayout->setContentsMargins(50, 40, 50, 40);
    contentLayout->setSpacing(20);

    // 版本号
    QLabel* version = new QLabel("Version 1.0");
    version->setAlignment(Qt::AlignCenter);
    version->setStyleSheet("color: #044BB7; font-size: 20px; font-weight: bold; background: transparent;");

    // 文本内容 (注意：文字颜色改为了深色 #333 和 #044BB7)
    QTextBrowser* browser = new QTextBrowser();
    browser->setHtml(
        "<h3 style='color:#044BB7; text-align:center;'>开发团队</h3>"
        "<p style='color:#333; font-size:18px; text-align:center; font-family: Microsoft YaHei;'>"
        "开发者：张涵玮、刘智童、周秉龙、闵世宇<br><br>"
        "Designed with Qt 6 & C++"
        "</p>"
    );
    // 修改 TextBrowser 样式：去边框，透明背景
    browser->setStyleSheet(
        "background: transparent; border: none;"
    );

    // ✅ 返回按钮 (使用图片按钮)
    m_btnBack = new GameButton("assets/images/返回主菜单.png");

    // 布局添加
    contentLayout->addWidget(version);
    contentLayout->addWidget(browser);

    // 按钮居中
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(m_btnBack);
    btnLayout->addStretch();
    contentLayout->addLayout(btnLayout);

    // 返回到主菜单 (Page 1) 或登录页 (Page 0)，视需求而定
    connect(m_btnBack, &QPushButton::clicked, [this]() { m_mainWin->switchPage(1); });

    // 添加 Box 代理
    m_boxProxy = scene->addWidget(container);
    m_boxProxy->setZValue(1);

    // =========================================================
    // 4. Logo (Z=2)
    // =========================================================
    m_logo = new GameLogo("assets/images/logo_宝石迷阵.png");
    m_logoProxy = scene->addWidget(m_logo);
    m_logoProxy->setZValue(2);


    // =========================================================
    // ✅ 新增：开发文档按钮 (Z=3, 保证在背景框之上)
    // =========================================================
    // 假设图片放在 assets/images/ 下
    m_btnDoc = new GameButton("assets/images/开发文档.png");
    m_docProxy = scene->addWidget(m_btnDoc);
    m_docProxy->setZValue(3); // 确保比背景框(Z可能为1)高

    // 点击打开 Word 文档
    connect(m_btnDoc, &QPushButton::clicked, this, [=]() {
        // 如果是 Windows 且文件路径包含中文，QUrl::fromLocalFile 处理最稳妥
        QDesktopServices::openUrl(QUrl::fromLocalFile("assets/test.docx"));
        });

    // =========================================================
    // ✅ 新增：代码仓库按钮 (Z=3)
    // =========================================================
    m_btnRepo = new GameButton("assets/images/代码仓库.png");
    m_repoProxy = scene->addWidget(m_btnRepo);
    m_repoProxy->setZValue(3);

    // 点击打开 GitHub 仓库
    connect(m_btnRepo, &QPushButton::clicked, this, [=]() {
        QDesktopServices::openUrl(QUrl("https://github.com/Yeah495/GemMatch_CPlusPlus"));
        });






    mainLayout->addWidget(m_view);
}

void PageAbout::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

    if (m_videoItem && m_view) {
        m_videoItem->setSize(QSizeF(this->size()));
        m_view->setSceneRect(0, 0, this->width(), this->height());

        // 居中容器 (稍微偏下一点 +60)
        if (m_boxProxy) {
            QWidget* w = m_boxProxy->widget();
            if (w) m_boxProxy->setPos((this->width() - w->width()) / 2,
                (this->height() - w->height()) / 2 + 60);
        }
        // 居中 Logo (顶部 12% 位置)
        if (m_logoProxy) {
            QWidget* w = m_logoProxy->widget();
            if (w) m_logoProxy->setPos((this->width() - w->width()) / 2,
                this->height() * 0.06);
        }


        int centerX = this->width() / 2;
        int centerY = this->height() / 2;
        int buttonY = centerY + 100; // 垂直高度：比中心点低 100 像素，落入框内

        // 1. 开发文档按钮（放在左边）
        if (m_docProxy) {
            QWidget* w = m_docProxy->widget();
            if (w) m_docProxy->setPos(centerX - w->width() - 20, buttonY); // 中心向左偏移 20
        }

        // 2. 代码仓库按钮（放在右边）
        if (m_repoProxy) {
            QWidget* w = m_repoProxy->widget();
            if (w) m_repoProxy->setPos(centerX + 20, buttonY); // 中心向右偏移 20
        }
    }
}

void PageAbout::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    if (m_player) {
        m_player->setSource(QUrl::fromLocalFile(m_videoPath));
        m_player->play();
    }
    // ✅ 播放 Logo 动画
    if (m_logo) m_logo->startEntrance();
}

void PageAbout::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    if (m_player) {
        m_player->stop();
        m_player->setSource(QUrl());
    }
}