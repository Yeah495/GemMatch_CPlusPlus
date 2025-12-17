#pragma once
#ifndef PAGESETTINGS_H
#define PAGESETTINGS_H

#include <QWidget>

class MainWindow;

class PageSettings : public QWidget {
    Q_OBJECT
public:
    explicit PageSettings(MainWindow* mainWin);
private:
    MainWindow* m_mainWin;
    void setupUI();
};

#endif // PAGESETTINGS_H