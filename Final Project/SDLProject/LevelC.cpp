#include "LevelC.h"
#include "Utility.h"

#define LEVEL_WIDTH 20
#define LEVEL_HEIGHT 14

constexpr char SPRITESHEET_FILEPATH[] = "assets/images/pink.png",
                FONTSHEET_FILEPATH[]         = "assets/fonts/font1.png",
                ENEMY_FILEPATH[]       = "assets/images/jumping_ai.png",
                ENEMY_GUARD_FILEPATH[]       = "assets/images/onion.png";

unsigned int LEVELC_DATA[] =
{
    5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5,
    5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5,
    5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 5,
    5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 5,
    5, 0, 0, 0, 0, 0, 0, 5, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 5,
    5, 2, 0, 0, 2, 2, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 5,
    5, 2, 2, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 5,
    5, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3, 4, 3,
};

LevelC::~LevelC()
{
    Mix_FreeMusic(m_game_state.bgm);
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeChunk(m_game_state.win_sfx);
    Mix_FreeChunk(m_game_state.lose_sfx);
    delete [] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    delete    m_game_state.ammo;
}

void LevelC::initialise()
{
    m_game_state.next_scene_id = -1;
    
    GLuint map_texture_id = Utility::load_texture("assets/images/grocery_tiles.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELC_DATA, map_texture_id, 1.0f, 5, 1);
    
    int player_walking_animation[4][4] =
    {
        {12,13,14,15},  //left
        {8,9,10,11}, //right
        {4,5,6,7}, //up
        {0,1,2,3} ,//down
    };

    glm::vec3 acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
    
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
           4,                         // animation column amount
           4,                         // animation row amount
           1.0f,                      // width
           1.0f,                       // height
           PLAYER
    );
        
    m_game_state.player->set_position(glm::vec3(1.0f, 0.0f, 0.0f));
    m_game_state.player->set_scale(glm::vec3(0.8f, 0.8f, 0.0f));
    m_game_state.player->set_jumping_power(3.0f);
    m_game_state.player->set_lives(m_game_state.lives);
    
    /**Enemies' stuff */
    int enemy_jumping_animation[2][1] =
    {
        {0},
        {1},
    };
    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);

    //jumping enemies
    m_game_state.enemies = new Entity[ENEMY_COUNT];
    for(int i = 0; i < ENEMY_COUNT-1; i++){
        m_game_state.enemies[i] =  Entity(enemy_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, JUMPER, JUMPING);
        m_game_state.enemies[i].set_entity_type(ENEMY);
        m_game_state.enemies[i].set_ai_type(JUMPER);
        m_game_state.enemies[i].set_jumping_power(4.0f);
        m_game_state.enemies[i].set_movement(glm::vec3(0.0f));
        m_game_state.enemies[i].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
        //animating enemy
        m_game_state.enemies[i].set_animation_time(0.0f);
        m_game_state.enemies[i].set_animation_cols(2);
        m_game_state.enemies[i].set_animation_rows(1);
        m_game_state.enemies[i].set_animation_index(0);
        m_game_state.enemies[i].set_animation_frames(2);
        m_game_state.enemies[i].set_jumping(enemy_jumping_animation);
        m_game_state.enemies[i].set_scale(glm::vec3(0.8f, 0.8f, 0.0f));
    }
    
    
    m_game_state.enemies[0].set_position(glm::vec3(8.5f, -2.0f, 0.0f));
    m_game_state.enemies[1].set_position(glm::vec3(16.5f, -2.0f, 0.0f));
    
    
    //guard enemies
    GLuint enemy_guard_texture_id = Utility::load_texture(ENEMY_GUARD_FILEPATH);
    m_game_state.enemies[2] =  Entity(enemy_guard_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, GUARD, IDLE);
    m_game_state.enemies[2].set_entity_type(ENEMY);
    m_game_state.enemies[2].set_scale(glm::vec3(0.8f, 0.8f, 0.0f));
    m_game_state.enemies[2].set_movement(glm::vec3(0.0f));
    m_game_state.enemies[2].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    m_game_state.enemies[2].set_position(glm::vec3(12.5f, 0.0f, 0.0f));//spawn on platform
    
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("assets/audio/bgmC.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1); //-1 = loop forever
    Mix_VolumeMusic(20.0f);
    
    m_game_state.jump_sfx = Mix_LoadWAV("assets/audio/jump.wav");
    m_game_state.lose_sfx= Mix_LoadWAV("assets/audio/lose.wav");
    m_game_state.win_sfx= Mix_LoadWAV("assets/audio/win.wav");
}

void LevelC::update(float delta_time)
{
    if(!m_game_state.pause_screen){
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
        
        if (m_game_state.player->get_game_status() && !m_game_state.sound_played) {
            if(m_game_state.player->get_win_status()){
                Mix_PlayChannel(-1, m_game_state.win_sfx, 0);
                m_game_state.sound_played = true;
            }else{
                Mix_PlayChannel(-1, m_game_state.lose_sfx, 0);
                m_game_state.sound_played = true;
            }
        }
    }
}

void LevelC::render(ShaderProgram *program)
{
    m_game_state.map->render(program);
    m_game_state.player->render(program);
    for (int i = 0; i < ENEMY_COUNT; i++)    m_game_state.enemies[i].render(program);
    //player wins the game when they get to the end of level C, so X = 18.0f and y = -6.0f
    int lives = m_game_state.player->get_lives();
    glm::vec3 player_pos = m_game_state.player->get_position();
    
    //spotlight logic
    program->set_spotlight(1);
    program->set_light_position_matrix(player_pos);

    if(m_game_state.player->get_game_status()){ //true = over
        if(m_game_state.player->get_win_status()){
            Utility::draw_text(program, Utility::load_texture(FONTSHEET_FILEPATH), "You Win", 0.5f, 0.005f, glm::vec3(player_pos.x-0.75f, player_pos.y +2.0f, 0.0f));
        }else{
            Utility::draw_text(program, Utility::load_texture(FONTSHEET_FILEPATH), "You Lose", 0.5f, 0.005f, glm::vec3(player_pos.x-0.75f, player_pos.y +2.0f, 0.0f));
        }
    }else{
        Utility::draw_text(program, Utility::load_texture(FONTSHEET_FILEPATH), "Lives:" + std::to_string(lives), 0.3f, 0.005f, glm::vec3(player_pos.x-0.75f, player_pos.y +1.0f, 0.0f)); //lives above the players head
    }
}
