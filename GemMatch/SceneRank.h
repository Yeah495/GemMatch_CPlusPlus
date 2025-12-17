#pragma once
#ifndef SCENERANK_H
#define SCENERANK_H

#include <QWidget>
#include <QTableWidget>

class MainWindow;

class SceneRank : public QWidget {
    Q_OBJECT
public:
    explicit SceneRank(MainWindow* mainWin);

protected:
    // 每次显示时刷新数据
    void showEvent(QShowEvent* event) override;

private:
    MainWindow* m_mainWin;
    QTableWidget* m_table;

    void setupUI();
    void loadRankData();
};

#endif // SCENERANK_H