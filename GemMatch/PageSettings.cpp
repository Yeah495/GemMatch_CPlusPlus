#include "PageSettings.h"
#include "MainWindow.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QPushButton>

PageSettings::PageSettings(MainWindow* mainWin) : QWidget(mainWin), m_mainWin(mainWin) {
    setupUI();
}

void PageSettings::setupUI() {
    this->setStyleSheet("PageSettings { border-image: url(:/assets/images/bg_menu.png); } "
        "QLabel { color: white; font-size: 20px; font-weight: bold; }");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);

    QWidget* container = new QWidget(this);
    container->setFixedSize(400, 500);
    container->setStyleSheet("background: rgba(0,0,0,0.8); border-radius: 15px;");

    QVBoxLayout* form = new QVBoxLayout(container);
    form->setSpacing(30);
    form->setContentsMargins(40, 40, 40, 40);

    form->addWidget(new QLabel("设置"), 0, Qt::AlignCenter);

    // 音量
    form->addWidget(new QLabel("背景音量："));
    QSlider* slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 100);
    slider->setValue(50);
    form->addWidget(slider);

    // 难度
    form->addWidget(new QLabel("难度："));
    QComboBox* combo = new QComboBox();
    combo->addItems({ "简单", "困难", "极限" });
    combo->setStyleSheet("padding: 5px; font-size: 16px;");
    form->addWidget(combo);

    form->addStretch();

    QPushButton* btnBack = new QPushButton("保存 ");
    btnBack->setStyleSheet("padding: 10px; background: gold; border-radius: 5px; font-weight: bold;");
    connect(btnBack, &QPushButton::clicked, [this]() { m_mainWin->switchPage(1); });

    form->addWidget(btnBack);

    layout->addWidget(container);
}