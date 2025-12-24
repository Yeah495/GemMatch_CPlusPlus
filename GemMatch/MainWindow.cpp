#include "MainWindow.h"
#include "PageLogin.h"
#include "SceneStart.h"
#include "SceneGame.h"
#include "PageSettings.h"
#include "PageAbout.h"
#include "SceneRank.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    this->setFixedSize(1280, 800);

    m_stack = new QStackedWidget(this);
    setCentralWidget(m_stack);

    // 1. 实例化所有子页面
    setupAllPages();

    // 2. 初始化遮罩 (语言按钮已在此处彻底移除)
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

    m_controller = new GameController(this);  // 必须在m_pageGame创建之后创建

    // 2. 按索引顺序添加到 Stack
    m_stack->addWidget(m_pageLogin);    // Index 0
    m_stack->addWidget(m_pageStart);    // Index 1
    m_stack->addWidget(m_pageGame);     // Index 2
    m_stack->addWidget(m_pageSettings); // Index 3
    m_stack->addWidget(m_pageAbout);    // Index 4
    m_stack->addWidget(m_pageRank);     // Index 5

    // 3. 处理游戏页面的“返回主菜单”信号
    connect(m_pageGame, &SceneGame::backToMenu, [this]() {
        // 切换回主菜单,并停止计时
        this->switchPage(1);
        m_controller->endGame();
        });

    // 默认显示登录页
    m_stack->setCurrentIndex(0);
}

void MainWindow::setupGlobalUI() {
    // 亮度遮罩：初始化
    m_brightnessOverlay = new QWidget(this);
    m_brightnessOverlay->setGeometry(0, 0, 1280, 800);
    m_brightnessOverlay->setStyleSheet("background-color: rgba(0,0,0,0);");
    m_brightnessOverlay->hide();

    // 关键：设置遮罩忽略鼠标事件
    m_brightnessOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
}

void MainWindow::switchPage(int index) {
    if (index >= 0 && index < m_stack->count()) {
        m_stack->setCurrentIndex(index);
    }
}

void MainWindow::setGlobalBrightness(int value) {
    // 避免频繁重绘
    static int lastValue = 100;
    if (abs(lastValue - value) < 5 && value != 0 && value != 100) {
        return; // 添加阈值减少调用频率
    }
    lastValue = value;

    // 使用定时器延迟应用效果
    QTimer::singleShot(50, [this, value]() {
        m_brightness = value;
        updateBrightness();
        });
}

void MainWindow::updateBrightness() {
    // 亮度为 100 时，alpha 为 0
    int alpha = (100 - m_brightness) * 2;

    if (alpha <= 0) {
        // 当亮度最高（透明度为0）时，直接隐藏掉控件
        m_brightnessOverlay->hide();
    }
    else {
        m_brightnessOverlay->show();
        m_brightnessOverlay->setStyleSheet(QString("background-color: rgba(0,0,0,%1);").arg(alpha));
    }
}

MainWindow::~MainWindow() {}

SceneGame* MainWindow::getGamePage() {
    return m_pageGame;
}

void MainWindow::startNewGame(int difficulty) {
    // 1. 切换到游戏画面
    switchPage(2);

    // 2. 命令控制器开始一局新游戏
    m_controller->startGame(difficulty);
}