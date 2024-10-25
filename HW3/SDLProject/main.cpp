/**
* Author: Elizabeth Castroverde (eac9917)
* Assignment: Lunar Lander
* Date due: 2024-10-27, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define PLATFORM_COUNT 10

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_mixer.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include <cstdlib>
#include "Entity.h"

// ––––– STRUCTS AND ENUMS ––––– //
struct GameState
{
    Entity* player;
    Entity* platforms;
    Entity* font;
};

// ––––– CONSTANTS ––––– //
constexpr int WINDOW_WIDTH  = 640 *2,
          WINDOW_HEIGHT = 480 * 2;

constexpr float BG_RED     = 0.1922f,
            BG_BLUE    = 0.549f,
            BG_GREEN   = 0.9059f,
            BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

//acceleration due to gravity, make a small value
constexpr float GRAVITY = -0.03f;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;
/**
 source: https://opengameart.org/content/apollo-moon-landing-sprites
 */
constexpr char SPRITESHEET_FILEPATH[] = "assets/lander.png";
constexpr char MOUNTAIN_FILEPATH[]    = "assets/mountain.png";
constexpr char PLATFORM_FILEPATH[]    = "assets/platform.png";
constexpr char FONTSHEET_FILEPATH[]   = "assets/font1.png";
int random_int;

constexpr int NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL  = 0;
constexpr GLint TEXTURE_BORDER   = 0;

int fuel_level = 5000;

// ––––– GLOBAL VARIABLES ––––– //
GameState g_state;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

// ––––– GENERAL FUNCTIONS ––––– //
GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("NOT LUNAR LANDER!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    // ––––– VIDEO ––––– //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_program.set_projection_matrix(g_projection_matrix);
    g_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // ––––– PLATFORMS ––––– //
    GLuint mountain_texture_id = load_texture(MOUNTAIN_FILEPATH);
    GLuint platform_texture_id = load_texture(PLATFORM_FILEPATH);

    g_state.platforms = new Entity[PLATFORM_COUNT];

    // Set the type of every platform entity to PLATFORM
    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
        int y_pos = -3.5f;
        //make this specific one makes a platform that needs to be landed on
        if(i%3 == 0){ //need to offset the landing pads by the height of the image
            g_state.platforms[i].set_entity_type(TARGET);
            g_state.platforms[i].set_texture_id(platform_texture_id);
            y_pos = -4.5f;
        }else{
            g_state.platforms[i].set_entity_type(PLATFORM);
            g_state.platforms[i].set_texture_id(mountain_texture_id);
            g_state.platforms[i].set_scale(glm::vec3(1.0f, 2.0f, 0.0f));
        }
        
        g_state.platforms[i].set_position(glm::vec3(i+0.5f - PLATFORM_COUNT / 2.0f, y_pos, 0.0f)); //x,y,z
        g_state.platforms[i].set_width(0.8f);
        g_state.platforms[i].set_height(1.0f);
        
        g_state.platforms[i].update(0.0f, NULL, NULL, 0);
    }

    // ––––– LANDER ––––– //
    GLuint player_texture_id = load_texture(SPRITESHEET_FILEPATH);
    
    
    //lander initially moves down and to the right
    glm::vec3 acceleration = glm::vec3(0.02f,GRAVITY, 0.0f);

    std::vector<std::vector<int>> lander_animation = {{0},{1},{2}};

    g_state.player = new Entity(
            player_texture_id,         // texture id
            5.0f,                      // speed
            acceleration,              // acceleration
            lander_animation,  // animation index sets
            0.0f,                      // animation time
            4,                         // animation frame amount
            0,                         // current animation index
            3,                         // animation column amount
            1,                         // animation row amount
            0.5f,                      // width
            0.5f,                       // height
            LANDER
        );
    
    //lander starts at the top left of the screen
    g_state.player->set_position(glm::vec3(-3.5f, 2.75f, 0.0f));
    g_state.player->set_scale(glm::vec3(0.5f, 0.5f, 0.5f));
    g_state.player->set_can_thrust(true);
    //TEXT
    GLuint font_texture_id = load_texture(FONTSHEET_FILEPATH);
    g_state.font = new Entity;
    g_state.font->set_texture_id(font_texture_id);
    g_state.font->set_entity_type(FONT);
    
    // ––––– GENERAL ––––– //
    glEnable(GL_BLEND);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //set background to black
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = false;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_game_is_running = false;
                        break;

                    default:
                        break;
                }

            default:
                break;
        }
    }

    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
    //angle the specific object based on the key taht is pressed
    if (key_state[SDL_SCANCODE_LEFT])
    {
        g_state.player->rotate_left();
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        g_state.player->rotate_right();
    }
    
    if (key_state[SDL_SCANCODE_UP] && g_state.player->get_can_thrust())
    {//turn on the gas and also change the sprite to be the one spiting out rocket fuel
        g_state.player->set_animation_state(HIGH);
        g_state.player->set_thrust(true);
    }else{
        //movement is based on acceleration changes
        g_state.player->set_animation_state(NONE);
        g_state.player->set_thrust(false);
    }

    if (glm::length(g_state.player->get_movement()) > 1.0f)
    {
        g_state.player->normalise_movement();
    }
}

void update()
{
    if(g_state.player->get_thrust()){
        if(fuel_level >0){
            fuel_level--;
        }else{ //ran out of fuel
            g_state.player->set_can_thrust(false);
            g_state.player->set_thrust(false);
        }
    }
    //rocket always moves downwards
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    delta_time += g_accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP)
    {
        g_state.player->update(FIXED_TIMESTEP, NULL, g_state.platforms, PLATFORM_COUNT);
        delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    g_state.player->render(&g_program);

    for (int i = 0; i < PLATFORM_COUNT; i++) g_state.platforms[i].render(&g_program);
    //fuel UI
    g_state.font->draw_text(&g_program, g_state.font->get_texture_id(), "Fuel: " + std::to_string(fuel_level), 0.25f, 0.005f, glm::vec3(-4.75f, 3.5f, 0.0f));
    
    if(!g_state.player->get_player_status()){ //this means the game is over
        if(g_state.player->get_mission_status()){ //lander landed successfully
            g_state.font->draw_text(&g_program, g_state.font->get_texture_id(), "Mission accomplished!", 0.3f, 0.005f, glm::vec3(-3.0f, 2.0f, 0.0f));
        }else{
            g_state.font->draw_text(&g_program, g_state.font->get_texture_id(), "Mission failed", 0.3f, 0.05f, glm::vec3(-2.5f, 2.0f, 0.0f));
        }
    }
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();

    delete [] g_state.platforms;
    delete g_state.player;
}

// ––––– GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    srand(static_cast<int>(time(0)));
    
    initialise();
    int limit = PLATFORM_COUNT + 1;
    random_int = random() % limit;  // range [0, 100)

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
