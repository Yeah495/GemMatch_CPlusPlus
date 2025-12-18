#include "SceneRank.h"
#include "MainWindow.h"
#include "UserManager.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QHeaderView>

SceneRank::SceneRank(MainWindow* mainWin) : QWidget(mainWin), m_mainWin(mainWin) {
    setupUI();
}

void SceneRank::setupUI() {
    this->setStyleSheet("SceneRank { border-image: url(:/assets/images/bg_menu.png); }");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(100, 50, 100, 50);

    // 标题
    QLabel* title = new QLabel("TOP PLAYERS");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 40px; color: gold; font-weight: bold; margin-bottom: 20px;");
    layout->addWidget(title);

    // 表格
    m_table = new QTableWidget();
    m_table->setColumnCount(3);
    m_table->setHorizontalHeaderLabels({ "Rank", "Player", "High Score" });
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);

    // 表格样式
    m_table->setStyleSheet("QTableWidget { background: rgba(0,0,0,0.7); gridline-color: #555; color: white; font-size: 18px; border: none; }"
        "QHeaderView::section { background: #333; color: gold; padding: 5px; font-weight: bold; border: 1px solid #555; }");

    layout->addWidget(m_table);

    // 返回按钮
    QPushButton* btnBack = new QPushButton("BACK");
    btnBack->setStyleSheet("QPushButton { font-size: 18px; color: white; background: #444; padding: 10px; border-radius: 5px; }"
        "QPushButton:hover { background: #666; }");
    connect(btnBack, &QPushButton::clicked, [this]() { m_mainWin->switchPage(1); }); // 回主菜单

    layout->addWidget(btnBack, 0, Qt::AlignCenter);
}

void SceneRank::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    loadRankData();
}

void SceneRank::loadRankData() {
    QList<UserData> list = UserManager::instance().getTop10();

    m_table->setRowCount(list.size());
    for (int i = 0; i < list.size(); ++i) {
        // 排名
        QTableWidgetItem* itemRank = new QTableWidgetItem(QString::number(i + 1));
        itemRank->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(i, 0, itemRank);

        // 用户名
        QTableWidgetItem* itemName = new QTableWidgetItem(list[i].username);
        itemName->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(i, 1, itemName);

        // 分数
        QTableWidgetItem* itemScore = new QTableWidgetItem(QString::number(list[i].highScore));
        itemScore->setTextAlignment(Qt::AlignCenter);
        itemScore->setForeground(QBrush(Qt::yellow));
        m_table->setItem(i, 2, itemScore);
    }
}