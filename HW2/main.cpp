/**
* Author: Elizabeth Castroverde (eac9917)
* Assignment: Pong Clone
* Date due: 2024-10-12, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
/**
 -paddles/players = done
 -single-player switch = done
 -bounces off walls
 -game over
 
 Player 1: guinea pig on the left, controls: W (up), S (down)
  Player 2: rabbit on the right, controls: up key, down key
 */

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
enum AppStatus { RUNNING, TERMINATED };
constexpr float WINDOW_SIZE_MULT = 1.0f;
constexpr int WINDOW_WIDTH  = 640 * WINDOW_SIZE_MULT,
              WINDOW_HEIGHT = 480 * WINDOW_SIZE_MULT;
constexpr float BG_RED     = 0.9765625f,
                BG_GREEN   = 0.97265625f,
                BG_BLUE    = 0.9609375f,
                BG_OPACITY = 1.0f;
constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;
constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";
constexpr float MILLISECONDS_IN_SECOND = 1000.0;

constexpr bool game_over = false;
constexpr bool move_up = false;


/*sources:
Rabbit:https://www.pngegg.com/en/png-bbapk/download
Carrot:https://www.pngegg.com/en/png-eqwjv
Guinea Pig: https://www.pngegg.com/en/png-bknon/download
 */
constexpr char PLAYER_1_SPRITE_FILEPATH[] = "gp.png", //on the left
               PLAYER_2_SPRITE_FILEPATH[] = "rabbit.png", //on the right
               BALL_SPRITE_FILEPATH[]  = "carrot.png";
constexpr float MINIMUM_COLLISION_DISTANCE = 1.0f;
constexpr glm::vec3 INIT_SCALE_BALL = glm::vec3(0.5f, 0.5f, 0.5f),
                    INIT_POS_BALL   = glm::vec3(0.0f, 0.0f, 0.0f),
                    INIT_SCALE_PLAYER_1 = glm::vec3(1.5f, 1.5f, 1.0f),
                    INIT_POS_PLAYER_1  = glm::vec3(-4.5f, 0.0f, 0.0f),
                    INIT_SCALE_PLAYER_2 = glm::vec3(1.5f, 1.5f, 1.0f),
                    INIT_POS_PLAYER_2  = glm::vec3(4.5f, 0.0f, 0.0f);
                    
SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();
glm::mat4 g_view_matrix, g_player_1_matrix, g_player_2_matrix, g_projection_matrix, g_ball_matrix;
float g_previous_ticks = 0.0f;
GLuint g_player_1_texture_id;
GLuint g_player_2_texture_id;
GLuint g_ball_texture_id;
glm::vec3 g_player_1_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_player_1_movement = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_player_2_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_player_2_movement = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_ball_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_ball_movement = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_ball_velocity = glm::vec3(0.0f, 0.0f, 0.0f);
float g_player_1_speed = 1.0f;
float g_player_2_speed = 1.0f;
float initial_speed = 5.0f;
float random_angle = glm::radians(static_cast<float>((rand() % 120) - 60));
//flag to track auto mode for player 2
bool player_2_auto_mode = false;


void initialise();
void process_input();
void update();
void render();
void shutdown();

constexpr GLint NUMBER_OF_TEXTURES = 1;  // to be generated, that is
constexpr GLint LEVEL_OF_DETAIL    = 0;  // base image level; Level n is the nth mipmap reduction image
constexpr GLint TEXTURE_BORDER     = 0;  // this value MUST be zero
GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);
    
    return textureID;
}
void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Carrot Pong!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
    
    if (g_display_window == nullptr)
    {
        shutdown();
    }
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_player_1_matrix = glm::mat4(1.0f);
    g_player_2_matrix = glm::mat4(1.0f);
    
    
    //setting up position and ball movement
    g_ball_matrix = glm::mat4(1.0f);
    g_ball_matrix = glm::translate(g_ball_matrix, glm::vec3(1.0f, 1.0f, 0.0f));
    g_ball_position += g_ball_movement;
    g_ball_position = glm::vec3(0.0f, 0.0f, 0.0f);
    g_ball_velocity.x = initial_speed * cos(random_angle);
    g_ball_velocity.y = initial_speed * sin(random_angle);
    
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    g_player_1_texture_id = load_texture(PLAYER_1_SPRITE_FILEPATH);
    g_player_2_texture_id = load_texture(PLAYER_2_SPRITE_FILEPATH);
    g_ball_texture_id = load_texture(BALL_SPRITE_FILEPATH);
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    g_player_1_movement = glm::vec3(0.0f);
    g_player_2_movement = glm::vec3(0.0f);
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_app_status = TERMINATED;
                        break;
                        
                    case SDLK_t:
                        // Toggle Player 2 auto mode
                        player_2_auto_mode = !player_2_auto_mode;
                        break;
                        
                    default:
                        break;
                }
                break;
                
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
   // while(!game_over){ //only allow them to play the game when the game is not over
        //Player 1 controls, can only move up or down
        if (key_state[SDL_SCANCODE_W]) {
            g_player_1_movement.y = 1.0f; //up
        } else if (key_state[SDL_SCANCODE_S]) {
            g_player_1_movement.y = -1.0f; //down
        }
        //regular player 2 controls
        if (!player_2_auto_mode) {
            if (key_state[SDL_SCANCODE_UP]) {
                g_player_2_movement.y = 1.0f; //up
            } else if (key_state[SDL_SCANCODE_DOWN]) {
                g_player_2_movement.y = -1.0f; //down
                std::cout << g_player_2_movement.y << std::endl;
            }
        }
        //clamp, so they can't go off the screen
        constexpr float limit = 3.0f;
        g_player_1_position.y = glm::clamp(g_player_1_position.y, -limit, limit);
        g_player_2_position.y = glm::clamp(g_player_2_position.y, -limit, limit);
        
        // This makes sure that the player can't "cheat" their way into moving
        if (glm::length(g_player_1_movement) > 1.0f) {
            g_player_1_movement = glm::normalize(g_player_1_movement);
        }
        if (glm::length(g_player_2_movement) > 1.0f) {
            g_player_2_movement = glm::normalize(g_player_2_movement);
        }
    //}
    
}
//void reset_ball() //reset the ball to the center of the screen with random x and y velocity
//{
//    g_ball_position = glm::vec3(0.0f, 0.0f, 0.0f);
//    float initial_speed = 5.0f;
//    float random_angle = glm::radians((rand() % 120) - 60);
//
//    g_ball_velocity.x = initial_speed * cos(random_angle);
//    g_ball_velocity.y = initial_speed * sin(random_angle);
//
//}

void update()
{
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    //when player 2 is in auto mode
    if (player_2_auto_mode) {
        //oscilate up and down
        g_player_2_position.y = 3.0f *cos(ticks); //mult to inc speed
        g_player_2_position.y = glm::clamp(g_player_2_position.y, -3.0f, 3.0f); //keep within bounds
    } else {
        g_player_2_position += g_player_2_movement * g_player_2_speed * delta_time;
    }

    // Update Player 1
    g_player_1_position += g_player_1_movement * g_player_1_speed * delta_time;

    // Update matrices for rendering
    g_player_1_matrix = glm::mat4(1.0f);
    g_player_1_matrix = glm::translate(g_player_1_matrix, INIT_POS_PLAYER_1 + g_player_1_position);
    g_player_2_matrix = glm::mat4(1.0f);
    g_player_2_matrix = glm::translate(g_player_2_matrix, INIT_POS_PLAYER_2 + g_player_2_position);
    
    // Scale the players
    g_player_1_matrix = glm::scale(g_player_1_matrix, INIT_SCALE_PLAYER_1);
    g_player_2_matrix = glm::scale(g_player_2_matrix, INIT_SCALE_PLAYER_2);
    
    // Update ball position based on its movement
    g_ball_position += g_ball_velocity * delta_time;
    
    g_ball_matrix = glm::mat4(1.0f);
    g_ball_matrix = glm::translate(g_ball_matrix, INIT_POS_BALL);
    g_ball_matrix = glm::translate(g_ball_matrix, g_ball_position);
    
    //collisions
    
    
    //ball collision with top and bottom walls, ball bounces
    if (g_ball_position.y >= 3.0f || g_ball_position.y <= -3.0f) {
        g_ball_velocity.y *= -1.0f; //bounce that carrot
    }
    
    //ball collision with L and R walls, L = player_2 scores, R = player_1 scores, ball resets
    if (g_ball_position.y >= 3.0f || g_ball_position.y <= -3.0f) {
        g_ball_velocity.y *= -1.0f; //bounce that carrot
    }
    
    //ball collision with top and bottom walls, ball bounces

    //ball collision with L and R walls, L = player_2 scores, R = player_1 scores, ball resets
    
    //ball and player 1
    float x_distance = fabs(g_player_1_position.x + INIT_POS_PLAYER_1.x - INIT_POS_BALL.x) -
        ((INIT_SCALE_BALL.x + INIT_SCALE_PLAYER_1.x) / 2.0f);
    float y_distance = fabs(g_player_1_position.y + INIT_POS_PLAYER_1.y - INIT_POS_BALL.y) -
        ((INIT_SCALE_BALL.y + INIT_SCALE_PLAYER_1.y) / 2.0f);
    
    if (x_distance < 0.0f && y_distance < 0.0f)
    {
        std::cout << std::time(nullptr) << ": Collision.\n";
    }
    
    //ball and player 2
}
void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}
void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };
    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };
    
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    // Bind texture
    draw_object(g_player_1_matrix, g_player_1_texture_id);
    draw_object(g_player_2_matrix, g_player_2_texture_id);
    draw_object(g_ball_matrix, g_ball_texture_id);
    
    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    SDL_GL_SwapWindow(g_display_window);
}
void shutdown() { SDL_Quit(); }
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

