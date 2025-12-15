#include "MainWindow.h"
#include "GameController.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 1. 创建主窗口 (View)
    MainWindow w;
    w.show();

    // 2. 创建控制器 (Controller) 并绑定 View
    // Controller 内部会自动创建 Model
    GameController controller(&w);

    // 3. 启动游戏
    // 在实际项目中，这通常由 UI 上的“开始游戏”按钮触发
    controller.startGame();

    return a.exec();
    
}
