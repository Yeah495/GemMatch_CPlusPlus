#ifndef SCENEGAME_H
#define SCENEGAME_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QLCDNumber>
#include <QLabel>
#include <QPushButton>
#include <functional>
#include <QGraphicsVideoItem> 
#include <QMediaPlayer>       
#include <QAudioOutput> 
#include <QGraphicsProxyWidget> // 新增

#include "Board.h" 
#include "GemItem.h" 
#include "GameButton.h" // ✅ 引入

class MainWindow;

class SceneGame : public QWidget {
    Q_OBJECT

public:
    explicit SceneGame(MainWindow* mainWin);
    ~SceneGame();

    void renderBoard(const Board& board);
    void animateSwap(int r1, int c1, int r2, int c2, std::function<void()> finishedCallback);
    void animateExplosion(const std::vector<QPoint>& points, std::function<void()> finishedCallback);
    void animateFall(const Board& newBoard, std::function<void()> finishedCallback);
    void setGemSelected(int r, int c, bool selected);
    void updateScore(int score);

    void setPauseButtonText(const QString& path);
    void updateTime(int seconds, bool isFrozen = false);

    void updateSkillButtonText(int bombCount, int shuffleCount, int timeCount);
    void startShakeAnimation();
protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

signals:
    void gemClicked(int row, int col);
    void backToMenu();

    void pauseGame();       // 暂停信号
    void skillBomb();       // 炸弹技能信号
    void skillShuffle();    // 洗牌技能信号
    void skillTime();       // 时间技能信号
    // 【新增】提示信号
    void hintRequested();



private:
    MainWindow* m_mainWin;

    // 背景视频
    QGraphicsView* m_bgView;
    QGraphicsScene* m_bgScene;
    QGraphicsVideoItem* m_videoItem;
    QMediaPlayer* m_player;
    QAudioOutput* m_audioOutput;

    // 游戏棋盘视图 (保持不动)
    QGraphicsView* m_view;
    QGraphicsScene* m_scene;
    GemItem* m_items[8][8];

    // --- 右侧 UI 控件 (升级) ---
    QLCDNumber* m_scoreDisplay;
    QLabel* m_timeLabel;

    // 技能按键 (改为 GameButton)
    GameButton* m_btnSkillBomb;
    GameButton* m_btnSkillShuffle;
    GameButton* m_btnSkillTime;
    GameButton* m_btnSkill4; // 第4个技能


    // 【新增】
    GameButton* m_btnHint; // 提示按键
    // 系统按键
    GameButton* m_btnPause;
    GameButton* m_btnExit;



    QString m_videoPath;

    QGraphicsProxyWidget* m_boardProxy;      // 棋盘代理
    QGraphicsProxyWidget* m_rightPanelProxy; // 右侧面板代理

    void setupUI();
    QPointF getScreenPos(int row, int col) const;
    const int CELL_SIZE = 65;

    QLabel* m_avatarLabel;// 新增：游戏界面头像
    // 【新增】提示按钮点击
    void onHintClicked();
};

#endif // SCENEGAME_H