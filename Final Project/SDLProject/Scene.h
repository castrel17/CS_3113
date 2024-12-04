#pragma once
#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Util.h"
#include "Entity.h"
#include "Map.h"

/**
    Notice that the game's state is now part of the Scene class, not the main file.
*/
struct GameState
{
    // ————— GAME OBJECTS ————— //
    Map *map;
    Entity *player;
    Entity *enemies;
    Entity *ammo;
    Entity *orb;
    //Entity *lives;
    // ————— AUDIO ————— //
    Mix_Music *bgm;
    Mix_Chunk *stomp_sfx;
    Mix_Chunk *lose_sfx;
    Mix_Chunk *win_sfx;
    Mix_Chunk *next_level_sfx;
    bool sound_played = false;
    GLuint font;
    
    bool pause_screen = false;
    
    // ————— POINTERS TO OTHER SCENES ————— //
    int next_scene_id;
    float lives;
};

class Scene {
protected:
    GameState m_game_state;
    
public:
    // ————— ATTRIBUTES ————— //
    int m_number_of_enemies = 1;
    
    // ————— METHODS ————— //
    virtual void initialise() = 0;
    virtual void update(float delta_time) = 0;
    virtual void render(ShaderProgram *program) = 0;
    
    void set_pause_screen(bool status){m_game_state.pause_screen = status;}
    // ————— GETTERS ————— //
    GameState const get_state() const { return m_game_state;             }
    int const get_number_of_enemies() const { return m_number_of_enemies; }
    
    void set_num_lives(int new_lives) { m_game_state.lives = new_lives; }
    int get_num_lives() const { return m_game_state.lives; }
};
