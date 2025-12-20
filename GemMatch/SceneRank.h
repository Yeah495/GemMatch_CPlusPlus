#pragma once
#ifndef SCENERANK_H
#define SCENERANK_H

#include <QWidget>
#include <QTableWidget>
#include <QGraphicsView>           
#include <QGraphicsVideoItem>      
#include <QMediaPlayer>            
#include <QAudioOutput>            
#include <QGraphicsProxyWidget>   

class MainWindow;

class SceneRank : public QWidget {
    Q_OBJECT
public:
    explicit SceneRank(MainWindow* mainWin);

protected:
    void showEvent(QShowEvent* event) override;

    void hideEvent(QHideEvent* event) override; // ✅ 新增
    void resizeEvent(QResizeEvent* event) override;

private:
    MainWindow* m_mainWin;
    QTableWidget* m_table;

    // ========== 视频背景相关 ==========
    QGraphicsView* m_view;
    QGraphicsVideoItem* m_videoItem;
    QMediaPlayer* m_player;
    QAudioOutput* m_audioOutput;

    QString m_videoPath; // ✅ 新增
    // =================================

    void setupUI();
    void loadRankData();
};

#endif // SCENERANK_H
