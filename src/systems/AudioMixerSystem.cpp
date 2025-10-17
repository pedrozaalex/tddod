#include <SDL3/SDL.h>
#include <vector>
#include <cstring>

#include "components/SoundInstance.h"
#include "systems/AudioMixerSystem.h"

void initAudioMixerSystem(SDL_AudioStream** outStream)
{
    SDL_AudioSpec spec;

    spec.channels = 2;
    spec.format = SDL_AUDIO_S16;
    spec.freq = 44100;

    *outStream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
        &spec,
        NULL,
        NULL
    );

    if (!*outStream) {
        SDL_Log("Failed to create audio stream: %s", SDL_GetError());
        return;
    }

    SDL_ResumeAudioStreamDevice(*outStream);
}

void updateAudioMixerSystem(Registry& registry, SDL_AudioStream* stream)
{
    if (!stream) return;

    // Check how much audio is queued - only generate more if needed
    // Keep at least 1/10th second of audio buffered (44100 samples/sec * 2 channels * 2 bytes)
    const int minQueuedBytes = (44100 * 2 * 2) / 10;

    if (SDL_GetAudioStreamQueued(stream) < minQueuedBytes) {
        const int bufferSize = 4096 / 4 * 2 * 2; // samples * channels * bytes_per_sample
        std::vector<Uint8> mixBuffer(bufferSize, 0);

        // Mix all active sound instances
        registry.view<SoundInstance>().each([&registry, &mixBuffer, bufferSize](auto entity, SoundInstance& soundInstance)
            {
                if (!soundInstance.buffer || !soundInstance.len) {
                    registry.destroy(entity);
                    return;
                }

                int bytesToMix = (bufferSize > soundInstance.len) ? soundInstance.len : bufferSize;

                SDL_MixAudio(mixBuffer.data(), soundInstance.buffer, SDL_AUDIO_S16, bytesToMix, 0.5f);

                soundInstance.buffer += bytesToMix;
                soundInstance.len -= bytesToMix;

                if (soundInstance.len == 0) {
                    registry.destroy(entity);
                }
            });

        SDL_PutAudioStreamData(stream, mixBuffer.data(), bufferSize);
    }
}

void cleanupAudioMixerSystem(SDL_AudioStream* stream)
{
    if (stream) {
        SDL_DestroyAudioStream(stream);
    }
}