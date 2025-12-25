#pragma once

#include "Error.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <filesystem>
#include <iostream>
#include <optional>
#include <glm/glm.hpp>

#define MUSIC_SOUND_SOURCES 1 // num sources reserved for music
#define SFX_SOUND_SOURCES 127  // num sources reserved for various sound effects

inline const std::filesystem::path AUDIO_DIR = std::filesystem::path("resources") / "audio";

extern ALCdevice *s_audio_device;
extern ALCcontext *s_audio_context;

/**
 * @brief Singleton class for playing sounds using OpenAL
 */
class SoundPlayer
{
private:
	ALuint m_musicSourcesPool[MUSIC_SOUND_SOURCES];
	ALuint m_SFXSourcesPool[SFX_SOUND_SOURCES];
	uint32_t m_nextRoundRobinIndex = 0; // Tracks the "oldest" source for stealing

	SoundPlayer()
	{
		// Clear previous errors so we don't catch garbage
		alGetError();

		alGenSources(MUSIC_SOUND_SOURCES, m_musicSourcesPool);
		alGenSources(SFX_SOUND_SOURCES, m_SFXSourcesPool);

		ALenum err = alGetError();
		if (err != AL_NO_ERROR)
		{
			DEBUG_PRINT("OpenAL Error: Could not generate sources. Code: " << err);
		}
	}

	~SoundPlayer()
	{
		alDeleteSources(SFX_SOUND_SOURCES, m_SFXSourcesPool);
		alDeleteSources(MUSIC_SOUND_SOURCES, m_musicSourcesPool);
	}

	// Tries to find a source that is not currently playing.
	// If all are playing, returns 0 (indicating none found).
	ALuint findFreeSource(ALuint *sources)
	{
		for (int i = 0; i < SFX_SOUND_SOURCES; i++)
		{
			ALuint source = sources[i];
			ALint state;
			alGetSourcei(source, AL_SOURCE_STATE, &state);

			// If the source is stopped or hasn't been used yet (INITIAL), it's free.
			if (state == AL_STOPPED || state == AL_INITIAL)
			{
				return source;
			}
		}
		return 0; // 0 is essentially "nullptr" for OpenAL handles
	}

	// Forcefully gets the next source in line, overwriting whatever is playing there.
	ALuint stealNextSource(ALuint *sources)
	{
		ALuint source = sources[m_nextRoundRobinIndex];

		m_nextRoundRobinIndex = ((m_nextRoundRobinIndex + 1) % SFX_SOUND_SOURCES);
		return source;
	}

	ALuint getFreeOrStealSource(ALuint *sources)
	{
		ALuint source = findFreeSource(sources);
		if (source != 0)
		{
			return source;
		}
		else
		{
			return stealNextSource(m_SFXSourcesPool);
		}
	}

public:
	SoundPlayer(const SoundPlayer &) = delete;
	SoundPlayer &operator=(const SoundPlayer &) = delete;

	static SoundPlayer &getInstance()
	{
		static SoundPlayer instance;
		return instance;
	}

	/**
	 * @brief Plays background music from the given buffer.
	 *
	 * @param buffer The OpenAL buffer ID containing the music data.
	 * @param loop Whether the music should loop continuously.
	 */
	void PlayMusic(ALuint buffer, bool loop = true)
	{
		ALuint source = getFreeOrStealSource(m_musicSourcesPool);

		alSourcei(source, AL_BUFFER, buffer);
		// alSourcef(source, AL_GAIN, 0.5f); // Music is usually quieter
		alSourcei(source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
		alSourcePlay(source);
#ifdef DEBUG
		ALenum err = alGetError();
		if (err != AL_NO_ERROR)
		{
			DEBUG_PRINT("OpenAL Error: Could not play music. Source: " << source << " Code: " << err);
		}
#endif
	}

	/**
	 * @brief Plays a sound effect (SFX) at the given position (if any).
	 *
	 * @param buffer The OpenAL buffer ID containing the sound data.
	 * @param position Optional 3D position to play the sound at.
	 * @param force_play If true, will steal a source if none are free; otherwise, will skip playing if all are busy.
	 */
	void PlaySFX(ALuint buffer, std::optional<glm::vec3> position = std::nullopt, bool force_play = false)
	{
		ALuint source = 0;
		if (force_play)
			source = getFreeOrStealSource(m_SFXSourcesPool);
		else
		{
			source = findFreeSource(m_SFXSourcesPool);
			if (source == 0)
				return;
		}

		alSourcei(source, AL_BUFFER, buffer);
		alSourcef(source, AL_GAIN, 0.5f); // TEMP: Quieter because i don't want to blow up my ears with SFX
		if (position.has_value())
			alSource3f(source, AL_POSITION, position->x, position->y, position->z);

		alSourcePlay(source);

#ifdef DEBUG
		ALenum err = alGetError();
		if (err != AL_NO_ERROR)
		{
			DEBUG_PRINT("OpenAL Error: Could not play SFX. Source: " << source << " Code: " << err);
		}
#endif
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