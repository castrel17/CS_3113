/**
* Author: Elizabeth Castroverde
* Assignment: Rise of the AI
* Date due: 2024-11-9, 11:59pm
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
#define ENEMY_COUNT 3
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 5

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"

// ————— GAME STATE ————— //
struct GameState
{
    Entity *player;
    Entity *enemies;
    Entity *ammo;
    
    Map *map;
    
    Mix_Music *bgm;
    Mix_Chunk *jump_sfx;
};

enum AppStatus { RUNNING, TERMINATED };

// ————— CONSTANTS ————— //
constexpr int WINDOW_WIDTH  = 640 * 2,
          WINDOW_HEIGHT = 480 * 2;

constexpr float BG_RED     = 0.1922f,
            BG_BLUE    = 0.549f,
            BG_GREEN   = 0.9059f,
            BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char GAME_WINDOW_NAME[] = "AI Game";

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;
constexpr int FONTBANK_SIZE = 16;
void draw_text(ShaderProgram *shader_program, GLuint font_texture_id, std::string text,
               float font_size, float spacing, glm::vec3 position);


/*
 sources: 
 player: https://axulart.itch.io/small-8-direction-characters
 platform: https://kenney.nl/assets/platformer-art-pixel-redux
 sounds:
 AI: https://kenney.nl/assets/platformer-art-pixel-redux
 */
constexpr char SPRITESHEET_FILEPATH[] = "assets/images/player copy.png",
           MAP_TILESET_FILEPATH[] = "assets/images/tilemap_packed.png",
           BGM_FILEPATH[]         = "assets/audio/dooblydoo.mp3",
           JUMP_SFX_FILEPATH[]    = "assets/audio/bounce.wav",
            AI1_FILEPATH[]       = "assets/images/AI1.png",
            AI2_FILEPATH[]         = "assets/images/AI2.png",
            AI3_FILEPATH[]         = "assets/images/AI3.png",
            FONTSHEET_FILEPATH[]         = "assets/fonts/font1.png",
            AMMO_FILEPATH[]         = "assets/images/fish.png";
            

constexpr int NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL  = 0;
constexpr GLint TEXTURE_BORDER   = 0;
GLuint g_font_texture_id;
unsigned int LEVEL_1_DATA[] =
{
    //actual map
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 3, 1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 0, 0, 0, 0, 0, 0, 0, 3, 1, 1, 1, 1, 1,
    2, 0, 0, 0, 0, 0, 1, 0, 0, 2, 2, 2, 2, 2,
    2, 0, 0, 0, 0, 0, 2, 0, 0, 2, 2, 2, 2, 2,
    2, 0, 0, 0, 0, 0, 2, 0, 0, 2, 2, 2, 2, 2,
    2, 0, 0, 0, 0, 0, 2, 0, 0, 2, 2, 2, 2, 2
    
    //for testing
//    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//    2, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2, 2, 2,
//    2, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2, 2, 2
    
};

// ————— VARIABLES ————— //
GameState g_game_state;

SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f,
      g_accumulator    = 0.0f;


void initialise();
void process_input();
void update();
void render();
void shutdown();
void draw_text(ShaderProgram *shader_program, GLuint font_texture_id, std::string text,
               float font_size, float spacing, glm::vec3 position);

void draw_text(ShaderProgram *shader_program, GLuint font_texture_id, std::string text,
               float font_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for
    // each character. Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their
        //    position relative to the whole sentence)
        int spritesheet_index = (int) text[i];  // ascii value of character
        float offset = (font_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float) (spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float) (spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
        });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
        });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    shader_program->set_model_matrix(model_matrix);
    glUseProgram(shader_program->get_program_id());

    glVertexAttribPointer(shader_program->get_position_attribute(), 2, GL_FLOAT, false, 0,
                          vertices.data());
    glEnableVertexAttribArray(shader_program->get_position_attribute());

    glVertexAttribPointer(shader_program->get_tex_coordinate_attribute(), 2, GL_FLOAT,
                          false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(shader_program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int) (text.size() * 6));

    glDisableVertexAttribArray(shader_program->get_position_attribute());
    glDisableVertexAttribArray(shader_program->get_tex_coordinate_attribute());
}


// ————— GENERAL FUNCTIONS ————— //
GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    GLuint texture_id;
    glGenTextures(NUMBER_OF_TEXTURES, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    stbi_image_free(image);
    
    return texture_id;
}

void initialise()
{
    // ————— GENERAL ————— //
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow(GAME_WINDOW_NAME,
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    if (context == nullptr)
    {
        LOG("ERROR: Could not create OpenGL context.\n");
        shutdown();
    }
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    // ————— VIDEO SETUP ————— //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    // ————— MAP SET-UP ————— //
    GLuint map_texture_id = load_texture(MAP_TILESET_FILEPATH);
    g_game_state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, LEVEL_1_DATA, map_texture_id, 1.0f, 5, 1);
    
    // ————— PLAYER ————— //
    GLuint player_texture_id = load_texture(SPRITESHEET_FILEPATH);

    int player_walking_animation[4][3] =
    {
        {1, 3, 9},  //left
        {3, 7,11}, //right
        {2, 6, 10}, //up
        {0, 4, 8} ,//down
    };

    glm::vec3 acceleration = glm::vec3(0.0f,-4.905f, 0.0f);

    g_game_state.player = new Entity(
        player_texture_id,         // texture id
        5.0f,                      // speed
        acceleration,              // acceleration
        3.0f,                      // jumping power
        player_walking_animation,  // animation index sets
        0.0f,                      // animation time
        4,                         // animation frame amount
        0,                         // current animation index
        4,                         // animation column amount
        3,                         // animation row amount
        0.9f,                      // width
        0.9f,                       // height
        PLAYER
    );
    g_game_state.player->set_position(glm::vec3(0.0f, -1.0f, 0.0f));
    // Jumping
    g_game_state.player->set_jumping_power(4.0f);
    g_game_state.player->set_entity_type(PLAYER);
    
    // ––––– AI1 (GUARD) ––––– //
    g_game_state.enemies = new Entity[ENEMY_COUNT];
    
    GLuint ai1_texture_id = load_texture(AI1_FILEPATH);
    
    g_game_state.enemies[0] =  Entity(ai1_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, GUARD, IDLE);
    g_game_state.enemies[0].set_entity_type(ENEMY);
    g_game_state.enemies[0].set_position(glm::vec3(4.0f, 0.0f, 0.0f));
    g_game_state.enemies[0].set_movement(glm::vec3(0.0f));
    g_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    
//    // ––––– AI2 (JUMPER) ––––– //
    GLuint ai2_texture_id = load_texture(AI2_FILEPATH);
    
    g_game_state.enemies[1] =  Entity(ai2_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, JUMPER, IDLE);
    g_game_state.enemies[1].set_entity_type(ENEMY);
    g_game_state.enemies[1].set_ai_type(JUMPER);
    g_game_state.enemies[1].set_position(glm::vec3(6.0f, -2.0f, 0.0f));
    g_game_state.enemies[1].set_jumping_power(4.0f);
    g_game_state.enemies[1].set_movement(glm::vec3(0.0f));
    g_game_state.enemies[1].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));

   
    // ––––– AI3 (SHOOTER) ––––– //
    GLuint ai3_texture_id = load_texture(AI3_FILEPATH);
    
    g_game_state.enemies[2] =  Entity(ai3_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, SHOOTER, IDLE);
    g_game_state.enemies[2].set_entity_type(ENEMY);
    g_game_state.enemies[2].set_ai_type(SHOOTER);
    g_game_state.enemies[2].set_position(glm::vec3(12.0f, -1.0f, 0.0f));
    g_game_state.enemies[2].set_movement(glm::vec3(0.0f));
    
    // ––––– AMMO ––––– //
    GLuint ammo_texture_id = load_texture(AMMO_FILEPATH);
    g_game_state.ammo =  new Entity(ammo_texture_id, 1.0f, 1.0f, 1.0f, AMMO);
    g_game_state.ammo->set_entity_type(AMMO);
    g_game_state.ammo->set_position(glm::vec3(12.0f, -1.0f, 0.0f));
    g_game_state.ammo->set_start_position(glm::vec3(12.0f, -1.0f, 0.0f));
    g_game_state.ammo->set_scale(glm::vec3(0.5f, 0.5f, 0.0f));
    
    
    // ––––– text ––––– //
    g_font_texture_id = load_texture(FONTSHEET_FILEPATH);
    
    g_game_state.jump_sfx = Mix_LoadWAV(JUMP_SFX_FILEPATH);
    
    // ————— BLENDING ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_game_state.player->set_movement(glm::vec3(0.0f));
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
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
                        if (g_game_state.player->get_collided_bottom())
                        {
                            g_game_state.player->jump();
                            Mix_PlayChannel(-1, g_game_state.jump_sfx, 0);
                        }
                        break;
                        
                    default:
                        break;
                }
            case SDLK_RSHIFT: //this fires a bullet
                break;
                
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT])       g_game_state.player->move_left();
    else if (key_state[SDL_SCANCODE_RIGHT]) g_game_state.player->move_right();
         
    if (glm::length(g_game_state.player->get_movement()) > 1.0f)
        g_game_state.player->normalise_movement();
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
    
    while (delta_time >= FIXED_TIMESTEP)
    {
        g_game_state.player->update(FIXED_TIMESTEP, g_game_state.player, g_game_state.enemies, 3,
                                    g_game_state.map, g_game_state.ammo);
        
        //UPDATE AI POSITION
        for (int i = 0; i < ENEMY_COUNT; i++) {
            g_game_state.enemies[i].update(FIXED_TIMESTEP, g_game_state.player, NULL, 0,
                                           g_game_state.map, g_game_state.ammo);
        }
        g_game_state.ammo->update(FIXED_TIMESTEP, g_game_state.player, g_game_state.player, 1,
                                    g_game_state.map, g_game_state.ammo);
        
        delta_time -= FIXED_TIMESTEP;
    }
    
    g_accumulator = delta_time;
    
    g_view_matrix = glm::mat4(1.0f);
    
    // Camera Follows the player
    g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_game_state.player->get_position().x, 0.0f, 0.0f));
}

void render()
{
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    g_game_state.player->render(&g_shader_program);
    g_game_state.map->render(&g_shader_program);
    g_game_state.ammo->render(&g_shader_program);
    for (int i = 0; i < ENEMY_COUNT; i++)    g_game_state.enemies[i].render(&g_shader_program);
    glm::vec3 player_pos = g_game_state.player->get_position();
    //but if the player falls of the screen just put it in the middle
    
    if(!g_game_state.player->get_state()){ //player died
        if(g_game_state.player->get_on_screen()){
            draw_text(&g_shader_program, g_font_texture_id, "You Lose :(", 0.5f, 0.05f, player_pos-1.0f);
        }else{
            draw_text(&g_shader_program, g_font_texture_id, "You Lose :(", 0.5f, 0.05f, glm::vec3(0.0f, 0.0f, 0.0f));
        }
    }
    else if(g_game_state.player->get_win_status()){ //player wins
        draw_text(&g_shader_program, g_font_texture_id, "You Win!", 0.5f, 0.05f, player_pos);
    }
    
    //win if the player defeated all the enemies and is still on the platform
    //lose if an enemy kills the player or the player falls off the platform
    SDL_GL_SwapWindow(g_display_window);
    
    
    
}

void shutdown()
{
    SDL_Quit();
    
    delete [] g_game_state.enemies;
    delete    g_game_state.player;
    delete    g_game_state.map;
    delete    g_game_state.ammo;
    Mix_FreeChunk(g_game_state.jump_sfx);
    Mix_FreeMusic(g_game_state.bgm);
}

// ————— GAME LOOP ————— //
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}