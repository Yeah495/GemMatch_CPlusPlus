#ifndef SCENEGAME_H
#define SCENEGAME_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QLCDNumber>
#include <QLabel>
#include <QPushButton>
#include <functional>
#include <QGraphicsVideoItem> // 新增：视频图元
#include <QMediaPlayer>       // 新增：播放器
#include <QAudioOutput>       // 新增：音频输出
#include "Board.h" // 引用你的 Board 数据结构
#include "GemItem.h" // 引用宝石图元

class MainWindow; // 前向声明

class SceneGame : public QWidget {
    Q_OBJECT

public:
    explicit SceneGame(MainWindow* mainWin);
    ~SceneGame();

    // --- 供 Controller 调用的核心接口 ---

    // 渲染棋盘：根据 Model 数据生成宝石图元
    void renderBoard(const Board& board);

    // 动画：交换
    void animateSwap(int r1, int c1, int r2, int c2, std::function<void()> finishedCallback);
    // 动画：消除
    void animateExplosion(const std::vector<QPoint>& points, std::function<void()> finishedCallback);
    // 动画：下落
    void animateFall(const Board& newBoard, std::function<void()> finishedCallback);

    // 设置高亮选中
    void setGemSelected(int r, int c, bool selected);

    // 更新 UI 显示
    void updateScore(int score);
    void updateTime(int seconds);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override; // 新增：窗口大小改变事件

signals:
    // 转发宝石点击信号给 Controller (row, col)
    void gemClicked(int row, int col);

    // 这里的信号供 MainWindow 切换页面使用
    void backToMenu();

private:
    MainWindow* m_mainWin;

    // --- 背景视频组件 (新增) ---
    QGraphicsView* m_bgView;      // 背景视图（最底层）
    QGraphicsScene* m_bgScene;    // 背景场景
    QGraphicsVideoItem* m_videoItem;
    QMediaPlayer* m_player;
    QAudioOutput* m_audioOutput;

    // --- 游戏核心图形视图 ---
    QGraphicsView* m_view;   // 负责显示棋盘的窗口 (现在嵌入在背景视图中)
    QGraphicsScene* m_scene; // 负责管理宝石的场景

    // 宝石图元指针数组 (方便通过坐标找图元)
    GemItem* m_items[8][8];

    // --- 右侧 UI 控件 ---
    QLCDNumber* m_scoreDisplay;
    QLabel* m_timeLabel;
    QPushButton* m_btnSkillBomb;
    QPushButton* m_btnSkillShuffle;
    QPushButton* m_btnSkillTime;
    QPushButton* m_btnPause;
    QPushButton* m_btnExit;

    // --- 内部辅助 ---
    void setupUI();
    QPointF getScreenPos(int row, int col) const; // 计算宝石在 Scene 中的坐标

    const int CELL_SIZE = 65; // 格子大小
};

#endif // SCENEGAME_H