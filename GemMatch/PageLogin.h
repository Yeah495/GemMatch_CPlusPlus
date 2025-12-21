#ifndef PAGELOGIN_H
#define PAGELOGIN_H

#include <QWidget>
#include <QLineEdit>
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QMediaPlayer>
#include <QAudioOutput>
#include "GameButton.h"
#include "GameLogo.h"

class MainWindow;

class PageLogin : public QWidget {
    Q_OBJECT
public:
    explicit PageLogin(MainWindow* mainWin);

private:
    MainWindow* m_mainWin;
    QLineEdit* m_editUser;
    QLineEdit* m_editPass;
    QLineEdit* m_editEmail;

    // 使用 Graphics View 框架
    QGraphicsView* m_view;
    QGraphicsVideoItem* m_videoItem;
    QMediaPlayer* m_player;
    QAudioOutput* m_audioOutput;

    QString m_videoPath;

    // 修改旧的 QLabel/QPushButton 为新类
    GameLogo* m_logo;       // 原 title
    GameButton* m_btnLogin; // 原 btnLogin
    GameButton* m_btnReg;   // 原 btnReg


    // --- 关键修改：保存两个代理对象，以便独立控制位置 ---
    QGraphicsProxyWidget* m_loginBoxProxy; // 登录框（输入框+按钮）的代理
    QGraphicsProxyWidget* m_logoProxy;     // Logo 的独立代理



    void setupUI();
    void onLoginClicked();
    void onRegisterClicked();

protected:
    void resizeEvent(QResizeEvent* event) override;

    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

    // 新增调试：鼠标点击事件
    void mousePressEvent(QMouseEvent* event) override;
};

#endif
