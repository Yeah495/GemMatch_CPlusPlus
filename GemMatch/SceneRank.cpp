#include "SceneRank.h"
#include "MainWindow.h"
#include "UserManager.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QHeaderView>
#include <QGraphicsProxyWidget>  // ✅ 新增
#include <QUrl>                  // ✅ 新增

SceneRank::SceneRank(MainWindow* mainWin) : QWidget(mainWin), m_mainWin(mainWin) {
    setupUI();
}

void SceneRank::setupUI() {
    // ========== 步骤 1: 创建主布局 ==========
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ========== 步骤 2: 创建 Graphics View ==========
    m_view = new QGraphicsView(this);
    m_view->setStyleSheet("border: none; background: transparent;");
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QGraphicsScene* scene = new QGraphicsScene(this);
    m_view->setScene(scene);

    // ========== 步骤 3: 添加视频层（底层，Z=0）==========
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(0.0f);

    m_videoItem = new QGraphicsVideoItem();
    m_videoItem->setSize(QSizeF(1280, 800));  // ✅ 修改：适配窗口尺寸
    m_videoItem->setZValue(0);
    scene->addItem(m_videoItem);

    m_player->setVideoOutput(m_videoItem);
    m_videoPath = "assets/videos/7.mp4";
    m_player->setLoops(QMediaPlayer::Infinite);

    // ========== 步骤 4: 创建 UI 容器 ==========
    QWidget* container = new QWidget();
    container->setFixedSize(900, 650);  // ✅ 修改：适配窗口
    container->setStyleSheet("background: rgba(0,0,0,0.8); border-radius: 20px;");

    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setContentsMargins(40, 40, 40, 40);  // ✅ 调整边距
    layout->setSpacing(15);

    // 标题
    QLabel* title = new QLabel("排行榜");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 32px; color: gold; font-weight: bold; margin-bottom: 15px;");  // ✅ 调整字体
    layout->addWidget(title);

    // 表格
    m_table = new QTableWidget();
    m_table->setColumnCount(3);
    m_table->setHorizontalHeaderLabels({ "排名", "用户", "得分" });
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->verticalHeader()->setVisible(false);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);

    // 表格样式
    m_table->setStyleSheet(
        "QTableWidget { background: rgba(0,0,0,0.7); gridline-color: #555; "
        "color: white; font-size: 16px; border: none; }"  // ✅ 调整字体
        "QHeaderView::section { background: #333; color: gold; padding: 5px; "
        "font-weight: bold; border: 1px solid #555; }"
    );

    layout->addWidget(m_table);

    // 返回按钮
    QPushButton* btnBack = new QPushButton("返回");
    btnBack->setStyleSheet(
        "QPushButton { font-size: 16px; color: white; background: #444; "
        "padding: 8px; border-radius: 5px; }"
        "QPushButton:hover { background: #666; }"
    );
    connect(btnBack, &QPushButton::clicked, [this]() { m_mainWin->switchPage(1); });

    layout->addWidget(btnBack, 0, Qt::AlignCenter);

    // ========== 步骤 5: 将容器添加到场景 ==========
    QGraphicsProxyWidget* proxy = scene->addWidget(container);
    proxy->setZValue(1);
    proxy->setPos((1280 - 900) / 2, (800 - 650) / 2);  // ✅ 修改：适配窗口

    // ========== 步骤 6: 添加到主布局 ==========
    mainLayout->addWidget(m_view);
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

void SceneRank::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

    if (m_videoItem && m_view) {
        m_videoItem->setSize(QSizeF(this->size()));
        m_view->setSceneRect(0, 0, this->width(), this->height());

        QList<QGraphicsItem*> items = m_view->scene()->items();
        for (auto* item : items) {
            if (QGraphicsProxyWidget* proxy = qgraphicsitem_cast<QGraphicsProxyWidget*>(item)) {
                QWidget* widget = proxy->widget();
                if (widget) {
                    proxy->setPos((this->width() - widget->width()) / 2,
                        (this->height() - widget->height()) / 2);
                }
            }
        }
    }
}





// 修改 showEvent
void SceneRank::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    loadRankData(); //原本的逻辑

    // 【新增】懒加载视频
    if (m_player) {
        m_player->setSource(QUrl::fromLocalFile(m_videoPath));
        m_player->play();
    }
}

// 【新增】hideEvent
void SceneRank::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);

    // 【新增】卸载视频
    if (m_player) {
        m_player->stop();
        m_player->setSource(QUrl()); // ✅ 释放内存的关键
    }
}