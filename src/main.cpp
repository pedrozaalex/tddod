/*
    Program entry and main loop.
    This is where we initialize SDL and poll events.
*/

#if defined(WIN32)
#include <windows.h>
#endif

#include <GL/gl3w.h>
#include <SDL3/SDL.h>

#include <chrono>
#include <stdio.h>

#include "constants.h"
#include "game.h"

#include "helpers/InputHelpers.h"
#include "systems/AudioMixerSystem.h"

#if defined(WIN32)
int CALLBACK WinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdCount)
#else
int main(int argc, char** argv)
#endif
{
    // Init SDL
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    auto sdlWindow = SDL_CreateWindow(
        "Data Oriented Tower Defense",
        WIDTH, HEIGHT,
        SDL_WINDOW_OPENGL);

    // Init OpenGL
    auto glContext = SDL_GL_CreateContext(sdlWindow);
    SDL_GL_SetSwapInterval(1);
    gl3wInit();

    // Create our registry and initialize the game
    Registry registry;
    Game::init(registry);

    // Init audio with SDL3 audio stream
    SDL_AudioStream *audioStream = nullptr;
    initAudioMixerSystem(&audioStream);
    if (!audioStream) {
        SDL_Log("Failed to initialize audio system");
    }

    // Main loop
    int updateSpeed = 1;
    bool done = false;
    auto lastTime = std::chrono::high_resolution_clock::now();
    while (!done)
    {
        // Calculate delta time
        auto now = std::chrono::high_resolution_clock::now();
        auto diffTime = now - lastTime;
        lastTime = now;
        auto dt = (float)((double)std::chrono::duration_cast<std::chrono::microseconds>(diffTime).count() / 1000000.0);

        // Poll events
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                done = true;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_TAB) updateSpeed = 10;
                else if (event.key.key == SDLK_SPACE)
                {
                    if (updateSpeed) updateSpeed = 0;
                    else updateSpeed = 1;
                }
                Input::onKeyDown(registry, event.key.key);
                break;
            case SDL_EVENT_KEY_UP:
                if (event.key.key == SDLK_TAB) updateSpeed = 1;
                Input::onKeyUp(registry, event.key.key);
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                Input::onMouseButtonDown(registry, event.button.button);
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                Input::onMouseButtonUp(registry, event.button.button);
                break;
            case SDL_EVENT_MOUSE_MOTION:
                Input::onMouseMotion(registry, event.motion.x, event.motion.y);
                break;
            }
        }

        // Update game simulation
        for (int i = 0; i < updateSpeed; ++i)
        {
            Game::updateSim(registry, dt);
        }

        // Update UI stuff, independent from simulation
        Game::update(registry, dt);

        // Update audio - feed the audio stream with mixed sound data
        if (audioStream) {
            updateAudioMixerSystem(registry, audioStream);
        }

        // Draw game
        Game::render(registry);

        // Swap buffers
        SDL_GL_SwapWindow(sdlWindow);
    }

    // Cleanup
    cleanupAudioMixerSystem(audioStream);
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(sdlWindow);
    SDL_Quit();
}