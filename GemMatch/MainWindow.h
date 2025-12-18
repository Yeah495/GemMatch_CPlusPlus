#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>

// 前向声明所有子页面类，避免头文件互相包含
class PageLogin;
class SceneStart;
class SceneGame;    // 这里改为 SceneGame
class PageSettings;
class PageAbout;
class SceneRank;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    // 切换页面接口：
    // 0: 登录, 1: 主菜单, 2: 游戏, 3: 设置, 4: 关于, 5: 排行榜
    void switchPage(int index);

    // 获取游戏页面指针（供 Controller 连接信号槽使用）
    SceneGame* getGamePage();

private:
    QStackedWidget* m_stack;

    PageLogin* m_pageLogin;
    SceneStart* m_pageStart;
    SceneGame* m_pageGame; // 类型修正为 SceneGame*
    PageSettings* m_pageSettings;
    PageAbout* m_pageAbout;
    SceneRank* m_pageRank;
};

#endif // MAINWINDOW_H