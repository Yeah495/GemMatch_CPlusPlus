#ifndef SCENESTART_H
#define SCENESTART_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QGraphicsProxyWidget>

// 引入自定义控件
#include "GameButton.h"
#include "GameLogo.h"

class MainWindow;

class SceneStart : public QWidget {
    Q_OBJECT
public:
    explicit SceneStart(MainWindow* mainWin);

private:
    MainWindow* m_mainWin;
    void setupUI();

    // 视频背景
    QGraphicsView* m_view;
    QGraphicsVideoItem* m_videoItem;
    QMediaPlayer* m_player;
    QAudioOutput* m_audioOutput;
    QString m_videoPath;

    // --- UI 组件 ---
    GameLogo* m_logo;

    // 角落按钮
    GameButton* m_btnAbout;    // 左上
    GameButton* m_btnSettings; // 右上

    // 中央按钮组
    GameButton* m_btnEasy;
    GameButton* m_btnHard;
    GameButton* m_btnExtreme;
    GameButton* m_btnStart;
    GameButton* m_btnRank;

    // 代理容器（用于定位）
    QGraphicsProxyWidget* m_logoProxy;
    QGraphicsProxyWidget* m_menuProxy;    // 中央菜单框
    QGraphicsProxyWidget* m_aboutProxy;   // 左上
    QGraphicsProxyWidget* m_settingProxy; // 右上

protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
};

#endif // SCENESTART_H