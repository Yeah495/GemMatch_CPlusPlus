#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QtWidgets/QApplication>
#include "GameController.h"
#include "GameOverDialog.h"

//前向声明所有子页面类和中控
class PageLogin;
class SceneStart;
class SceneGame;
class PageSettings;
class PageAbout;
class SceneRank;
class PageAdmin;
class PageStatistics;

class GameController;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    //切换页面
    void switchPage(int index);

    //得到主游戏界面对象
    SceneGame* getGamePage() const { return m_pageGame; }

    //开始游戏
    void startNewGame(int difficulty = 3);

    //设置亮度和音乐
    void setGlobalBrightness(int value);
    void setBGMVolume(float volume);

    //得到音乐音量
    float getBGMVolume() const;

    //设置与获取头像
    void setAvatarFromFile(const QString& filePath);
    void setAvatarFromPixmap(const QPixmap& pixmap);
    const QPixmap& getAvatarPixmap() const;

private:
    //初始化ui界面
    void setupAllPages();
    void setupGlobalUI();

    //页面指针
    QStackedWidget* m_stack;
    PageLogin* m_pageLogin;
    SceneStart* m_pageStart;
    SceneGame* m_pageGame;
    PageSettings* m_pageSettings;
    PageAbout* m_pageAbout;
    SceneRank* m_pageRank;
    PageAdmin* m_pageAdmin;
    PageStatistics* m_pageStatistics;

    //唯一中控
    GameController* m_controller;

    //音乐组件
    QMediaPlayer* m_bgmPlayer = nullptr;
    QAudioOutput* m_bgmAudioOutput = nullptr;

    //随机头像
    QPixmap m_avatarPixmap;
    
    //亮度组件和函数
    int m_brightness = 100;
    QWidget* m_brightnessOverlay;
    void updateBrightness();

    void onGameOver(int score);
};

#endif // MAINWINDOW_H