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
#include <QGraphicsProxyWidget>
#include <QVariantAnimation>
#include "Board.h" 
#include "GemItem.h" 
#include "GameButton.h"


class MainWindow;

class SceneGame : public QWidget {
    Q_OBJECT

public:
    explicit SceneGame(MainWindow* mainWin);
    ~SceneGame();

    void renderBoard(const Board& board); //画宝石盘

    //交换,消除,下落,震动,提示,新纪录动画
    void animateSwap(int r1, int c1, int r2, int c2, std::function<void()> finishedCallback);
    void animateExplosion(const std::vector<QPoint>& points, std::function<void()> finishedCallback);
    void animateFall(const Board& newBoard, std::function<void()> finishedCallback);
    void startShakeAnimation();
    void showHintAnimation(const QPoint& p1, const QPoint& p2);  //显示提示动画
    void stopHintAnimation(); // 停止提示动画
    void playNewRecordAnimation(int recordType, std::function<void()> callback);
    void hideRecordAnimation();  //去除新纪录显示

    //选中宝石效果
    void setGemSelected(int r, int c, bool selected);

    void updateScore(int score);  //更新分数
    void updateTime(int seconds, bool isFrozen = false);  //更新时间
    void updateSkillButtonText(int bombCount, int shuffleCount, int timeCount, int allCount);  //更新技能次数

    //暂停按钮状态
    void setPauseButtonText(const QString& path);
    void setPauseButtonEnabled(bool enabled);
protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
signals:
    void gemClicked(int row, int col);  //宝石点击信号
    void backToMenu(); //返回主菜单信号

    void pauseGame();       // 暂停信号
    void skillBomb();       // 炸弹技能信号
    void skillShuffle();    // 洗牌技能信号
    void skillTime();       // 时间技能信号
    void skillAll();       // 万能技能信号
    void hintRequested();  //提示信号
private:
    const int CELL_SIZE = 65;
    void setupUI();
    //主窗口指针
    MainWindow* m_mainWin;

    // 背景视频
    QGraphicsView* m_bgView;
    QGraphicsScene* m_bgScene;
    QGraphicsVideoItem* m_videoItem;
    QMediaPlayer* m_player;
    QAudioOutput* m_audioOutput;
    QString m_videoPath;

    //游戏棋盘视图
    QGraphicsView* m_view;
    QGraphicsScene* m_scene;
    GemItem* m_items[8][8];

    //头像
    QLabel* m_avatarLabel;

    //UI 控件
    QLCDNumber* m_scoreDisplay;
    QLabel* m_timeLabel;

    //技能按键
    GameButton* m_btnSkillBomb;
    GameButton* m_btnSkillShuffle;
    GameButton* m_btnSkillTime;
    GameButton* m_btnSkillAll;

    //系统按键
    GameButton* m_btnHint;
    GameButton* m_btnPause;
    GameButton* m_btnExit;

    //组件容器
    QGraphicsProxyWidget* m_boardProxy;      
    QGraphicsProxyWidget* m_rightPanelProxy; 
    
	//提示组件
    QList<QAbstractAnimation*> m_hintAnims;

    //新纪录组件
    QGraphicsPixmapItem* m_recordItem = nullptr;

    QPointF getScreenPos(int row, int col) const; //得到宝石坐标对应的屏幕位置
};

#endif // SCENEGAME_H