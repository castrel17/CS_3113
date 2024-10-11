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
 
 Player 1:  left W (up), S (down)
  Player 2: right, controls: up key, down key
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
#include <vector>
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


//these values change so can't be constexpr
bool game_over = false;
bool player_2_auto_mode = false; //flag to track auto mode for player 2
bool player_1_win = false;

/*sources: me lol */
constexpr char PLAYER_1_SPRITE_FILEPATH[] = "ghost.png", //on the left
               PLAYER_2_SPRITE_FILEPATH[] = "ghost.png", //on the right
               BALL_SPRITE_FILEPATH[]  = "pumpkin.png",
               FONTSHEET_FILEPATH[]   = "font.png";

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
float g_player_1_speed = 2.0f;
float g_player_2_speed = 2.0f;
float initial_speed = 2.0f;
float random_angle = glm::radians(static_cast<float>((rand() % 120) - 60));


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

//TEXT STUFF
constexpr int FONTBANK_SIZE = 16;
GLuint g_font_texture_id;

void draw_text(ShaderProgram *program, GLuint font_texture_id, std::string text,
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
    
    program->set_model_matrix(model_matrix);
    glUseProgram(program->get_program_id());
    
    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0,
                          vertices.data());
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0,
                          texture_coordinates.data());
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());
    
    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int) (text.size() * 6));
    
    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}


void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Spooky Not Pong!",
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
    
    g_font_texture_id = load_texture(FONTSHEET_FILEPATH);
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //set background color to black
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
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
            g_player_1_movement.y = g_player_1_speed; //up
        } else if (key_state[SDL_SCANCODE_S]) {
            g_player_1_movement.y = -g_player_1_speed; //down
        }
        //regular player 2 controls
        if (!player_2_auto_mode) {
            if (key_state[SDL_SCANCODE_UP]) {
                g_player_2_movement.y = g_player_2_speed; //up
            } else if (key_state[SDL_SCANCODE_DOWN]) {
                g_player_2_movement.y = -g_player_2_speed; //down
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
void reset_ball(float dir) //reset the ball to the center of the screen with random x and y velocity
{//when player 1 scores then the ball heads towards player 2 and when player 2 scores the ball heads towards player one
    float new_random_angle = glm::radians(static_cast<float>((rand() % 120) - 60));
    g_ball_position = glm::vec3(0.0f, 0.0f, 0.0f);

    g_ball_velocity.x = initial_speed * dir*cos(new_random_angle);
    g_ball_velocity.y = initial_speed * dir*sin(new_random_angle);

}

void update()
{
    if(!game_over){
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
        //ball collision with top and bottom walls, ball bounces, make sure to correct position so it does not get stuck
        if (g_ball_position.y >= 3.4f) { //top
            g_ball_velocity.y *= -1.0f;
            g_ball_position.y = 3.4f - (INIT_SCALE_BALL.y / 2.0f); //bottom
        }
        else if (g_ball_position.y <= -3.4f) {
            g_ball_velocity.y *= -1.0f;
            g_ball_position.y = -3.4f + (INIT_SCALE_BALL.y / 2.0f);
        }
        
        //ball collision with L and R walls, L = player_2 scores, R = player_1 scores, ball resets
        if (g_ball_position.x >= 6.0f) { //player 1 scores
            player_1_win = true;
            std::cout << "Player 1 wins!" << std::endl;
            game_over = true;
            //reset_ball(1.0f); //ball heads towards player 2
        }else if (g_ball_position.x <= -6.0f) { //player 2 scores
            std::cout << "Player 2 wins!" << std::endl;
            game_over = true;
            //reset_ball(-1.0f); //ball heads towards player 1
        }
        
        //---------------------------------DOES NOT WORK YET ---------------------------------
        //ball and player 1
        float x_distance_p1 = fabs(g_ball_position.x - (INIT_POS_PLAYER_1.x + g_player_1_position.x)) - ((INIT_SCALE_PLAYER_1.x + INIT_SCALE_BALL.x) / 2.0f);


        float y_distance_p1 = fabs(g_ball_position.y - g_player_1_position.y) -
                              ((INIT_SCALE_PLAYER_1.y + INIT_SCALE_BALL.y) / 2.0f);
        if (x_distance_p1 < 0.0f && y_distance_p1 < 0.0f)
        {
            g_ball_velocity.x *= -1.0f; //bounce
            std::cout << "Collision with Player 1!" << std::endl;
            //g_ball_position.x = g_player_1_position.x + .15f;
        }
        
        //ball and player 2
        float x_distance_p2 = fabs(g_ball_position.x - (INIT_POS_PLAYER_2.x + g_player_2_position.x)) - ((INIT_SCALE_PLAYER_2.x + INIT_SCALE_BALL.x) / 2.0f);


        float y_distance_p2 = fabs(g_ball_position.y - g_player_2_position.y) -
                              ((INIT_SCALE_PLAYER_2.y + INIT_SCALE_BALL.y) / 2.0f);

        //collision check
        if (x_distance_p2 < 0.0f && y_distance_p2 < 0.0f)
        {
            g_ball_velocity.x *= -1.0f;  //bounce
            std::cout << "Collision with Player 2!" << std::endl;
            //g_ball_position.x = g_player_2_position.x -.15f;
        }
    }
}


void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
    if(!game_over){
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
        
    }else{
        if (player_1_win) {
                    draw_text(&g_shader_program, g_font_texture_id, "Player 1 Wins!", 0.5f, 0.05f, glm::vec3(-1.5f, 0.0f, 0.0f));
                } else {
                    draw_text(&g_shader_program, g_font_texture_id, "Player 2 Wins!", 0.5f, 0.05f, glm::vec3(-1.5f, 0.0f, 0.0f));
                }
    }
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

