#pragma once
#include <QWidget>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>

class VideoBackground : public QWidget {
    Q_OBJECT
public:
    // 构造函数
    explicit VideoBackground(const QString& videoPath, QWidget* parent = nullptr);
    ~VideoBackground();

    void play();
    void pause();
    void stop();

    void setVolume(int volume);

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