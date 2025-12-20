#ifndef PAGESETTINGS_H
#define PAGESETTINGS_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
// ✅ 在文件开头添加
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QGraphicsProxyWidget>    

class MainWindow;

class PageSettings : public QWidget {
    Q_OBJECT
public:
    explicit PageSettings(MainWindow* mainWin);

private:
    MainWindow* m_mainWin;
    void setupUI();

    QLabel* m_labelTitle;
    QLabel* m_labelMusic;
    QLabel* m_labelBrightness;

    QGraphicsView* m_view;
    QGraphicsVideoItem* m_videoItem;
    QMediaPlayer* m_player;
    QAudioOutput* m_audioOutput;

protected:
    void resizeEvent(QResizeEvent* event) override;
};

#endif