#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QMediaPlayer>
#include <QAudioOutput>

#include "GameController.h"

// 前向声明所有子页面类，避免头文件互相包含
class PageLogin;
class SceneStart;
class SceneGame;
class PageSettings;
class PageAbout;
class SceneRank;


class GameController; // 前置声明
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    SceneGame* getGamePage();

    // 页面跳转
    void switchPage(int index);

    // 提供给其他页面的接口
    SceneGame* getGamePage() const { return m_pageGame; }

    void startNewGame(int difficulty = 3);

    // 设置接口
    void setGlobalBrightness(int value);

    // 语言接口
    void setLanguage(int lang); // 0 = CN, 1 = EN
    int language() const { return m_language; }

    // Background music control (0-100)
    Q_SLOT void setBackgroundVolume(int volume);
    int backgroundVolume() const;

private:
    void setupAllPages();
    void setupGlobalUI();
    void updateBrightness();

    QStackedWidget* m_stack;

    // 页面指针
    PageLogin* m_pageLogin;
    SceneStart* m_pageStart;
    SceneGame* m_pageGame;
    PageSettings* m_pageSettings;
    PageAbout* m_pageAbout;
    SceneRank* m_pageRank;

    GameController* m_controller;

    // 悬浮/覆盖控件
    QWidget* m_brightnessOverlay;

    // Background music
    QMediaPlayer* m_bgPlayer;
    QAudioOutput* m_bgAudioOutput;

    int m_brightness = 100;
    int m_language = 0; // 0: CN, 1: EN
};

#endif