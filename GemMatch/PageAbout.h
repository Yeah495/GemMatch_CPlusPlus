#pragma once
#ifndef PAGEABOUT_H
#define PAGEABOUT_H

#include <QWidget>

class MainWindow;

class PageAbout : public QWidget {
    Q_OBJECT
public:
    explicit PageAbout(MainWindow* mainWin);
private:
    MainWindow* m_mainWin;
    void setupUI();
};

#endif // PAGEABOUT_H