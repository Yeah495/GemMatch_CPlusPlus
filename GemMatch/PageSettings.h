#ifndef PAGESETTINGS_H
#define PAGESETTINGS_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QGraphicsProxyWidget>
#include <QComboBox>
#include <QPushButton>

#include "GameButton.h"
#include "GameLogo.h"

class MainWindow;

class PageSettings : public QWidget {
    Q_OBJECT
public:
    explicit PageSettings(MainWindow* mainWin);

    void setLanguage(int lang); // 0 CN, 1 EN

private:
    MainWindow* m_mainWin;
    void setupUI();

    QLabel* m_labelTitle;
    QLabel* m_labelLang; // label for language row
    QLabel* m_labelMusic;
    QLabel* m_labelBrightness;


    // ✅ 改用图片按钮
    GameButton* m_btnBack;
    GameButton* m_btnReLogin;

    QGraphicsView* m_view;
    QGraphicsVideoItem* m_videoItem;
    QMediaPlayer* m_player;
    QAudioOutput* m_audioOutput;

    // ✅ 新增：音量滑块
    QSlider* m_musicSlider;

    QString m_videoPath; // ✅ 新增

    // ✅ 新增：Logo 相关
    GameLogo* m_logo;

    // ✅ 新增：独立的代理控件（控制位置）
    QGraphicsProxyWidget* m_boxProxy;  // 设置框代理
    QGraphicsProxyWidget* m_logoProxy; // Logo 代理

    // ✅ 修改：语言切换改为按钮，而不是下拉框
    QPushButton* m_btnLang;

    // ✅ 新增：头像按钮（为了支持点击更换）
    QPushButton* m_btnAvatar;

    // ✅ 新增：空函数
    void onChangeAvatar();// 更换头像逻辑
    void onToggleLanguage(); // ✅ 切换语言逻辑

protected:
    void resizeEvent(QResizeEvent* event) override;

    void showEvent(QShowEvent* event) override; // ✅ 新增
    void hideEvent(QHideEvent* event) override; // ✅ 新增

    
};

#endif