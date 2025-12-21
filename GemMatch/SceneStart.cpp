#include "SceneStart.h"
#include "MainWindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QApplication>
#include <QPainter>
#include <QStyleOption>

SceneStart::SceneStart(MainWindow* mainWin) : QWidget(mainWin), m_mainWin(mainWin) {
    setupUI();
}

void SceneStart::paintEvent(QPaintEvent* event) {
    QStyleOption opt;
    // 如果 init 报错，请使用 initFrom
    opt.initFrom(this);

    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void SceneStart::setupUI() {
    this->setObjectName("SceneStart");
    // 背景
    this->setStyleSheet("#SceneStart { border-image: url(:/assets/images/menu.jpg); }");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);

    // 标题
    QLabel* title = new QLabel("宝石迷阵 ");
    title->setStyleSheet("font-size: 60px; font-weight: bold; color: white; margin-bottom: 50px;");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    // 通用按钮样式
    QString btnStyle = "QPushButton { font-size: 24px; color: white; background-color: rgba(0,0,0,0.6); "
        "border: 2px solid gold; border-radius: 25px; padding: 10px 40px; min-width: 200px; }"
        "QPushButton:hover { background-color: gold; color: black; }";

    QPushButton* btnStart = new QPushButton("开始游戏");
    btnStart->setStyleSheet(btnStyle);

    QPushButton* btnRank = new QPushButton("排行榜");
    btnRank->setStyleSheet(btnStyle);

    QPushButton* btnSettings = new QPushButton("设置");
    btnSettings->setStyleSheet(btnStyle);

    QPushButton* btnAbout = new QPushButton("关于");
    btnAbout->setStyleSheet(btnStyle);

    QPushButton* btnExit = new QPushButton("退出游戏");
    btnExit->setStyleSheet(btnStyle);

    layout->addWidget(btnStart);
    layout->addWidget(btnRank);
    layout->addWidget(btnSettings);
    layout->addWidget(btnAbout);
    layout->addWidget(btnExit);

    // 导航连接
    connect(btnStart, &QPushButton::clicked, [this]() { 
        m_mainWin->startNewGame(); 
    });
    connect(btnRank, &QPushButton::clicked, [this]() { m_mainWin->switchPage(5); });
    connect(btnSettings, &QPushButton::clicked, [this]() { m_mainWin->switchPage(3); });
    connect(btnAbout, &QPushButton::clicked, [this]() { m_mainWin->switchPage(4); });
    connect(btnExit, &QPushButton::clicked, []() { QApplication::quit(); });
}