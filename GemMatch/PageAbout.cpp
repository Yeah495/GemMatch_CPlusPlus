#include "PageAbout.h"
#include "MainWindow.h"
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QPushButton>

PageAbout::PageAbout(MainWindow* mainWin) : QWidget(mainWin), m_mainWin(mainWin) {
    setupUI();
}

void PageAbout::setupUI() {
    this->setStyleSheet("PageAbout { border-image: url(:/assets/images/bg_menu.png); }");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(150, 80, 150, 80);

    QTextBrowser* browser = new QTextBrowser();
    browser->setHtml(
        "<h1 style='color:gold; text-align:center;'>GEM MATCH</h1>"
        "<h3 style='color:white; text-align:center;'>Version 1.0</h3>"
        "<br>"
        "<p style='color:#ccc; font-size:16px; text-align:center;'>"
        "Developed by: Your Name & Team<br>"
        "Technology: C++ / Qt 6.0<br>"
        "Course: Comprehensive Software Experiment<br>"
        "<br>"
        "Copyright © 2025 All Rights Reserved."
        "</p>"
    );
    browser->setStyleSheet("background: rgba(0,0,0,0.8); border: 2px solid #555; border-radius: 10px; padding: 20px;");

    layout->addWidget(browser);

    QPushButton* btnBack = new QPushButton("BACK");
    btnBack->setStyleSheet("font-size: 18px; padding: 10px; background: #444; color: white; border-radius: 5px; margin-top: 20px;");
    connect(btnBack, &QPushButton::clicked, [this]() { m_mainWin->switchPage(1); });

    layout->addWidget(btnBack, 0, Qt::AlignCenter);
}