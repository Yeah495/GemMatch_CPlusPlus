#ifndef SCENESTART_H
#define SCENESTART_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QGraphicsProxyWidget>

#include "GameButton.h"
#include "GameLogo.h"

#include <QMessageBox>

class MainWindow;

class SceneStart : public QWidget {
    Q_OBJECT
public:
    explicit SceneStart(MainWindow* mainWin);

private:
    MainWindow* m_mainWin;
    void setupUI();

    //视频背景
    QGraphicsView* m_view;
    QGraphicsVideoItem* m_videoItem;
    QMediaPlayer* m_player;
    QAudioOutput* m_audioOutput;
    QString m_videoPath;

    //UI组件
    //logo
    GameLogo* m_logo;

    // 角落按钮
    GameButton* m_btnAbout;    
    GameButton* m_btnSettings; 

    // 中央按钮组
    GameButton* m_btnEasy;
    GameButton* m_btnHard;
    GameButton* m_btnExtreme;
    GameButton* m_btnStart;
    GameButton* m_btnRank;

    //个人战绩和头像
    GameButton* m_btnHistory; 
    QLabel* m_avatar;     
    QGraphicsProxyWidget* m_avatarProxy;

    //定位容器
    QGraphicsProxyWidget* m_logoProxy;
    QGraphicsProxyWidget* m_menuProxy;   
    QGraphicsProxyWidget* m_aboutProxy;  
    QGraphicsProxyWidget* m_settingProxy;

    int m_currentDifficulty; //记录当前选中的难度 (3, 5, 7)

    //处理选中
    void selectDifficulty(int level);
protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
};

#endif // SCENESTART_H