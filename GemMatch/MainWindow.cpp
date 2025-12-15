#include "MainWindow.h"
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUI();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
    this->setWindowTitle("宝石迷阵 - C++ Qt 实训项目");
    this->resize(1024, 768);

    // 创建 View 和 Scene
    m_sceneGame = new SceneGame(this);
    m_view = new QGraphicsView(m_sceneGame, this);

    // 优化渲染属性
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setRenderHint(QPainter::SmoothPixmapTransform);
    m_view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

    // 设置为中心部件
    setCentralWidget(m_view);

    // 可以在这里添加菜单栏 (开始、撤销、关于)
    // QMenu* gameMenu = menuBar()->addMenu("游戏");
    // gameMenu->addAction("新游戏");
    // gameMenu->addAction("撤销");
}