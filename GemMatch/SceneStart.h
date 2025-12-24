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

#include <QMessageBox>

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

    GameButton* m_btnHistory; // 个人战绩
    QLabel* m_avatar;         // 头像 (使用QLabel显示处理后的圆形图片)

    // 代理容器（用于定位）
    QGraphicsProxyWidget* m_logoProxy;
    QGraphicsProxyWidget* m_menuProxy;    // 中央菜单框
    QGraphicsProxyWidget* m_aboutProxy;   // 左下
    QGraphicsProxyWidget* m_settingProxy; // 右下


    // 【新增】
    QGraphicsProxyWidget* m_avatarProxy;  // 头像代理

    int m_currentDifficulty; // [新增] 记录当前选中的难度 (3, 5, 7)

    // [新增] 辅助函数：统一处理选中逻辑
    void selectDifficulty(int level);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
};

#endif // SCENESTART_H