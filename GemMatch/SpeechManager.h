#pragma once
#include <QString>

class SpeechManager
{
public:
    static SpeechManager& instance();
    void speak(const QString& text);

private:
    SpeechManager();
    ~SpeechManager();

    SpeechManager(const SpeechManager&) = delete;
    SpeechManager& operator=(const SpeechManager&) = delete;

    class TtsWorker;
    TtsWorker* m_worker = nullptr;
    void* m_thread = nullptr; // 避免在头文件引入 QThread（也可以直接写 QThread*）
};
