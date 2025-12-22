#pragma once

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QSoundEffect>
#include <QString>
#include <memory>
#include <QMutex>

// 简单的单例声音管理器：用于播放背景音乐与短音效
class SoundManager : public QObject {
    Q_OBJECT
public:
    static SoundManager& instance();

    // 背景音乐
    void playBackground(const QString& path); // path may be qrc like :/assets/sounds/bg_music.mp3
    void stopBackground();
    void setBackgroundVolume(qreal volume); // 0.0 - 1.0

    // 短音效（点击、消除等）
    void playClick();
    void playMouth();

private:
    explicit SoundManager(QObject* parent = nullptr);
    // 禁用复制
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    QMediaPlayer* m_bgPlayer;
    QAudioOutput* m_bgOutput;

    QSoundEffect* m_clickEffect;
    QSoundEffect* m_mouthEffect;

    QMutex m_mutex;
};
