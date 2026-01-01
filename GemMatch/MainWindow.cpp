#include "MainWindow.h"
#include "PageLogin.h"
#include "SceneStart.h"
#include "SceneGame.h"
#include "PageSettings.h"
#include "PageAbout.h"
#include "SceneRank.h"
#include "PageAdmin.h"
#include "PageStatistics.h" 
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QPainterPath>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    this->setFixedSize(1280,800);

    m_stack = new QStackedWidget(this);
    setCentralWidget(m_stack);

    //初始化所有子页面
    setupAllPages();

    //初始化遮罩和按钮
    setupGlobalUI();

    //初始化音乐
    m_bgmPlayer = new QMediaPlayer(this);
    m_bgmAudioOutput = new QAudioOutput(this);
    m_bgmPlayer->setAudioOutput(m_bgmAudioOutput);
    m_bgmAudioOutput->setVolume(0.5f);
    m_bgmPlayer->setSource(QUrl("qrc:/assets/sound/bgpiano.wav"));
    m_bgmPlayer->setLoops(QMediaPlayer::Infinite);
    m_bgmPlayer->play();

    //退出时停止并释放音乐
    QObject::connect(qApp, &QApplication::aboutToQuit, this, [this]() {
        if (m_bgmPlayer) {
            m_bgmPlayer->stop();
            m_bgmPlayer->setSource(QUrl());
        }
        if (m_bgmAudioOutput) {
            m_bgmAudioOutput->setVolume(0.0);
        }
        });
}

void MainWindow::setupAllPages() {
    m_pageLogin = new PageLogin(this);
    m_pageStart = new SceneStart(this);
    m_pageGame = new SceneGame(this);
    m_pageSettings = new PageSettings(this);
    m_pageAbout = new PageAbout(this);
    m_pageRank = new SceneRank(this);
    m_pageAdmin = new PageAdmin(this);
    m_pageStatistics = new PageStatistics(this);

    m_controller = new GameController(this);//必须在m_pageGame创建之后创建
    
    //按索引顺序添加到Stack
    m_stack->addWidget(m_pageLogin);    
    m_stack->addWidget(m_pageStart);    
    m_stack->addWidget(m_pageGame);     
    m_stack->addWidget(m_pageSettings); 
    m_stack->addWidget(m_pageAbout);    
    m_stack->addWidget(m_pageRank);     
    m_stack->addWidget(m_pageAdmin);
    m_stack->addWidget(m_pageStatistics);

    //处理游戏页面的“返回主菜单”信号,结束
    connect(m_pageGame, &SceneGame::backToMenu, [this]() {
        //切换回主菜单,并停止计时,停止音效
        m_controller->endGame();
        this->switchPage(1); 
        });

    //接收中控游戏结束信号
    connect(m_controller, &GameController::gameOver,
        this, &MainWindow::onGameOver);

    //显示登录页
    m_stack->setCurrentIndex(0);
}

void MainWindow::setupGlobalUI() {
    //亮度初始化
    m_brightnessOverlay = new QWidget(this);
    m_brightnessOverlay->setGeometry(0, 0, 1280, 800);
    m_brightnessOverlay->setStyleSheet("background-color: rgba(0,0,0,0);");
    m_brightnessOverlay->hide();

    //忽略鼠标事件
    m_brightnessOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
}

void MainWindow::switchPage(int index) {
    if (index >= 0 && index < m_stack->count()) {
        m_stack->setCurrentIndex(index);
    }
}

void MainWindow::setGlobalBrightness(int value) {
    static int lastValue = 100;
    if (abs(lastValue - value) < 5 && value != 0 && value != 100) {
        return; // 添加阈值减少调用频率
    }
    lastValue = value;

    //使用定时器延迟应用效果
    QTimer::singleShot(50, [this, value]() {
        m_brightness = value;
        updateBrightness();
        });
}

void MainWindow::updateBrightness() {
    int alpha = (100 - m_brightness) * 2;

    if (alpha <= 0) {
        //当亮度最高（透明度为0）时，直接隐藏掉控件
        m_brightnessOverlay->hide();
    }
    else {
        m_brightnessOverlay->show();
        m_brightnessOverlay->setStyleSheet(QString("background-color: rgba(0,0,0,%1);").arg(alpha));
    }
}

void MainWindow::setBGMVolume(float volume) {
    if (m_bgmAudioOutput) {
        float v = std::max(0.0f, std::min(1.0f, volume));
        m_bgmAudioOutput->setVolume(v);
    } else {
        return;
    }
}

float MainWindow::getBGMVolume() const {
    if (m_bgmAudioOutput) {
        return m_bgmAudioOutput->volume();
    }
    return 0.0f;
}

MainWindow::~MainWindow() {}

void MainWindow::startNewGame(int difficulty) {
    switchPage(2);

    //命令控制器开始一局新游戏
    m_controller->startGame(difficulty);
}

void MainWindow::onGameOver(int score) {
    //创建并显示游戏结束对话框
    GameOverDialog* dialog = new GameOverDialog(score, this);

    connect(dialog, &GameOverDialog::restartGame, [this, dialog]() {
        dialog->accept();
        this->startNewGame(); //重新开始游戏
        });

    connect(dialog, &GameOverDialog::backToMenu, [this, dialog]() {
        dialog->accept();
        m_controller->endGame();
        this->switchPage(1); //返回主菜单
        });

    dialog->exec();
    delete dialog;
}

void MainWindow::setAvatarFromFile(const QString& filePath) {
    QPixmap pix(filePath);
    if (pix.isNull()) return;
    setAvatarFromPixmap(pix);
}

void MainWindow::setAvatarFromPixmap(const QPixmap& pixmap) {
    if (pixmap.isNull()) return;

    //裁剪
    const int size = 100;
    QPixmap target(size, size);
    target.fill(Qt::transparent);

    QPainter painter(&target);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QPainterPath path;
    path.addEllipse(2, 2, size - 4, size - 4);
    painter.setClipPath(path);

    painter.drawPixmap(0, 0, size, size, pixmap);

    painter.setClipping(false);
    QPen pen(Qt::white, 4);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(2, 2, size - 4, size - 4);

    m_avatarPixmap = target;
}

const QPixmap& MainWindow::getAvatarPixmap() const {
    return m_avatarPixmap;
}