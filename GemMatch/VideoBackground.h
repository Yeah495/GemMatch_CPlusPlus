#pragma once
#include <QWidget>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>

class VideoBackground : public QWidget {
    Q_OBJECT
public:
    // ✅ 修正：构造函数包含 videoPath 参数
    explicit VideoBackground(const QString& videoPath, QWidget* parent = nullptr);
    ~VideoBackground();

    void play();
    void pause();
    void stop();
    // ✅ 修正：声明 setVolume 方法
    void setVolume(int volume); // 0-100

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onErrorOccurred(QMediaPlayer::Error error, const QString& errorString);

private:
    QMediaPlayer* m_player;
    QVideoWidget* m_videoWidget;
    QAudioOutput* m_audioOutput;
};