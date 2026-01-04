#ifndef PAGEABOUT_H
#define PAGEABOUT_H

#include <QWidget>
#include <QLabel>
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QGraphicsProxyWidget>
#include <QDesktopServices> 
#include <QUrl>            


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


    GameLogo* m_logo;
    GameButton* m_btnBack; 
//两个按钮的控件指针
    GameButton* m_btnDoc;  // 开发文档按钮
    GameButton* m_btnRepo; // 代码仓库按钮


    QGraphicsProxyWidget* m_boxProxy;
    QGraphicsProxyWidget* m_logoProxy;
 
    QGraphicsProxyWidget* m_docProxy;
    QGraphicsProxyWidget* m_repoProxy;


protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
};

#endif