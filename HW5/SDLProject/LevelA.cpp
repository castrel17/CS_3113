#include "LevelA.h"
#include "Utility.h"

#define LEVEL_WIDTH 20
#define LEVEL_HEIGHT 8

constexpr char SPRITESHEET_FILEPATH[] = "assets/images/player.png",
           AI1_FILEPATH[]       = "assets/images/onion.png",
FONTSHEET_FILEPATH[]         = "assets/fonts/font1.png";

unsigned int LEVELA_DATA[] = {
    5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 5, 0,
    5, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 2, 5, 0,
    5, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 2, 2, 5, 0,
    5, 0, 0, 2, 1, 0, 2, 2, 2, 0, 1, 1, 1, 1, 0, 2, 2, 2, 5, 0,
    5, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 0,
    5, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 0,
};

LevelA::~LevelA()
{
    Mix_FreeMusic(m_game_state.bgm);
    Mix_FreeChunk(m_game_state.jump_sfx);
    delete [] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    delete    m_game_state.ammo;
}

void LevelA::initialise()
{
    m_game_state.next_scene_id = -1;
    
    GLuint map_texture_id = Utility::load_texture("assets/images/grocery_tiles.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELA_DATA, map_texture_id, 1.0f, 5, 1);
    
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
    m_game_state.player->set_position(glm::vec3(1.0f, 0.0f, 0.0f));
    m_game_state.player->set_scale(glm::vec3(0.8f, 0.8f, 0.0f));
    m_game_state.player->set_jumping_power(3.0f);
    
    /**Enemies' stuff */
    // ––––– AI1 (GUARD) ––––– //
    m_game_state.enemies = new Entity[ENEMY_COUNT];
    
    GLuint ai1_texture_id = Utility::load_texture(AI1_FILEPATH);
    
    m_game_state.enemies[0] =  Entity(ai1_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, GUARD, IDLE);
    m_game_state.enemies[0].set_entity_type(ENEMY);
    m_game_state.enemies[0].set_scale(glm::vec3(0.8f, 0.8f, 0.0f));
    m_game_state.enemies[0].set_movement(glm::vec3(0.0f));
    m_game_state.enemies[0].set_position(glm::vec3(8.0f, -2.0f, 0.0f)); //spawn on platform
    m_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("assets/audio/bgm2.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1); //-1 = loop forever
    Mix_VolumeMusic(20.0f);
    
    m_game_state.jump_sfx = Mix_LoadWAV("assets/audio/jump.wav");
    m_game_state.lose_sfx= Mix_LoadWAV("assets/audio/lose.wav");
}

void LevelA::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);
    
    for (int i = 0; i < ENEMY_COUNT; i++) m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0,
                                                                         m_game_state.map);
    m_game_state.lives = m_game_state.player->get_lives();
    if (m_game_state.player->get_position().y < -10.0f) {
        m_game_state.next_scene_id = 1;
    }
    
    if (m_game_state.player->get_game_status() && !m_game_state.sound_played) {
        Mix_PlayChannel(-1, m_game_state.lose_sfx, 0);
        m_game_state.sound_played = true;
    }
}

void LevelA::render(ShaderProgram *program)
{
    m_game_state.map->render(program);
    m_game_state.player->render(program);
    m_game_state.enemies->render(program);
    int lives = m_game_state.player->get_lives();
    glm::vec3 player_pos = m_game_state.player->get_position();
    
    if(m_game_state.player->get_game_status()){ //true = over
        Utility::draw_text(program, Utility::load_texture(FONTSHEET_FILEPATH), "You Lose", 0.5f, 0.005f, glm::vec3(player_pos.x-0.75f, player_pos.y +2.0f, 0.0f)); //lives above the players head
    }else{
        Utility::draw_text(program, Utility::load_texture(FONTSHEET_FILEPATH), "Lives:" + std::to_string(lives), 0.3f, 0.005f, glm::vec3(player_pos.x-0.75f, player_pos.y +1.0f, 0.0f)); //lives above the players head
    }
}
