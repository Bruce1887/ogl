#pragma once

#include "Error.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <filesystem>
#include <iostream>

#define NUM_SOUND_SOURCES 64 // we just assume every machine that runs our game can handle 64 simultaneous sounds

inline const std::filesystem::path AUDIO_DIR = std::filesystem::path("resources") / "audio";

extern ALCdevice *s_audio_device;
extern ALCcontext *s_audio_context;

// A simple sound manager that preallocates a pool of sound sources for playing SFX
// Is not responsible for loading audio data into buffers, just managing sources (playing sounds)
class SoundPlayer
{
private:
	ALuint m_soundSourcesPool[NUM_SOUND_SOURCES];
	uint32_t m_nextSoundIndex = 0; // Changed to non-static instance member

	SoundPlayer()
	{
		alGetError(); // Clear existing errors
		alGenSources(NUM_SOUND_SOURCES, m_soundSourcesPool);

		ALenum err = alGetError();
		if (err != AL_NO_ERROR)
		{
			std::cerr << "OpenAL Error: Could not generate " << NUM_SOUND_SOURCES << " sources. Error code: " << err << std::endl;
			// Consider falling back to a smaller number here
		}
	}

	// Private destructor: Ensures sources are cleaned up when the singleton dies
	~SoundPlayer()
	{
		alDeleteSources(NUM_SOUND_SOURCES, m_soundSourcesPool);
	}

	ALuint getNextSoundSource()
	{
		ALuint source = m_soundSourcesPool[m_nextSoundIndex];
		m_nextSoundIndex = (m_nextSoundIndex + 1) % NUM_SOUND_SOURCES;
		return source;
	}

public:
	// Delete copy constructor and assignment operator (Safety)
	SoundPlayer(const SoundPlayer &) = delete;
	SoundPlayer &operator=(const SoundPlayer &) = delete;

	// The Accessor: Thread-safe in C++11 and later
	static SoundPlayer &getInstance()
	{
		static SoundPlayer instance;
		return instance;
	}

	void PlaySFX(ALuint buffer)
	{
		DEBUG_PRINT("Playing SFX with buffer ID: " << buffer);
		ALuint source = getNextSoundSource();
		alSourcei(source, AL_BUFFER, buffer);
		alSourcef(source, AL_GAIN, 1.0f);
		alSourcePlay(source);
	}
};

bool Audio_Init();
void Audio_Shutdown();

ALuint LoadWav(std::filesystem::path path);

// The different sounds an entity can have
struct EntitySounds
{
	// Just raw openAL buffer IDs for now (could be wrapped later)
	ALuint m_attackSound = 0;
	// Add more sounds as needed
	// ALuint m_hurtSound;
	// ALuint m_deathSound;
	// ALuint m_walkSound;
};