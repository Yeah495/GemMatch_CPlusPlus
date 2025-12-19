#ifndef PAGESETTINGS_H
#define PAGESETTINGS_H

#include <QWidget>
#include <QSlider>
#include <QLabel>

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
};

#endif