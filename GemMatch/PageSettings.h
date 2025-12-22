#ifndef PAGESETTINGS_H
#define PAGESETTINGS_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
// ✅ 在文件开头添加
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QGraphicsProxyWidget>    

// ✅ 引入自定义控件
#include "GameButton.h"
#include "GameLogo.h"

class MainWindow;

class PageSettings : public QWidget {
    Q_OBJECT
public:
    explicit PageSettings(MainWindow* mainWin);

private:
    MainWindow* m_mainWin;
    void setupUI();

    QLabel* m_labelTitle;
    QLabel* m_labelMusic;
    QLabel* m_labelBrightness;


    // ✅ 改用图片按钮
    GameButton* m_btnBack;
    GameButton* m_btnReLogin;

    QGraphicsView* m_view;
    QGraphicsVideoItem* m_videoItem;
    QMediaPlayer* m_player;
    QAudioOutput* m_audioOutput;

    QString m_videoPath; // ✅ 新增

    // ✅ 新增：Logo 相关
    GameLogo* m_logo;

    // ✅ 新增：独立的代理控件（控制位置）
    QGraphicsProxyWidget* m_boxProxy;  // 设置框代理
    QGraphicsProxyWidget* m_logoProxy; // Logo 代理

protected:
    void resizeEvent(QResizeEvent* event) override;

    void showEvent(QShowEvent* event) override; // ✅ 新增
    void hideEvent(QHideEvent* event) override; // ✅ 新增
};

#endif