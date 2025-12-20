#pragma once
#ifndef SCENESTART_H
#define SCENESTART_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QMediaPlayer>
#include <QAudioOutput>

class MainWindow;

class SceneStart : public QWidget {
    Q_OBJECT
public:
    explicit SceneStart(MainWindow* mainWin);

private:
    MainWindow* m_mainWin;
    void setupUI();

    // ========== 视频背景相关 ==========
    QGraphicsView* m_view;
    QGraphicsVideoItem* m_videoItem;
    QMediaPlayer* m_player;
    QAudioOutput* m_audioOutput;
    // =================================

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
};

#endif // SCENESTART_H
