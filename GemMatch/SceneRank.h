#ifndef SCENERANK_H
#define SCENERANK_H

#include <QWidget>
#include <QTableWidget>
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QMediaPlayer>
#include <QGraphicsProxyWidget>

#include "GameButton.h"
#include "GameLogo.h"

class MainWindow;

class SceneRank : public QWidget {
    Q_OBJECT
public:
    explicit SceneRank(MainWindow* mainWin);

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    MainWindow* m_mainWin;
    QTableWidget* m_table;

    QGraphicsView* m_view;
    QGraphicsVideoItem* m_videoItem;
    QMediaPlayer* m_player;
    QAudioOutput* m_audioOutput;
    QString m_videoPath;

    GameLogo* m_logo;
    GameButton* m_btnEasy;
    GameButton* m_btnHard;
    GameButton* m_btnExtreme;
    GameButton* m_btnBack;

    QGraphicsProxyWidget* m_containerProxy;
    QGraphicsProxyWidget* m_logoProxy;

    void setupUI();
    void loadRankData(int difficultyLevel = 3);
};

#endif 