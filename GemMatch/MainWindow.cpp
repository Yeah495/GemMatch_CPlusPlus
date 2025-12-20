#include "MainWindow.h"
#include "PageLogin.h"
#include "SceneStart.h"
#include "SceneGame.h"
#include "PageSettings.h"
#include "PageAbout.h"
#include "SceneRank.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    this->setFixedSize(1280,800);

    m_stack = new QStackedWidget(this);
    setCentralWidget(m_stack);

    // 1. 实例化所有子页面
    setupAllPages();

    // 2. 初始化遮罩和按钮
    setupGlobalUI();

    // 进入默认页
    switchPage(0);
}

void MainWindow::setupAllPages() {
    m_pageLogin = new PageLogin(this);
    m_pageStart = new SceneStart(this);
    m_pageGame = new SceneGame(this);
    m_pageSettings = new PageSettings(this);
    m_pageAbout = new PageAbout(this);
    m_pageRank = new SceneRank(this);

    m_stack->addWidget(m_pageLogin);    // 0
    m_stack->addWidget(m_pageStart);    // 1
    m_stack->addWidget(m_pageGame);     // 2
    m_stack->addWidget(m_pageSettings); // 3
    m_stack->addWidget(m_pageAbout);    // 4
    m_stack->addWidget(m_pageRank);     // 5

    connect(m_pageGame, &SceneGame::backToMenu, [this]() { this->switchPage(1); });
}

void MainWindow::setupGlobalUI() {
    // 亮度遮罩：初始化
    m_brightnessOverlay = new QWidget(this);
    m_brightnessOverlay->setGeometry(0, 0, 1280, 800);
    // 移除 WA_TransparentForInput 报错代码
    // 通过 updateBrightness 中的 hide() 逻辑来防止拦截鼠标信号
    m_brightnessOverlay->setStyleSheet("background-color: rgba(0,0,0,0);");
    m_brightnessOverlay->hide();

    // 语言切换按钮
    m_langBtn = new QPushButton("CN / EN", this);
    m_langBtn->setGeometry(900, 20, 100, 40);
    m_langBtn->setStyleSheet("background: gold; border-radius: 10px; font-weight: bold;");
    connect(m_langBtn, &QPushButton::clicked, this, &MainWindow::toggleLanguage);
}

void MainWindow::switchPage(int index) {
    if (index >= 0 && index < m_stack->count()) {
        m_stack->setCurrentIndex(index);

        // 确保语言按钮始终在最上面
        m_langBtn->raise();

        // 如果遮罩正在显示，则也将遮罩置顶
        if (m_brightnessOverlay->isVisible()) {
            m_brightnessOverlay->raise();
            // 注意：如果遮罩置顶会拦截点击。
            // 优化技巧：将常用 UI 按钮（如语言按钮）放在遮罩之上
            m_langBtn->raise();
        }
    }
}

void MainWindow::setGlobalBrightness(int value) {
    m_brightness = value;
    updateBrightness();
}

void MainWindow::updateBrightness() {
    // 亮度为 100 时，alpha 为 0
    int alpha = (100 - m_brightness) * 2;

    if (alpha <= 0) {
        // 当亮度最高（透明度为0）时，直接隐藏掉控件
        // 这样它就完全不会干扰页面的鼠标点击事件
        m_brightnessOverlay->hide();
    }
    else {
        m_brightnessOverlay->show();
        m_brightnessOverlay->setStyleSheet(QString("background-color: rgba(0,0,0,%1);").arg(alpha));
        // 将语言按钮提至遮罩上方，确保亮度低时也能切换语言
        m_langBtn->raise();
    }
}

void MainWindow::toggleLanguage() {
    m_language = (m_language == 0) ? 1 : 0;
    m_langBtn->setText(m_language == 0 ? "中文" : "English");
}

MainWindow::~MainWindow() {}