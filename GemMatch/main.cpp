#include "MainWindow.h"
#include "GameController.h"
#include <QtWidgets/QApplication>
#include <QMediaPlayer>
#include <QAudioOutput>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 1. 创建主窗口 (View)
    MainWindow w;
    w.show();

    // 设置并播放背景音乐（本地文件）
    QMediaPlayer* bgPlayer = new QMediaPlayer(&a);
    QAudioOutput* bgOutput = new QAudioOutput(&a);
    bgPlayer->setAudioOutput(bgOutput);
    bgOutput->setVolume(0.5); // 默认 50%
    bgPlayer->setLoops(QMediaPlayer::Infinite);
    bgPlayer->setSource(QUrl::fromLocalFile("D:/GemMatch_CPlusPlus/GemMatch/assets/sound/bgpiano.wav"));
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

    // 2. 创建控制器 (Controller) 并绑定 View
    // Controller 内部会自动创建 Model
    GameController controller(&w);

    // 3. 启动游戏
    controller.startGame();

    return a.exec();
}
