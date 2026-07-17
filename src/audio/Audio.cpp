#include "Audio.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <xaudio2.h>

static constexpr float pi = 3.14159265f;

static inline short toPCM(float v)
{
    v = std::clamp(v, -1.0f, 1.0f);
    return (short)(v * 32767.0f);
}

static inline float prand(unsigned int &s)
{
    s = (s * 2891050953u) + 1437714103u;
    return (float)(int)(s >> 8) * (1.0f / (float)(1 << 23));
}

static inline float lpfilter(float x, float &prev, float alpha)
{
    float y = ((1.0f - alpha) * x) + (alpha * prev);
    prev = y;
    return y;
}

void Audio::synthStep(Audio::SoundSet s, int v, std::vector<short> &out)
{
    const auto sr = (float)sampleRate;
    switch (s)
    {
        case SoundSet::Soil:
        {
            int n = (int)(0.180f * sr);
            out.resize(n);
            float k = 20.0f + ((float)v * 0.5f);
            float alpha = std::exp(-2.0f * pi * 400.0f / sr);
            float prev = 0.0f;
            unsigned int seed = 0xA5B3C7D9u + ((unsigned int)v * 0x12345u);
            for (int i = 0; i < n; i++)
            {
                float t = (float)i / sr;
                float env = std::exp(-k * t);
                float lp = lpfilter(prand(seed), prev, alpha);
                out[i] = toPCM(env * lp * 0.35f);
            }

            break;
        }

        case SoundSet::Grit:
        {
            int n = (int)(0.120f * sr);
            int click = (int)(0.003f * sr);
            out.resize(n);
            float alpha = std::exp(-2.0f * pi * 1200.0f / sr);
            float prev = 0.0f;
            unsigned int seed = 0xF1E2D3C4u + ((unsigned int)v * 0x56789u);
            for (int i = 0; i < n; i++)
            {
                float t = (float)i / sr;
                float noise = prand(seed);
                float lp = lpfilter(noise, prev, alpha);
                float hi = noise - (lp * 0.6f);
                float env;
                if (i < click)
                {
                    env = 1.0f;
                }
                else
                {
                    env = std::exp(-16.0f * (t - ((float)click / sr)));
                }

                out[i] = toPCM(env * hi * (0.45f + ((float)v * 0.02f)));
            }
            break;
        }

        case SoundSet::Stone:
        {
            int n = (int)(0.220f * sr);
            out.resize(n);
            float freq = 80.0f + ((float)v * 5.0f);
            float alpha = std::exp(-2.0f * pi * 300.0f / sr);
            float prev = 0.0f;
            float phase = 0.0f;
            float phInc = 2.0f * pi * freq / sr;
            unsigned int seed = 0xC3D4E5F6u + ((unsigned int)v * 0x98765u);
            for (int i = 0; i < n; i++)
            {
                float t = (float)i / sr;
                float env = std::exp(-12.0f * t);
                float sine = std::sin(phase);
                phase += phInc;
                float lp = lpfilter(prand(seed), prev, alpha);
                out[i] = toPCM(env * ((sine * 0.40f) + (lp * 0.15f)));
            }

            break;
        }

        case SoundSet::StoneHigh:
        {
            int n = (int)(0.080f * sr);
            out.resize(n);
            float freq = 380.0f + ((float)v * 15.0f);
            float phase = 0.0f;
            float phInc = 2.0f * pi * freq / sr;
            unsigned int seed = 0xB2C3D4E5u + ((unsigned int)v * 0x11111u);
            for (int i = 0; i < n; i++)
            {
                float t = (float)i / sr;
                float env = std::exp(-40.0f * t);
                float sine = std::sin(phase);
                phase += phInc;
                out[i] = toPCM(env * (sine + (prand(seed) * 0.1f)) * 0.40f);
            }

            break;
        }

        case SoundSet::Timber:
        {
            int n = (int)(0.200f * sr);
            out.resize(n);
            float freq = 160.0f + ((float)v * 12.5f);
            float alpha = std::exp(-2.0f * pi * 500.0f / sr);
            float prev = 0.0f;
            float phase = 0.0f;
            float phInc = 2.0f * pi * freq / sr;
            unsigned int seed = 0xD4E5F6A7u + ((unsigned int)v * 0x77777u);
            for (int i = 0; i < n; i++)
            {
                float t = (float)i / sr;
                float env = std::exp(-15.0f * t);
                float sine = std::sin(phase);
                phase += phInc;
                float lp = lpfilter(prand(seed), prev, alpha);
                out[i] = toPCM(env * ((sine * 0.35f) + (lp * 0.15f)));
            }

            break;
        }

        default:
            out.clear();
            break;
    }
}

void Audio::synthBreak(Audio::SoundSet s, int v, std::vector<short> &out)
{
    const auto sr = (float)sampleRate;
    switch (s)
    {
        case SoundSet::Soil:
        {
            int n = (int)(0.270f * sr);
            out.resize(n);
            float k = 14.0f + ((float)v * 0.5f);
            float alpha = std::exp(-2.0f * pi * 400.0f / sr);
            float prev = 0.0f;
            unsigned int seed = 0x9A8B7C6Du + ((unsigned int)v * 0x54321u);
            for (int i = 0; i < n; i++)
            {
                float t = (float)i / sr;
                float env = std::exp(-k * t);
                float lp = lpfilter(prand(seed), prev, alpha);
                out[i] = toPCM(env * lp * 0.50f);
            }

            break;
        }

        case SoundSet::Grit:
        {
            int n = (int)(0.180f * sr);
            int click = (int)(0.005f * sr);
            out.resize(n);
            float alpha = std::exp(-2.0f * pi * 1000.0f / sr);
            float prev = 0.0f;
            unsigned int seed = 0xE2D3C4B5u + ((unsigned int)v * 0xABCDEu);
            for (int i = 0; i < n; i++)
            {
                float t = (float)i / sr;
                float noise = prand(seed);
                float lp = lpfilter(noise, prev, alpha);
                float hi = noise - (lp * 0.5f);
                float env = (i < click) ? 1.0f : std::exp(-12.0f * (t - ((float)click / sr)));
                out[i] = toPCM(env * hi * 0.60f);
            }

            break;
        }

        case SoundSet::Stone:
        {
            int n = (int)(0.330f * sr);
            out.resize(n);
            float freq = 80.0f + ((float)v * 5.0f);
            float alpha = std::exp(-2.0f * pi * 250.0f / sr);
            float prev = 0.0f;
            float phase = 0.0f;
            float phInc = 2.0f * pi * freq / sr;
            unsigned int seed = 0xA1B2C3D4u + ((unsigned int)v * 0x13579u);
            for (int i = 0; i < n; i++)
            {
                float t = (float)i / sr;
                float env = std::exp(-9.0f * t);
                float sine = std::sin(phase);
                phase += phInc;
                float lp = lpfilter(prand(seed), prev, alpha);
                out[i] = toPCM(env * ((sine * 0.50f) + (lp * 0.20f)));
            }

            break;
        }

        case SoundSet::StoneHigh:
        {
            int n = (int)(0.120f * sr);
            out.resize(n);
            float freq = 380.0f + ((float)v * 15.0f);
            float phase = 0.0f;
            float phInc = 2.0f * pi * freq / sr;
            unsigned int seed = 0xC4D5E6F7u + ((unsigned int)v * 0x22222u);
            for (int i = 0; i < n; i++)
            {
                float t = (float)i / sr;
                float env = std::exp(-30.0f * t);
                float sine = std::sin(phase);
                phase += phInc;
                out[i] = toPCM(env * (sine + (prand(seed) * 0.15f)) * 0.55f);
            }

            break;
        }

        case SoundSet::Timber:
        {
            int n = (int)(0.300f * sr);
            out.resize(n);
            float freq = 160.0f + ((float)v * 12.5f);
            float alpha = std::exp(-2.0f * pi * 450.0f / sr);
            float prev = 0.0f;
            float phase = 0.0f;
            float phInc = 2.0f * pi * freq / sr;
            unsigned int seed = 0xE5F6A7B8u + ((unsigned int)v * 0x88888u);
            for (int i = 0; i < n; i++)
            {
                float t = (float)i / sr;
                float env = std::exp(-11.0f * t);
                float sine = std::sin(phase);
                phase += phInc;
                float lp = lpfilter(prand(seed), prev, alpha);
                out[i] = toPCM(env * ((sine * 0.45f) + (lp * 0.20f)));
            }

            break;
        }

        default:
            out.clear();
            break;
    }
}

void Audio::synthClick(std::vector<short> &out)
{
    const auto sr = (float)sampleRate;
    int n = (int)(0.050f * sr);
    out.resize(n);
    float phase = 0.0f;
    float phInc = 2.0f * pi * 1200.0f / sr;
    for (int i = 0; i < n; i++)
    {
        float t = (float)i / sr;
        float env = std::exp(-80.0f * t);
        out[i] = toPCM(std::sin(phase) * env * 0.60f);
        phase += phInc;
    }
}

void Audio::synthMusic(int track, std::vector<short> &out)
{
    static constexpr float dur = 30.0f;
    static constexpr float fadeIn = 2.0f;
    static constexpr float fadeOut = 3.0f;
    static constexpr float roots[4] = {110.0f, 98.0f, 130.8f, 87.3f};
    float root = roots[track];
    float freqs[4] = {root, root * (1.0f + 0.0027f), root * 2.0f * (1.0f - 0.0015f), root * 2.0f * (1.0f + 0.0018f),};
    static constexpr float amps[4] = {0.30f, 0.24f, 0.15f, 0.12f};
    float phases[4] = {};
    float phInc[4] = {};
    for (int j = 0; j < 4; j++)
    {
        phInc[j] = 2.0f * pi * freqs[j] / (float)sampleRate;
    }

    float tremoloRate = 0.07f + ((float)track * 0.007f);
    unsigned int seed = 0xC8B47F2Eu + ((unsigned int)track * 0x6E37u);
    float noisePrev = 0.0f;
    float noiseAlpha = std::exp(-2.0f * pi * 60.0f / (float)sampleRate);
    const auto sr = (float)sampleRate;
    int n = (int)(dur * sr);
    out.resize(n);
    for (int i = 0; i < n; i++)
    {
        float t = (float)i / sr;
        float osc = 0.0f;
        for (int j = 0; j < 4; j++)
        {
            osc += std::sin(phases[j]) * amps[j];
            phases[j] += phInc[j];
            if (phases[j] > 2.0f * pi)
            {
                phases[j] -= 2.0f * pi;
            }
        }

        float tremolo = 0.72f + (0.28f * std::sin(2.0f * pi * tremoloRate * t));
        osc *= tremolo;
        float noise = lpfilter(prand(seed) * 0.04f, noisePrev, noiseAlpha);
        float env = 1.0f;
        if (t < fadeIn)
        {
            env = t / fadeIn;
        }
        else if (t > dur - fadeOut)
        {
            env = (dur - t) / fadeOut;
        }

        out[i] = toPCM((osc + noise) * env * 0.48f);
    }
}

static WAVEFORMATEX makeWfx()
{
    WAVEFORMATEX wfx = {};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = (unsigned long)22050;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = 2;
    wfx.nAvgBytesPerSec = (unsigned long)(22050 * 2);
    wfx.cbSize = 0;
    return wfx;
}

void *Audio::makeVoice()
{
    auto *xa = static_cast<IXAudio2 *>(xaudio);
    WAVEFORMATEX wfx = makeWfx();
    IXAudio2SourceVoice *v = nullptr;
    if (FAILED(xa->CreateSourceVoice(&v, &wfx)))
    {
        return nullptr;
    }

    return v;
}

void *Audio::pickSfxVoice()
{
    for (auto &i : sfx)
    {
        auto *v = static_cast<IXAudio2SourceVoice *>(i);
        XAUDIO2_VOICE_STATE st{};
        v->GetState(&st);
        if (st.BuffersQueued == 0)
        {
            return v;
        }
    }

    auto *v = static_cast<IXAudio2SourceVoice*>(sfx[sfxNext]);
    v->Stop(0);
    v->FlushSourceBuffers();
    sfxNext = (sfxNext + 1) % sfxVoices;
    return v;
}

void Audio::submitOnce(void *voice, const std::vector<short> &buf)
{
    if (buf.empty())
    {
        return;
    }

    auto *v = static_cast<IXAudio2SourceVoice *>(voice);
    XAUDIO2_BUFFER xb = {};
    xb.AudioBytes = (unsigned int)(buf.size() * sizeof(short));
    xb.pAudioData = reinterpret_cast<const unsigned char *>(buf.data());
    xb.Flags = XAUDIO2_END_OF_STREAM;
    v->SubmitSourceBuffer(&xb);
    v->Start(0);
}

void Audio::startMusic(int track)
{
    musicPrequeued = false;
    auto *mv = static_cast<IXAudio2SourceVoice *>(music);
    mv->Stop(0);
    mv->FlushSourceBuffers();
    const auto &buf = musicBuf[track];
    if (buf.empty())
    {
        return;
    }

    XAUDIO2_BUFFER xb = {};
    xb.AudioBytes = (unsigned int)(buf.size() * sizeof(short));
    xb.pAudioData = reinterpret_cast<const unsigned char *>(buf.data());
    xb.Flags = XAUDIO2_END_OF_STREAM;
    mv->SubmitSourceBuffer(&xb);
    mv->Start(0);
}

Audio::SoundSet Audio::blockSound(BlockType t)
{
    switch (t)
    {
        case BlockType::Turf:
        case BlockType::Soil:
        case BlockType::Sapling:
        case BlockType::Pith:
        case BlockType::WeavePale:
        case BlockType::WeaveAsh:
        case BlockType::WeaveSlate:
        case BlockType::WeaveRust:
        case BlockType::WeaveBurn:
        case BlockType::WeaveGlow:
        case BlockType::WeaveBlight:
        case BlockType::WeaveMold:
        case BlockType::WeaveFern:
        case BlockType::WeaveFrost:
        case BlockType::WeaveAzure:
        case BlockType::WeaveDeep:
        case BlockType::WeaveDusk:
        case BlockType::WeaveMurk:
        case BlockType::WeaveBloom:
        case BlockType::WeaveBlush:
        case BlockType::Goldenbloom:
        case BlockType::Thornbloom:
        case BlockType::Dustshroom:
        case BlockType::Emberscap:
            return SoundSet::Soil;

        case BlockType::Grit:
        case BlockType::Rubble:
        case BlockType::Silt:
            return SoundSet::Grit;

        case BlockType::Stone:
        case BlockType::Bedrock:
        case BlockType::CharVein:
        case BlockType::IronVein:
        case BlockType::GoldVein:
            return SoundSet::Stone;

        case BlockType::Glaze:
        case BlockType::GoldBlock:
            return SoundSet::StoneHigh;

        case BlockType::Timber:
        case BlockType::Boards:
            return SoundSet::Timber;

        default:
            return SoundSet::None;
    }
}

void Audio::init()
{
    if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
    {
        comInit = true;
    }

    typedef HRESULT (WINAPI *PFN_XAudio2Create)(IXAudio2 **, unsigned int, XAUDIO2_PROCESSOR);
    HMODULE dll = LoadLibraryA("XAudio2_9.dll");
    if (!dll)
    {
        dll = LoadLibraryA("XAudio2_8.dll");
    }

    if (!dll)
    {
        std::cerr << "Audio: XAudio2 DLL not found" << std::endl;
        return;
    }

    auto pfnCreate = reinterpret_cast<PFN_XAudio2Create>(GetProcAddress(dll, "XAudio2Create"));
    if (!pfnCreate)
    {
        std::cerr << "Audio: XAudio2Create not found in DLL" << std::endl;
        FreeLibrary(dll);
        return;
    }

    xaDll = dll;
    IXAudio2 *xa = nullptr;
    if (FAILED(pfnCreate(&xa, 0, XAUDIO2_DEFAULT_PROCESSOR)))
    {
        std::cerr << "Audio: XAudio2Create failed" << std::endl;
        FreeLibrary(dll);
        xaDll = nullptr;
        return;
    }

    xaudio = xa;
    IXAudio2MasteringVoice *iMaster = nullptr;
    if (FAILED(xa->CreateMasteringVoice(&iMaster)))
    {
        std::cerr << "Audio: CreateMasteringVoice failed" << std::endl;
        xa->Release();
        xaudio = nullptr;
        FreeLibrary(dll);
        xaDll = nullptr;
        return;
    }

    master = iMaster;
    for (int i = 0; i < sfxVoices; i++)
    {
        sfx[i] = makeVoice();
        if (!sfx[i])
        {
            std::cerr << "Audio: CreateSourceVoice failed (sfx " << i << ')' << std::endl;
            for (int j = 0; j < i; j++)
            {
                static_cast<IXAudio2SourceVoice *>(sfx[j])->DestroyVoice();
            }

            iMaster->DestroyVoice();
            xa->Release();
            master = nullptr;
            xaudio = nullptr;
            FreeLibrary(dll);
            xaDll = nullptr;
            return;
        }
    }

    music = makeVoice();
    if (!music)
    {
        std::cerr << "Audio: CreateSourceVoice failed (music)" << std::endl;
        for (auto &i : sfx)
        {
            static_cast<IXAudio2SourceVoice *>(i)->DestroyVoice();
        }

        iMaster->DestroyVoice();
        xa->Release();
        master = nullptr;
        xaudio = nullptr;
        FreeLibrary(dll);
        xaDll = nullptr;
        return;
    }

    audioOk = true;
    for (int s = 0; s < 5; s++)
    {
        for (int v = 0; v < variants; v++)
        {
            synthStep(static_cast<SoundSet>(s), v, stepBuf[s][v]);
            synthBreak(static_cast<SoundSet>(s), v, breakBuf[s][v]);
        }
    }

    synthClick(clickBuf);
    for (int t = 0; t < musicTracks; t++)
    {
        synthMusic(t, musicBuf[t]);
    }

    startMusic(0);
}

void Audio::shutdown()
{
    if (!audioOk)
    {
        return;
    }

    audioOk = false;
    if (music)
    {
        auto *mv = static_cast<IXAudio2SourceVoice *>(music);
        mv->Stop(0);
        mv->DestroyVoice();
        music = nullptr;
    }

    for (auto &i : sfx)
    {
        if (i)
        {
            auto *v = static_cast<IXAudio2SourceVoice *>(i);
            v->Stop(0);
            v->DestroyVoice();
            i = nullptr;
        }
    }

    if (master)
    {
        static_cast<IXAudio2MasteringVoice *>(master)->DestroyVoice();
        master = nullptr;
    }

    if (xaudio)
    {
        static_cast<IXAudio2 *>(xaudio)->Release();
        xaudio = nullptr;
    }

    if (xaDll)
    {
        FreeLibrary(static_cast<HMODULE>(xaDll));
        xaDll = nullptr;
    }

    if (comInit)
    {
        CoUninitialize();
        comInit = false;
    }
}

void Audio::queueFootstep(Audio::SoundSet s)
{
    if (!audioOk || s == SoundSet::None)
    {
        return;
    }

    pendingQueue.push_back({s, false});
}

void Audio::queueBreak(Audio::SoundSet s)
{
    if (!audioOk || s == SoundSet::None)
    {
        return;
    }

    pendingQueue.push_back({s, true});
}

void Audio::flushSounds()
{
    if (!audioOk)
    {
        return;
    }

    for (auto &ps : pendingQueue)
    {
        int idx = (int)ps.s;
        if (ps.isBreak)
        {
            int v = breakVar[idx];
            breakVar[idx] = (v + 1) % variants;
            submitOnce(pickSfxVoice(), breakBuf[idx][v]);
        }
        else
        {
            int v = stepVar[idx];
            stepVar[idx] = (v + 1) % variants;
            submitOnce(pickSfxVoice(), stepBuf[idx][v]);
        }
    }

    pendingQueue.clear();
}

void Audio::tickMusic()
{
    if (!audioOk || !music)
    {
        return;
    }

    auto *mv = static_cast<IXAudio2SourceVoice *>(music);
    XAUDIO2_VOICE_STATE st{};
    mv->GetState(&st);
    if (st.BuffersQueued == 0)
    {
        musicTrack = (musicTrack + 1) % musicTracks;
        startMusic(musicTrack);
    }
    else if (st.BuffersQueued == 1 && !musicPrequeued)
    {
        musicTrack = (musicTrack + 1) % musicTracks;
        const auto &buf = musicBuf[musicTrack];
        if (!buf.empty())
        {
            XAUDIO2_BUFFER xb = {};
            xb.AudioBytes = (unsigned int)(buf.size() * sizeof(short));
            xb.pAudioData = reinterpret_cast<const unsigned char *>(buf.data());
            xb.Flags = XAUDIO2_END_OF_STREAM;
            mv->SubmitSourceBuffer(&xb);
            mv->Start(0);
            musicPrequeued = true;
        }
    }
    else if (st.BuffersQueued >= 2)
    {
        musicPrequeued = false;
    }
}

void Audio::toggleMute()
{
    if (!audioOk)
    {
        return;
    }

    muted = !muted;
    static_cast<IXAudio2MasteringVoice *>(master)->SetVolume(muted ? 0.0f : 1.0f);
}