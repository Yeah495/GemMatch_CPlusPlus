#include "MainWindow.h"
#include <QDebug>

// 引入所有子页面的头文件
#include "PageLogin.h"
#include "SceneStart.h"
#include "SceneGame.h"  // 引用正确的头文件
#include "PageSettings.h"
#include "PageAbout.h"
#include "SceneRank.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    this->setFixedSize(1024, 768);
    this->setWindowTitle("Gem Match - C++ Project");

    m_stack = new QStackedWidget(this);
    setCentralWidget(m_stack);

    // 1. 实例化所有页面
    m_pageLogin = new PageLogin(this);
    m_pageStart = new SceneStart(this);
    m_pageGame = new SceneGame(this); // 直接实例化 SceneGame
    m_pageSettings = new PageSettings(this);
    m_pageAbout = new PageAbout(this);
    m_pageRank = new SceneRank(this);

    m_controller = new GameController(this);  //必须在m_pageGame创建之后创建

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

MainWindow::~MainWindow() {
    // Qt的对象树机制会自动清理子对象，无需手动delete
}

void MainWindow::switchPage(int index) {
    if (index >= 0 && index < m_stack->count()) {
        m_stack->setCurrentIndex(index);
    }
}

SceneGame* MainWindow::getGamePage() {
    return m_pageGame;
}

void MainWindow::startNewGame() {
    // 1. 切换到游戏画面 (假设 index 2 是游戏页面)
    switchPage(2);

    // 2. 命令控制器开始一局新游戏
    // 这会触发：重置分数、重置时间、生成新棋盘、启动定时器
    m_controller->startGame();
}