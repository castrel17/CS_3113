#include "Menu.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

constexpr char SPRITESHEET_FILEPATH[] = "assets/images/player.png",
    FONTSHEET_FILEPATH[]         = "assets/fonts/font1.png",
BACKGROUND_FILEPATH[]         = "assets/images/bkgd.png",
           ENEMY_FILEPATH[]       = "assets/images/AI1.png";

unsigned int MENU_DATA[] =
{
    0, 0, 0,0 , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0,0 , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0,0 , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0,0 , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0,0 , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0,0 , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0,0 , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0,0 , 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

Menu::~Menu()
{
    delete [] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void Menu::initialise()
{
    m_game_state.next_scene_id = -1;
    
    GLuint map_texture_id = Utility::load_texture("assets/images/tilemap_packed.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, MENU_DATA, map_texture_id, 1.0f, 5, 1);
    
    int player_walking_animation[4][3] =
    {
        {6, 14, 22},  //left
        {2, 10, 18}, //right
        {0, 8, 16}, //up
        {20, 20, 20} ,//down
    };

    glm::vec3 acceleration = glm::vec3(0.0f, -4.81f, 0.0f);
    
    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    
    m_game_state.player = new Entity(
           player_texture_id,         // texture id
           5.0f,                      // speed
           acceleration,              // acceleration
           3.0f,                      // jumping power
           player_walking_animation,  // animation index sets
           0.0f,                      // animation time
           3,                         // animation frame amount
           0,                         // current animation index
           8,                         // animation column amount
           3,                         // animation row amount
           1.0f,                      // width
           1.0f,                       // height
           PLAYER
    );
    m_game_state.player->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
    m_game_state.player->deactivate();
    // Jumping
    m_game_state.player->set_jumping_power(3.0f);
    
    /**
    Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);

    m_game_state.enemies = new Entity[ENEMY_COUNT];

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
    m_game_state.enemies[i] =  Entity(enemy_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, GUARD, IDLE);
    }


    m_game_state.enemies[0].set_position(glm::vec3(8.0f, 0.0f, 0.0f));
    m_game_state.enemies[0].set_movement(glm::vec3(0.0f));
    m_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    
    
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("assets/audio/bgm2.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1); //-1 = loop forever
    Mix_VolumeMusic(10.0f);
    
    m_game_state.jump_sfx = Mix_LoadWAV("assets/audio/jump.wav");
}

void Menu::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);
    
    if (m_game_state.player->get_position().y < -10.0f) m_game_state.next_scene_id = 1;
}

void Menu::render(ShaderProgram *program)
{
    GLuint g_bkgd = Utility::load_texture(BACKGROUND_FILEPATH);
    Utility::render_background(g_bkgd, program);
    Utility::draw_text(program, Utility::load_texture(FONTSHEET_FILEPATH), "Welcome!", 0.5f, 0.005f, glm::vec3(3.0f, -2.0f, 0.0f));
    Utility::draw_text(program, Utility::load_texture(FONTSHEET_FILEPATH), "Press enter to start", 0.3f, 0.01f, glm::vec3(2.0f, -2.5f, 0.0f));
    m_game_state.map->render(program);
    m_game_state.player->render(program);
}
