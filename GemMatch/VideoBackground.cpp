#include "VideoBackground.h"
#include <QVBoxLayout>
#include <QFile>
#include <QUrl>
#include <QDebug>

VideoBackground::VideoBackground(const QString& videoPath, QWidget* parent)
    : QWidget(parent)
{
    // 【✅ 关键】让 VideoBackground 外壳也变成原生窗口
    // 这样 PageLogin 面对的就是两个平等的原生子窗口（VideoBackground 和 Container）
    // 此时 stackUnder 生效的概率最大。
    this->setAttribute(Qt::WA_NativeWindow);

    // 1. 创建媒体播放器
    m_player = new QMediaPlayer(this);

    // 2. 创建音频输出（Qt 6 必需）
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);

    // 3. 创建视频显示控件
    m_videoWidget = new QVideoWidget(this);
    m_player->setVideoOutput(m_videoWidget);

    // 4. 设置布局（让视频填满整个 VideoBackground 控件）
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_videoWidget);

    // 5. 加载视频文件
    if (QFile::exists(videoPath)) {
        m_player->setSource(QUrl::fromLocalFile(videoPath));
        // qDebug() << "Video loaded:" << videoPath;
    }
    else {
        qDebug() << "Error: Video file not found:" << videoPath;
    }

    // 6. 设置循环播放逻辑
    connect(m_player, &QMediaPlayer::mediaStatusChanged,
        this, &VideoBackground::onMediaStatusChanged);

    // 7. 错误处理
    connect(m_player, &QMediaPlayer::errorOccurred,
        this, &VideoBackground::onErrorOccurred);

    // 8. 默认设置
    m_audioOutput->setVolume(0.0f); // 默认静音
    m_videoWidget->setAspectRatioMode(Qt::IgnoreAspectRatio); // 填满屏幕，可能拉伸
    m_player->setLoops(QMediaPlayer::Infinite); // 设置无限循环 (Qt 6.x 部分版本支持)

    // 9. 开始播放
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

// ✅ 实现 setVolume
void VideoBackground::setVolume(int volume) {
    if (m_audioOutput) {
        // Qt6 volume is 0.0 to 1.0
        float linearVolume = qBound(0, volume, 100) / 100.0f;
        m_audioOutput->setVolume(linearVolume);
    }
}

void VideoBackground::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    // 确保内部的 VideoWidget 跟随父窗口大小变化（虽然 Layout 会处理，但双重保险）
    if (m_videoWidget) {
        m_videoWidget->setGeometry(rect());
    }
}

void VideoBackground::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    // 手动循环保障：如果播放结束，重置进度并播放
    if (status == QMediaPlayer::EndOfMedia) {
        m_player->setPosition(0);
        m_player->play();
    }
}

void VideoBackground::onErrorOccurred(QMediaPlayer::Error error, const QString& errorString) {
    qDebug() << "Video Error:" << errorString << "Code:" << error;
}