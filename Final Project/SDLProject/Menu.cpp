#include "Menu.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

constexpr char SPRITESHEET_FILEPATH[] = "assets/images/pink.png",
                FONTSHEET_FILEPATH[]         = "assets/fonts/font1.png",
ORB_FILEPATH[]       = "assets/images/orb.png",
            ENEMY_FILEPATH[]       = "assets/images/AI1.png";


GLuint g_bkgd_texture_id;
glm::mat4 g_bkgd_matrix;

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
    Mix_FreeMusic(m_game_state.bgm);
    Mix_FreeChunk(m_game_state.stomp_sfx);
    delete [] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
}

void Menu::initialise()
{
    m_game_state.next_scene_id = -1;
    
    GLuint map_texture_id = Utility::load_texture("assets/images/grocery_tiles.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, MENU_DATA, map_texture_id, 1.0f, 5, 1);
    
    int player_walking_animation[4][4] =
    {
        {12,13,14,15},  //left
        {8,9,10,11}, //right
        {4,5,6,7}, //up
        {0,1,2,3} ,//down
    };

    int player_attacking_animation[4][4] =
    {
        {28,29,30,13},  //left
        {20,21,22,23}, //right
        {8,9,10,11}, //up
        {4,5,6,7} ,//down
    };
    glm::vec3 acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
    
    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    
    m_game_state.player = new Entity(
           player_texture_id,         // texture id
           3.0f,                      // speed
           acceleration,              // acceleration
           3.0f,                      // jumping power
           player_walking_animation,  // animation index sets
           player_attacking_animation,
           0.0f,                      // animation time
           3,                         // animation frame amount
           0,                         // current animation index
           4,                         // animation column amount
           8,                         // animation row amount
           0.4f,                      // width
           0.4f,                       // height
           PLAYER
    );
    m_game_state.player->set_position(glm::vec3(-10.0f, 0.0f, 0.0f));
    m_game_state.player->set_scale(glm::vec3(0.5f, 0.5f, 0.0f));
   
    /**Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);
    m_game_state.enemies = new Entity[ENEMY_COUNT];
    m_game_state.enemies[0] =  Entity(enemy_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, GUARD, IDLE);
    m_game_state.enemies[0].set_position(glm::vec3(-10.0f, 0.0f, 0.0f));
    m_game_state.enemies[0].set_movement(glm::vec3(0.0f));
    
    
    /**ORB*/
    GLuint orb_texture_id = Utility::load_texture(ORB_FILEPATH);
    m_game_state.orb= new Entity(orb_texture_id, 0.0f, 0.5f, 0.5f, ORB);
    m_game_state.orb->set_position(glm::vec3(18.0f, -1.0f, 0.0f)); //spawn at the end of maze
    m_game_state.orb->set_scale(glm::vec3(0.4f, 0.4f, 0.0f));
    
    /**BGM and SFX*/
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("assets/audio/bgm2.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1); //-1 = loop forever
    Mix_VolumeMusic(20.0f);
    
    m_game_state.stomp_sfx = Mix_LoadWAV("assets/audio/jump.wav");
}

void Menu::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map,m_game_state.orb);
    m_game_state.player->deactivate();
    if (m_game_state.player->get_position().y < -10.0f) m_game_state.next_scene_id = 1;
}

void Menu::render(ShaderProgram *program)
{
    Utility::draw_text(program, Utility::load_texture(FONTSHEET_FILEPATH), "Welcome!", 0.5f, 0.005f, glm::vec3(3.0f, -2.0f, 0.0f));
    Utility::draw_text(program, Utility::load_texture(FONTSHEET_FILEPATH), "Press enter to start", 0.5f, 0.005f, glm::vec3(0.25f, -2.5f, 0.0f));
    program->set_spotlight(0);
    Utility::draw_text(program, Utility::load_texture(FONTSHEET_FILEPATH), "Press esc to pause", 0.5f, 0.005f, glm::vec3(0.25f, -3.0f, 0.0f));
    m_game_state.map->render(program);
    m_game_state.player->render(program);
    
    program->set_model_matrix(g_bkgd_matrix);
    glBindTexture(GL_TEXTURE_2D, g_bkgd_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
