#include "Audio.h"
#include <fstream>
#include <vector>
#include <cstring>

ALCdevice *s_audio_device = nullptr;
ALCcontext *s_audio_context = nullptr;

bool Audio_Init()
{
    s_audio_device = alcOpenDevice(nullptr);
    if (!s_audio_device) {
        DEBUG_PRINT("OpenAL Error: Could not open default device.");
        return false;
    }

    s_audio_context = alcCreateContext(s_audio_device, nullptr);
    if (!s_audio_context) {
        ALCenum err = alcGetError(s_audio_device);
        DEBUG_PRINT("OpenAL Error: Could not create context. Error code: " << err);
        alcCloseDevice(s_audio_device);
        return false;
    }

    if (!alcMakeContextCurrent(s_audio_context)) {
        DEBUG_PRINT("OpenAL Error: Could not make context current.");
        return false;
    }

    return true;
}

void Audio_Shutdown()
{
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(s_audio_context);
    alcCloseDevice(s_audio_device);
}

static ALenum GetFormat(int channels, int bits)
{
    if (channels == 1 && bits == 16) return AL_FORMAT_MONO16;
    if (channels == 2 && bits == 16) return AL_FORMAT_STEREO16;
    return 0;
}

ALuint SoundPlayer::LoadWav(std::filesystem::path path)
{
    std::ifstream f(path.c_str(), std::ios::binary); // TODO: might screw up on non-ASCII paths
    if (!f) return 0;

    char riff[4];
    f.read(riff, 4);
    f.ignore(4);
    char wave[4];
    f.read(wave, 4);

    char chunk[4];
    int size;
    short format, channels, bits;
    int sampleRate;
    std::vector<char> data;

    while (f.read(chunk, 4))
    {
        f.read(reinterpret_cast<char*>(&size), 4);
        if (!std::strncmp(chunk, "fmt ", 4))
        {
            f.read(reinterpret_cast<char*>(&format), 2);
            f.read(reinterpret_cast<char*>(&channels), 2);
            f.read(reinterpret_cast<char*>(&sampleRate), 4);
            f.ignore(6);
            f.read(reinterpret_cast<char*>(&bits), 2);
            f.ignore(size - 16);
        }
        else if (!std::strncmp(chunk, "data", 4))
        {
            data.resize(size);
            f.read(data.data(), size);
            break;
        }
        else
        {
            f.ignore(size);
        }
    }

    ALuint buffer;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, GetFormat(channels, bits),
                 data.data(), data.size(), sampleRate);
    
    m_buffers.push_back(buffer);
    return buffer;
}

void SoundPlayer::PauseAll()
{
    for (int i = 0; i < MUSIC_SOUND_SOURCES; i++)
    {
        ALint state;
        alGetSourcei(m_musicSourcesPool[i], AL_SOURCE_STATE, &state);
        if (state == AL_PLAYING)
            alSourcePause(m_musicSourcesPool[i]);
    }

    for (int i = 0; i < SFX_SOUND_SOURCES; i++)
    {
        ALint state;
        alGetSourcei(m_SFXSourcesPool[i], AL_SOURCE_STATE, &state);
        if (state == AL_PLAYING)
            alSourcePause(m_SFXSourcesPool[i]);
    }
}

void SoundPlayer::ResumeAll()
{
    
    for (int i = 0; i < MUSIC_SOUND_SOURCES; i++)
    {
        ALint state;
        alGetSourcei(m_musicSourcesPool[i], AL_SOURCE_STATE, &state);
        ALint buffer;
        alGetSourcei(m_musicSourcesPool[i], AL_BUFFER, &buffer);
        if (state == AL_PAUSED)
            alSourcePlay(m_musicSourcesPool[i]);
    }

    for (int i = 0; i < SFX_SOUND_SOURCES; i++)
    {
        ALint state;
        alGetSourcei(m_SFXSourcesPool[i], AL_SOURCE_STATE, &state);
        ALint buffer;
        alGetSourcei(m_SFXSourcesPool[i], AL_BUFFER, &buffer);
        if (state == AL_PAUSED)
            alSourcePlay(m_SFXSourcesPool[i]);
    }
}

void SoundPlayer::StopAll()
{
    for (int i = 0; i < MUSIC_SOUND_SOURCES; i++)
        alSourceStop(m_musicSourcesPool[i]);

    for (int i = 0; i < SFX_SOUND_SOURCES; i++)
        alSourceStop(m_SFXSourcesPool[i]);
}
void SoundPlayer::PauseMusic()
{
    if (!m_activeMusicSource) return;

    ALint state;
    alGetSourcei(m_activeMusicSource, AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING)
        alSourcePause(m_activeMusicSource);
}

void SoundPlayer::ResumeMusic()
{
    if (!m_activeMusicSource) return;

    ALint state;
    alGetSourcei(m_activeMusicSource, AL_SOURCE_STATE, &state);
    if (state == AL_PAUSED)
        alSourcePlay(m_activeMusicSource);
}