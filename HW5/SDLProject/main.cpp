/**
* Author: Elizabeth Castroverde
* Assignment: Platformer
* Date due: 2023-11-23, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
/**
 BGM and SFX
 BGM1:https://pixabay.com/users/sunnyvibesaudio-42625630/
 BGM2: https://pixabay.com/users/sekuora-40269569/?utm_source=link-attribution&utm_medium=referral&utm_campaign=music&utm_content=240795
 BGM3: https://pixabay.com/music/search/cute%20viedogame/?genre=video%2520games
 Win: https://freesound.org/people/Fupicat/sounds/521639/
 Lose: https://freesound.org/people/Fupicat/sounds/538151/
 Jump: https://freesound.org/people/el_boss/sounds/751698/
 Enemy AI: https://grafxkid.itch.io/sprite-pack-2
 */
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "LevelA.h"
#include "LevelB.h"
#include "Effects.h"
#include "Menu.h"
#include "LevelC.h"

// ––––– CONSTANTS ––––– //
constexpr int WINDOW_WIDTH  = 640*2,
WINDOW_HEIGHT = 480*2;

constexpr float BG_RED     = 0.705f,
            BG_BLUE    = 0.705f,
            BG_GREEN   = 0.705f,
            BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

bool pause_screen = false; //false = not paused

constexpr float MILLISECONDS_IN_SECOND = 1000.0;
int curr_lives = 3;
enum AppStatus { RUNNING, TERMINATED };

// ––––– GLOBAL VARIABLES ––––– //
Scene  *g_current_scene;
LevelA *g_levelA;
LevelB *g_levelB;
LevelC *g_levelC;

Effects *g_effects;
Scene   *g_levels[3];
Menu *g_menu;

SDL_Window* g_display_window;


ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

int g_old_stomp_count = 0;
int g_new_stomp_count = 0;
float g_shake_timer = 0.0f;

AppStatus g_app_status = RUNNING;

void swtich_to_scene(Scene *scene);
void initialise();
void process_input();
void update();
void render();
void shutdown();

// ––––– GENERAL FUNCTIONS ––––– //
void switch_to_scene(Scene *scene, int curr_lives)
{
    g_current_scene = scene;
    scene->set_num_lives(curr_lives);
    g_current_scene->initialise(); // DON'T FORGET THIS STEP!
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Escape the Grocery Store",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
    if (context == nullptr)
    {
        shutdown();
    }
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    g_levelA = new LevelA();
    g_levelB = new LevelB();
    g_levelC = new LevelC();
    
    g_levels[0] = g_levelA;
    g_levels[1] = g_levelB;
    g_levels[2] = g_levelC;
    
    g_menu = new Menu();

    // Start at maine menu
    switch_to_scene(g_menu, 0);
    
    g_effects = new Effects(g_projection_matrix, g_view_matrix);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    g_current_scene->get_state().player->set_movement(glm::vec3(0.0f));
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_app_status = TERMINATED;
                        break;
                        
                    case SDLK_SPACE:
                        // Jump
                        if (g_current_scene->get_state().player->get_collided_bottom())
                                                {
                                                    g_current_scene->get_state().player->jump();
                                                    Mix_PlayChannel(-1,  g_current_scene->get_state().jump_sfx, 0);
                                                }
                                                 break;
                    case SDLK_RETURN:// go to the next scene from the main menu
                        if(g_current_scene == g_menu){
                            switch_to_scene(g_levels[0], 3);
                        }
                        break;
                    case SDLK_ESCAPE: //this is the pause button
                        if(!pause_screen){
                            pause_screen = true;
                            g_current_scene->set_pause_screen(pause_screen);
                        }else{
                            pause_screen = false;
                            g_current_scene->set_pause_screen(pause_screen);
                        }
                        break;
                    default:
                        break;
                }
            
                
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT] && !pause_screen)        g_current_scene->get_state().player->move_left();
        else if (key_state[SDL_SCANCODE_RIGHT] && !pause_screen)  g_current_scene->get_state().player->move_right();
         
    if (glm::length( g_current_scene->get_state().player->get_movement()) > 1.0f)
        g_current_scene->get_state().player->normalise_movement();
    
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    delta_time += g_accumulator;
    
    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP) {
        g_current_scene->update(FIXED_TIMESTEP);
        g_effects->update(FIXED_TIMESTEP);
        //the screen shakes everytime a player stomps on an enemy
        g_new_stomp_count = g_current_scene->get_state().player->get_stomp_count();

        if (g_old_stomp_count < g_new_stomp_count) {
            g_effects->start(SHAKE, 1.0f);
            g_shake_timer = 1.0f;
        }

        if (g_shake_timer > 0) {
            g_shake_timer -= FIXED_TIMESTEP;
        } else {
            g_effects->start(NONE);
        }

        g_old_stomp_count = g_new_stomp_count;
        delta_time -= FIXED_TIMESTEP;
    }
    
    g_accumulator = delta_time;
    
    // Prevent the camera from showing anything outside of the "edge" of the level
    if(!pause_screen){
        g_view_matrix = glm::mat4(1.0f);
        
        if (g_current_scene->get_state().player->get_position().x > LEVEL1_LEFT_EDGE) {
            g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->get_state().player->get_position().x, 3.75, 0));
        } else {
            g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-5, 3.75, 0));
        }
        curr_lives = g_current_scene->get_num_lives();
        
        if (g_current_scene == g_levelA && g_current_scene->get_state().player->get_position().y < -10.0f) switch_to_scene(g_levelB, curr_lives);
        if (g_current_scene == g_levelB && g_current_scene->get_state().player->get_position().y < -10.0f && g_current_scene->get_state().player->get_position().x > 4.0f  ) switch_to_scene(g_levelC, curr_lives);

        
        g_view_matrix = glm::translate(g_view_matrix, g_effects->get_view_offset());
        
    }
}

void render()
{
    g_shader_program.set_view_matrix(g_view_matrix);
       
    glClear(GL_COLOR_BUFFER_BIT);
       
    // ————— RENDERING THE SCENE (i.e. map, character, enemies...) ————— //
    g_current_scene->render(&g_shader_program);
       
    g_effects->render();
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();
    delete g_levelA;
    delete g_levelB;
    delete g_levelC;
    delete g_effects;
    delete g_menu;
}

// ––––– DRIVER GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        
        if (g_current_scene->get_state().next_scene_id >= 0){
            curr_lives = g_current_scene->get_num_lives();
            switch_to_scene(g_levels[g_current_scene->get_state().next_scene_id], curr_lives);
        }
        
        render();
    }
    
    shutdown();
    return 0;
}
