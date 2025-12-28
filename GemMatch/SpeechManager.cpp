#include "SpeechManager.h"

#include <QThread>
#include <QMetaObject>
#include <QDebug>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Media.SpeechSynthesis.h>
#include <winrt/Windows.Media.Playback.h>
#include <winrt/Windows.Media.Core.h>

#pragma comment(lib, "windowsapp")

using namespace winrt;
using namespace winrt::Windows::Media::SpeechSynthesis;
using namespace winrt::Windows::Media::Playback;
using namespace winrt::Windows::Media::Core;

static std::wstring to_wstring(const QString& s) { return s.toStdWString(); }

// ================= Worker =================
class SpeechManager::TtsWorker : public QObject
{
    Q_OBJECT
public:
    TtsWorker() = default;

public slots:
    void initWinRt()
    {
        try {
            // ✅ 在专用线程中初始化 MTA（不要在 UI 线程 init）
            winrt::init_apartment(winrt::apartment_type::multi_threaded);

            // 预创建对象，避免第一次 speak 卡顿
            m_synth = SpeechSynthesizer();

            // 选 Yaoyao（找不到则回退 zh-CN 第一个）
            bool found = false;
            auto voices = SpeechSynthesizer::AllVoices();
            for (uint32_t i = 0; i < voices.Size(); ++i) {
                auto v = voices.GetAt(i);
                if (v.DisplayName() == L"Microsoft Yaoyao") {
                    m_synth.Voice(v);
                    found = true;
                    break;
                }
            }
            if (!found) {
                for (uint32_t i = 0; i < voices.Size(); ++i) {
                    auto v = voices.GetAt(i);
                    if (v.Language() == L"zh-CN") {
                        m_synth.Voice(v);
                        break;
                    }
                }
            }

            m_player = MediaPlayer();
        }
        catch (const winrt::hresult_error& e) {
            qWarning() << "[TTS] initWinRt failed:"
                << QString::number((int)e.code(), 16)
                << QString::fromWCharArray(e.message().c_str());
        }
    }

    void speakText(QString text)
    {
        try {
            if (!m_synth || !m_player) {
                initWinRt();
                if (!m_synth || !m_player) return;
            }

            // 合成
            auto stream = m_synth.SynthesizeTextToStreamAsync(to_wstring(text)).get();

            // 播放（固定 audio/wav，避免 ContentType 的坑）
            m_player.Source(MediaSource::CreateFromStream(stream, L"audio/wav"));
            m_player.Play();
        }
        catch (const winrt::hresult_error& e) {
            qWarning() << "[TTS] speak failed:"
                << QString::number((int)e.code(), 16)
                << QString::fromWCharArray(e.message().c_str());
        }
        catch (...) {
            qWarning() << "[TTS] speak failed: unknown exception";
        }
    }

private:
    SpeechSynthesizer m_synth{ nullptr };
    MediaPlayer m_player{ nullptr };
};

// ================= SpeechManager =================
SpeechManager& SpeechManager::instance()
{
    static SpeechManager inst;
    return inst;
}

SpeechManager::SpeechManager()
{
    auto* thread = new QThread();
    m_thread = thread;

    m_worker = new TtsWorker();
    m_worker->moveToThread(thread);

    QObject::connect(thread, &QThread::started, m_worker, &TtsWorker::initWinRt);
    QObject::connect(thread, &QThread::finished, m_worker, &QObject::deleteLater);

    thread->start();
}

SpeechManager::~SpeechManager()
{
    auto* thread = static_cast<QThread*>(m_thread);
    if (thread) {
        thread->quit();
        thread->wait();
        delete thread;
        m_thread = nullptr;
    }
}

void SpeechManager::speak(const QString& text)
{
    if (!m_worker) return;

    // ✅ UI线程只发消息，真正的 WinRT 调用都在 TTS 线程
    QMetaObject::invokeMethod(
        m_worker,
        "speakText",
        Qt::QueuedConnection,
        Q_ARG(QString, text)
    );
}

#include "SpeechManager.moc"
