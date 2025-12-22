#include "SoundManager.h"
#include <QUrl>

SoundManager& SoundManager::instance() {
    static SoundManager inst;
    return inst;
}

SoundManager::SoundManager(QObject* parent)
    : QObject(parent)
{
    m_bgPlayer = new QMediaPlayer(this);
    m_bgOutput = new QAudioOutput(this);
    m_bgPlayer->setAudioOutput(m_bgOutput);

    m_clickEffect = new QSoundEffect(this);
    m_mouthEffect = new QSoundEffect(this);

    // 默认音量
    m_bgOutput->setVolume(0.5);
    m_clickEffect->setVolume(0.9);
    m_mouthEffect->setVolume(0.9);

    // 预加载资源路径（从 qrc 加载）
    m_clickEffect->setSource(QUrl("qrc:/assets/sounds/click.wav"));
    m_mouthEffect->setSource(QUrl("qrc:/assets/sounds/mouth.wav"));
}

void SoundManager::playBackground(const QString& path) {
    if (!m_bgPlayer) return;
    m_bgPlayer->stop();
    // 支持 qrc 路径和本地路径
    QUrl url = path.startsWith("qrc:") || path.startsWith(":/") ? QUrl(path) : QUrl::fromLocalFile(path);
    m_bgPlayer->setSource(url);
    m_bgPlayer->setLoops(QMediaPlayer::Infinite);
    m_bgPlayer->play();
}

void SoundManager::stopBackground() {
    if (!m_bgPlayer) return;
    m_bgPlayer->stop();
    m_bgPlayer->setSource(QUrl());
}

void SoundManager::setBackgroundVolume(qreal volume) {
    if (m_bgOutput) {
        qreal v = qMax<qreal>(0.0, qMin<qreal>(1.0, volume));
        m_bgOutput->setVolume(v);
    }
}

void SoundManager::playClick() {
    if (m_clickEffect) {
        // 已预加载 qrc
        m_clickEffect->play();
    }
}

void SoundManager::playMouth() {
    if (m_mouthEffect) {
        m_mouthEffect->play();
    }
}
