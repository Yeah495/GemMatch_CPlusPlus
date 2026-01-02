#include "VideoBackground.h"
#include <QVBoxLayout>
#include <QFile>
#include <QUrl>
#include <QDebug>

VideoBackground::VideoBackground(const QString& videoPath, QWidget* parent)
    : QWidget(parent)
{
    //强制让 VideoBackground 变成一个 原生窗口
    this->setAttribute(Qt::WA_NativeWindow);

    //创建媒体播放器
    m_player = new QMediaPlayer(this);

    ////创建音频输出
    //m_audioOutput = new QAudioOutput(this);
    //m_player->setAudioOutput(m_audioOutput);

    //创建视频显示控件
    m_videoWidget = new QVideoWidget(this);
    m_player->setVideoOutput(m_videoWidget);

    //设置布局
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_videoWidget);

    //加载视频文件
    if (QFile::exists(videoPath)) {
        m_player->setSource(QUrl::fromLocalFile(videoPath));
        // qDebug() << "Video loaded:" << videoPath;
    }
    else {
        qDebug() << "Error: Video file not found:" << videoPath;
    }

    //设置循环播放逻辑
    connect(m_player, &QMediaPlayer::mediaStatusChanged,
        this, &VideoBackground::onMediaStatusChanged);

    //错误处理
    connect(m_player, &QMediaPlayer::errorOccurred,
        this, &VideoBackground::onErrorOccurred);

    //默认设置
    m_audioOutput->setVolume(0.0f); // 默认静音
    m_videoWidget->setAspectRatioMode(Qt::IgnoreAspectRatio); // 填满屏幕，
    m_player->setLoops(QMediaPlayer::Infinite); // 设置无限循环

    //开始播放
    m_player->play();
}

VideoBackground::~VideoBackground() {
    if (m_player) {
        m_player->stop();
    }
}

void VideoBackground::play() {
    if (m_player) m_player->play();
}

void VideoBackground::pause() {
    if (m_player) m_player->pause();
}

void VideoBackground::stop() {
    if (m_player) m_player->stop();
}

// 实现 setVolume
void VideoBackground::setVolume(int volume) {
    if (m_audioOutput) {
        // Qt6 volume is 0.0 to 1.0
        float linearVolume = qBound(0, volume, 100) / 100.0f;
        m_audioOutput->setVolume(linearVolume);
    }
}

void VideoBackground::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    //确保内部的 VideoWidget 跟随父窗口大小变化（虽然 Layout 会处理，但双重保险）
    if (m_videoWidget) {
        m_videoWidget->setGeometry(rect());
    }
}

void VideoBackground::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    //手动循环保障：如果播放结束，重置进度并播放
    if (status == QMediaPlayer::EndOfMedia) {
        m_player->setPosition(0);
        m_player->play();
    }
}

void VideoBackground::onErrorOccurred(QMediaPlayer::Error error, const QString& errorString) {
    qDebug() << "Video Error:" << errorString << "Code:" << error;
}