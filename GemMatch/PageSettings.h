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

private slots:
    void onMusicVolumeChanged(int value);

private:
    MainWindow* m_mainWin;
    void setupUI();

    QLabel* m_labelTitle;
    QLabel* m_labelMusic;
    QLabel* m_labelBrightness;

    QSlider* m_musicSlider;

    GameButton* m_btnBack;
    GameButton* m_btnReLogin;

    QGraphicsView* m_view;
    QGraphicsVideoItem* m_videoItem;
    QMediaPlayer* m_player;
    QAudioOutput* m_audioOutput;

    QString m_videoPath;
    GameLogo* m_logo;
    QGraphicsProxyWidget* m_boxProxy;
    QGraphicsProxyWidget* m_logoProxy;
    QPushButton* m_btnLang;
    QPushButton* m_btnAvatar;

    void onChangeAvatar();
    void onToggleLanguage();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
};

#endif#endif