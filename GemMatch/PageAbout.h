#ifndef PAGEABOUT_H
#define PAGEABOUT_H

#include <QWidget>
#include <QLabel>
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QGraphicsProxyWidget>

// ✅ 引入自定义控件
#include "GameButton.h"
#include "GameLogo.h"

class MainWindow;

class PageAbout : public QWidget {
    Q_OBJECT
public:
    explicit PageAbout(MainWindow* mainWin);

private:
    MainWindow* m_mainWin;
    void setupUI();

    // 视频背景
    QGraphicsView* m_view;
    QGraphicsVideoItem* m_videoItem;
    QMediaPlayer* m_player;
    QAudioOutput* m_audioOutput;
    QString m_videoPath;

    // ✅ 新增控件
    GameLogo* m_logo;
    GameButton* m_btnBack; // ✅ 这次加上了！

    // ✅ 两个代理
    QGraphicsProxyWidget* m_boxProxy;
    QGraphicsProxyWidget* m_logoProxy;


protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
};

#endif