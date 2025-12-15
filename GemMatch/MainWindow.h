#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include "SceneGame.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    SceneGame* getGameScene() { return m_sceneGame; }

private:
    void setupUI();

    QGraphicsView* m_view;
    SceneGame* m_sceneGame;
};

#endif // MAINWINDOW_H