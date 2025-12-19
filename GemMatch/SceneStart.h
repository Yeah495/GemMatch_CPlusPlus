#pragma once
#ifndef SCENESTART_H
#define SCENESTART_H

#include <QWidget>

class MainWindow;

class SceneStart : public QWidget {
    Q_OBJECT
public:
    explicit SceneStart(MainWindow* mainWin);

private:
    MainWindow* m_mainWin;
    void setupUI();

protected:
    void paintEvent(QPaintEvent* event) override;
};

#endif // SCENESTART_H