#ifndef AUDIOMIXERSYSTEM_H_INCLUDED
#define AUDIOMIXERSYSTEM_H_INCLUDED

#include "ecs.h"
#include <SDL3/SDL.h>

void initAudioMixerSystem(SDL_AudioStream** outStream);
void updateAudioMixerSystem(Registry& registry, SDL_AudioStream* stream);
void cleanupAudioMixerSystem(SDL_AudioStream* stream);

#endif