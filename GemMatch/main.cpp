#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include <QMediaPlayer>
#include <QAudioOutput>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 1. 创建主窗口 (View)
    MainWindow w;
    w.show();



    // 设置并播放背景音乐（从 qrc 资源）
    QMediaPlayer* bgPlayer = new QMediaPlayer(&a);
    QAudioOutput* bgOutput = new QAudioOutput(&a);
    bgPlayer->setAudioOutput(bgOutput);
    bgOutput->setVolume(0.5); // 默认 50%
    bgPlayer->setLoops(QMediaPlayer::Infinite);
    // 使用 qrc 路径，注意 GemMatch.qrc 中需要包含对应条目
    bgPlayer->setSource(QUrl("qrc:/assets/sound/bgpiano.wav"));
    bgPlayer->play();

    // 退出时停止并释放（parent 为 QApplication 会在退出时删除）
    QObject::connect(&a, &QApplication::aboutToQuit, [bgPlayer, bgOutput]() {
        if (bgPlayer) {
            bgPlayer->stop();
            bgPlayer->setSource(QUrl());
        }
        if (bgOutput) {
            bgOutput->setVolume(0.0);
        }
    });

    return a.exec();
}
