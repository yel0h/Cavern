#ifndef CAVERN_AUDIO_HPP
#define CAVERN_AUDIO_HPP
#include "src/world/Block.hpp"
#include <vector>

class Audio
{
public:
    enum class SoundSet {Soil, Grit, Stone, StoneHigh, Timber, None};

    static SoundSet blockSound(BlockType t);

    void init();

    void shutdown();

    void queueFootstep(SoundSet s);

    void queueBreak(SoundSet s);

    void flushSounds();

    void tickMusic();

    void setSfxEnabled(bool enabled);

    void setMusicEnabled(bool enabled);

private:
    static constexpr int sampleRate = 44100;
    static constexpr int variants = 4;
    static constexpr int musicTracks = 4;
    static constexpr int sfxVoices = 4;
    void *xaDll = nullptr;
    void *xaudio = nullptr;
    void *master = nullptr;
    void *sfx[sfxVoices] = {};
    void *music = nullptr;
    std::vector<short> stepBuf[5][variants];
    std::vector<short> breakBuf[5][variants];
    std::vector<short> clickBuf;
    std::vector<short> musicBuf[musicTracks];

    struct PendingSound
    {
        SoundSet s;
        bool isBreak;
    };

    std::vector<PendingSound> pendingQueue;
    int musicTrack = 0;
    int sfxNext = 0;
    int stepVar[5] = {};
    int breakVar[5] = {};
    bool audioOk = false;
    bool sfxEnabled = true;
    bool musicEnabled = true;
    bool comInit = false;
    bool musicPrequeued = false;

    static void synthStep(SoundSet s, int v, std::vector<short> &out);

    static void synthBreak(SoundSet s, int v, std::vector<short> &out);

    static void synthClick(std::vector<short> &out);

    static void synthMusic(int track, std::vector<short> &out);

    void *makeVoice();

    void *pickSfxVoice();

    static void submitOnce(void *voice, const std::vector<short> &buf);

    void startMusic(int track);
};
#endif//CAVERN_AUDIO_HPP