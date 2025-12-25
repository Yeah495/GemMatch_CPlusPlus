#include "MainWindow.h"
#include "PageLogin.h"
#include "SceneStart.h"
#include "SceneGame.h"
#include "PageSettings.h"
#include "PageAbout.h"
#include "SceneRank.h"
#include "PageAdmin.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    this->setFixedSize(1280,800);

    m_stack = new QStackedWidget(this);
    setCentralWidget(m_stack);

    // 1. 实例化所有子页面
    setupAllPages();

    // 2. 初始化遮罩和按钮
    setupGlobalUI();

    // 初始化全局BGM播放器
    m_bgmPlayer = new QMediaPlayer(this);
    m_bgmAudioOutput = new QAudioOutput(this);
    m_bgmPlayer->setAudioOutput(m_bgmAudioOutput);
    m_bgmAudioOutput->setVolume(0.5f); // 默认音量50%
    // 使用qrc资源路径：/assets/sound/bgpiano.wav
    m_bgmPlayer->setSource(QUrl("qrc:/assets/sound/bgpiano.wav"));
    m_bgmPlayer->setLoops(QMediaPlayer::Infinite);
    m_bgmPlayer->play();

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
    m_pageAdmin = new PageAdmin(this);

    m_controller = new GameController(this);  //必须在m_pageGame创建之后创建

    // 2. 按索引顺序添加到 Stack
    m_stack->addWidget(m_pageLogin);    // Index 0
    m_stack->addWidget(m_pageStart);    // Index 1
    m_stack->addWidget(m_pageGame);     // Index 2
    m_stack->addWidget(m_pageSettings); // Index 3
    m_stack->addWidget(m_pageAbout);    // Index 4
    m_stack->addWidget(m_pageRank);     // Index 5
    m_stack->addWidget(m_pageAdmin);

    // 3. 处理游戏页面的“返回主菜单”信号
    connect(m_pageGame, &SceneGame::backToMenu, [this]() {
        // 切换回主菜单,并停止计时,停止音效
        m_controller->endGame();
        this->switchPage(1); 
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

    //  关键：设置遮罩忽略鼠标事件
    m_brightnessOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);

    // 语言切换按钮
    m_langBtn = new QPushButton("CN / EN", this);
    m_langBtn->setGeometry(900, 20, 100, 40);
    m_langBtn->setStyleSheet("background: gold; border-radius: 10px; font-weight: bold;");
    connect(m_langBtn, &QPushButton::clicked, this, &MainWindow::toggleLanguage);
}

void MainWindow::switchPage(int index) {
    if (index >= 0 && index < m_stack->count()) {
        m_stack->setCurrentIndex(index);

        //  确保语言按钮在最上面
        m_langBtn->raise();
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
        // 亮度调节逻辑
        // 确保不要调用可能导致死循环的函数
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

        //  关键修改：确保遮罩的层级正确
        // 将遮罩放在页面内容之上，但语言按钮在遮罩之上
        m_brightnessOverlay->stackUnder(m_langBtn);
    }

    //  确保语言按钮始终在最上面
    m_langBtn->raise();
}

void MainWindow::toggleLanguage() {
    m_language = (m_language == 0) ? 1 : 0;
    m_langBtn->setText(m_language == 0 ? "中文" : "English");
}

void MainWindow::setBGMVolume(float volume) {
    if (m_bgmAudioOutput) {
        float v = std::max(0.0f, std::min(1.0f, volume));
        m_bgmAudioOutput->setVolume(v);
        qDebug() << "[BGM] set volume:" << v;
    } else {
        qDebug() << "[BGM] audio output not initialized";
    }
}

float MainWindow::getBGMVolume() const {
    if (m_bgmAudioOutput) {
        return m_bgmAudioOutput->volume();
    }
    return 0.0f;
}

MainWindow::~MainWindow() {}
SceneGame* MainWindow::getGamePage() {
    return m_pageGame;
}

void MainWindow::startNewGame(int difficulty) {
    // 1. 切换到游戏画面 (假设 index 2 是游戏页面)
    switchPage(2);

    // 2. 命令控制器开始一局新游戏
    // 这会触发：重置分数、重置时间、生成新棋盘、启动定时器
    m_controller->startGame(difficulty);
}