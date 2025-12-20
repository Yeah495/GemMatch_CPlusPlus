#pragma once
#ifndef PAGEABOUT_H
#define PAGEABOUT_H

#include <QWidget>
#include <QLabel>    
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QGraphicsProxyWidget>   

class MainWindow;

class PageAbout : public QWidget {
    Q_OBJECT
public:
    explicit PageAbout(MainWindow* mainWin);
private:
    MainWindow* m_mainWin;
    void setupUI();

    // ========== 新增：视频背景相关 ==========
    QGraphicsView* m_view;
    QGraphicsVideoItem* m_videoItem;
    QMediaPlayer* m_player;
    QAudioOutput* m_audioOutput;

protected:
    void resizeEvent(QResizeEvent* event) override;
};

#endif // PAGEABOUT_H