/**
* Author: Elizabeth Castroverde (eac9917)
* Assignment: Simple 2D Scene
* Date due: 2024-09-28, 11:58pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

enum AppStatus { RUNNING, TERMINATED };

constexpr int WINDOW_WIDTH  = 640 * 2,
              WINDOW_HEIGHT = 480 * 2;

constexpr float BG_RED     = 0.9765625f,
                BG_GREEN   = 0.97265625f,
                BG_BLUE    = 0.9609375f,
                BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X      = 0,
              VIEWPORT_Y      = 0,
              VIEWPORT_WIDTH  = WINDOW_WIDTH,
              VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
               F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

constexpr GLint NUMBER_OF_TEXTURES = 1, // to be generated, that is
                LEVEL_OF_DETAIL    = 0, // mipmap reduction image level
                TEXTURE_BORDER     = 0; // this value MUST be zero

//for the cat changing scale
constexpr float BASE_AMP = 1.0f,
                MAX_AMP = 1.0f,
                PULSE_SPEED = 2.0f;

/**Image sources:
 Cat: https://opensea.io/assets/matic/0x2953399124f0cbb46d2cbacd8a89cf0599974963/102397542235443926589854408932315867098078178487900162191082109999403960893441
 
 Dog: https://opensea.io/assets/matic/0x2953399124f0cbb46d2cbacd8a89cf0599974963/102397542235443926589854408932315867098078178487900162191082109998304449265665
 */
constexpr char CAT_SPRITE_FILEPATH[]    = "cat.png",
               DOG_SPRITE_FILEPATH[] = "dog.png";

constexpr float ROT_INCREMENT = 1.0f;

SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();

glm::mat4 g_view_matrix,
          g_cat_matrix,
          g_dog_matrix,
          g_projection_matrix;

//for delta time dependent transformationsa
float g_previous_ticks = 0.0f;
float g_dog_x = 0.0f;
float g_cat_x = 0.0f;

glm::vec3 g_rotation_cat    = glm::vec3(0.0f, 0.0f, 0.0f),
          g_rotation_dog = glm::vec3(0.0f, 0.0f, 0.0f);

GLuint g_cat_texture_id,
       g_dog_texture_id;

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

    stbi_image_free(image);

    return textureID;
}


void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);

    g_display_window = SDL_CreateWindow("Dog Chasing Cat!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    if (g_display_window == nullptr)
    {
        std::cerr << "Error: SDL window could not be created.\n";
        SDL_Quit();
        exit(1);
    }

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_cat_matrix       = glm::mat4(1.0f);
    g_dog_matrix     = glm::mat4(1.0f);
    g_view_matrix       = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    g_cat_texture_id   = load_texture(CAT_SPRITE_FILEPATH);
    g_dog_texture_id = load_texture(DOG_SPRITE_FILEPATH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //set background color to light green
    glClearColor(0.5f, 0.8f, 0.5f, 1.0f);
}


void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            g_app_status = TERMINATED;
        }
    }
    
}

void update()
{
    //delta time
    float g_ticks = (float) SDL_GetTicks() / 1000.0f;
    float g_delta_time = g_ticks - g_previous_ticks;
    g_previous_ticks = g_ticks;

    g_dog_x += 1.0f * g_delta_time;
    g_cat_x += 1.0f * g_delta_time;

    
    g_rotation_cat.y += ROT_INCREMENT * g_delta_time;
    g_rotation_dog.y += -1 * ROT_INCREMENT * g_delta_time;

    //reset
    g_cat_matrix    = glm::mat4(1.0f);
    g_dog_matrix = glm::mat4(1.0f);
    
    //cat moves diagonally on the screen back and forth (top right to bottom left) and gets bigger when it turns around
    g_cat_matrix = glm::translate(g_cat_matrix, glm::vec3(2.0f*glm::sin(g_cat_x), 2.0f*glm::sin(g_cat_x), 0.0f));
    
    glm::vec3 scale_factors = glm::vec3(BASE_AMP + MAX_AMP * 0.25f*glm::sin(g_ticks*PULSE_SPEED),
                                      BASE_AMP + MAX_AMP * 0.25f*glm::sin(g_ticks*PULSE_SPEED),
                                                                            0.0f);
    g_cat_matrix = glm::scale(g_cat_matrix, scale_factors);
    
    //dog rotates and chases cat around in a circle
    //dog moves relative to the cat
    g_dog_matrix = glm::translate(g_cat_matrix, glm::vec3(1.0f*glm::cos(g_dog_x), 1.0f*glm::sin(g_dog_x), 0.0f));
    g_dog_matrix = glm::rotate(g_dog_matrix,
                                  g_rotation_dog.y,
                                  glm::vec3(0.0f, 1.0f, 0.0f));
}


void draw_object(glm::mat4 &object_g_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_g_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}


void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertices
    float vertices[] =
    {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };

    // Textures
    float texture_coordinates[] =
    {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false,
                          0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT,
                          false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    draw_object(g_cat_matrix, g_cat_texture_id);
    draw_object(g_dog_matrix, g_dog_texture_id);

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
