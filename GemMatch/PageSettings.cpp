#include "PageSettings.h"
#include "MainWindow.h"
#include <QVBoxLayout>
#include <QPushButton>

PageSettings::PageSettings(MainWindow* mainWin) : QWidget(mainWin), m_mainWin(mainWin) {
    setupUI();
}

void PageSettings::setupUI() {
    // 背景样式
    this->setStyleSheet("PageSettings { border-image: url(:/assets/images/bg_menu.png); } "
        "QLabel { color: white; font-size: 18px; font-weight: bold; }");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QWidget* container = new QWidget(this);
    container->setFixedSize(400, 450);
    container->setStyleSheet("background: rgba(0,0,0,0.8); border-radius: 20px;");

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
    brightSlider->setRange(10, 100); // 最小保持10%亮度防止黑屏
    brightSlider->setValue(100);
    form->addWidget(brightSlider);
    connect(brightSlider, &QSlider::valueChanged, [this](int v) {
        m_mainWin->setGlobalBrightness(v);
        });

    form->addStretch();

    // 返回按钮
    QPushButton* btnBack = new QPushButton("保存并返回 (Save & Back)");
    btnBack->setStyleSheet("QPushButton { background: gold; color: black; padding: 10px; border-radius: 5px; }"
        "QPushButton:hover { background: white; }");
    connect(btnBack, &QPushButton::clicked, [this]() { m_mainWin->switchPage(0); });
    form->addWidget(btnBack);

    mainLayout->addWidget(container, 0, Qt::AlignCenter);
}