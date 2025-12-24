#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QFile>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    // 设置应用程序信息
    a.setApplicationName("SQLite 数据库查看器");
    a.setOrganizationName("MyCompany");

    // 设置样式（可选）
    a.setStyle(QStyleFactory::create("Fusion"));

    // 加载样式表（可选）
    QFile styleFile(":/styles/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        a.setStyleSheet(styleSheet);
    }

    MainWindow w;
    w.show();

    return a.exec();
}