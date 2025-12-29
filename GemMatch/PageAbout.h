#ifndef PAGEABOUT_H
#define PAGEABOUT_H

#include <QWidget>
#include <QLabel>
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QGraphicsProxyWidget>
#include <QDesktopServices> // ✅ 新增：用于打开外部链接
#include <QUrl>             // ✅ 新增：用于构建 URL

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
    // ✅ 新增：两个按钮的控件指针
    GameButton* m_btnDoc;  // 开发文档按钮
    GameButton* m_btnRepo; // 代码仓库按钮

    // ✅ 两个代理
    QGraphicsProxyWidget* m_boxProxy;
    QGraphicsProxyWidget* m_logoProxy;
    // ✅ 新增：两个按钮在场景中的代理指针（用于控制位置）
    QGraphicsProxyWidget* m_docProxy;
    QGraphicsProxyWidget* m_repoProxy;


protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
};

#endif