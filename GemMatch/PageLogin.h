#ifndef PAGELOGIN_H
#define PAGELOGIN_H

#include <QWidget>
#include <QLineEdit>
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QMediaPlayer>
#include <QAudioOutput>

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



    void setupUI();
    void onLoginClicked();
    void onRegisterClicked();

protected:
    void resizeEvent(QResizeEvent* event) override;

    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
};

#endif
