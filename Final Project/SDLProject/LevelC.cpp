#include "LevelC.h"
#include "Utility.h"

#define LEVEL_WIDTH 20
#define LEVEL_HEIGHT 8

constexpr char SPRITESHEET_FILEPATH[] = "assets/images/black.png",
           AI1_FILEPATH[]       = "assets/images/onion.png",
            ORB_FILEPATH[]       = "assets/images/orb.png",
            FONTSHEET_FILEPATH[]         = "assets/fonts/font1.png";

//three zones with three separate AI in varying difficulty that the player needs to fight off
//first enemy is guard with health of 4 --> rat
//second enemy is the cyclone with health of 5 --> drone
//third enemy is the shooter with health of 7 --> robocop
unsigned int LEVELC_DATA[] = {
    5, 2, 1, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2,
    1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 3,
    5, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 4,
    4, 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 5,
    3, 0, 0, 0, 4, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 1,
    2, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    1, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    5, 3, 4, 5, 4, 3, 4, 5, 4, 2, 1, 3, 4, 5, 1, 3, 2, 4, 5, 3,
};
int stomped = 0;
LevelC::~LevelC()
{
    Mix_FreeMusic(m_game_state.bgm);
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeChunk(m_game_state.lose_sfx);
    delete [] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    delete    m_game_state.ammo;
}

void LevelC::initialise()
{
    m_game_state.next_scene_id = -1;
    
    GLuint map_texture_id = Utility::load_texture("assets/images/cyber_tiles.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELC_DATA, map_texture_id, 1.0f, 4, 1);
    
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
           3.0f,                      // speed
           acceleration,              // acceleration
           3.0f,                      // jumping power
           player_walking_animation,  // animation index sets
           0.0f,                      // animation time
           3,                         // animation frame amount
           0,                         // current animation index
           4,                         // animation column amount
           4,                         // animation row amount
           0.4f,                      // width
           0.4f,                       // height
           PLAYER
    );
    m_game_state.player->set_position(glm::vec3(1.0f, -1.0f, 0.0f));
    m_game_state.player->set_scale(glm::vec3(0.4f, 0.4f, 0.0f));
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
    m_game_state.enemies[0].set_lives(5);
    
    /**ORB*/ //only spawn the orb if all of the enemies are defeated
    GLuint orb_texture_id = Utility::load_texture(ORB_FILEPATH);
    m_game_state.orb= new Entity(orb_texture_id, 0.0f, 0.5f, 0.5f, ORB);
    m_game_state.orb->set_position(glm::vec3(18.0f, -1.0f, 0.0f)); //spawn at the end of maze
    m_game_state.orb->set_scale(glm::vec3(0.4f, 0.4f, 0.0f));
    
    /**BGM and SFX*/
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("assets/audio/bgmC.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1); //-1 = loop forever
    Mix_VolumeMusic(20.0f);
    
    m_game_state.jump_sfx = Mix_LoadWAV("assets/audio/jump.wav");
    m_game_state.lose_sfx= Mix_LoadWAV("assets/audio/lose.wav");
}

void LevelC::update(float delta_time)
{
    if(!m_game_state.pause_screen){
        m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT + 1, m_game_state.map, m_game_state.orb);
        
        if(ENEMY_COUNT == m_game_state.player->get_stomp_count()){
            m_game_state.orb->update(delta_time, m_game_state.player, m_game_state.player, 1, m_game_state.map, m_game_state.orb);
        }
        
        if(ENEMY_COUNT == stomped){
            m_game_state.orb->update(delta_time, m_game_state.player, m_game_state.player, 1, m_game_state.map, m_game_state.orb);
        }
        
        for (int i = 0; i < ENEMY_COUNT; i++){
            m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map, m_game_state.orb);
            if(m_game_state.enemies[i].get_lives()<=0 && !m_game_state.enemies[i].get_stomped()){
                stomped++;
                m_game_state.enemies[i].set_stomped(true);
            }
        }
        m_game_state.lives = m_game_state.player->get_lives();
        
        if (m_game_state.player->get_game_status() && !m_game_state.sound_played) {
            Mix_PlayChannel(-1, m_game_state.lose_sfx, 0);
            m_game_state.sound_played = true;
        }
        
        if (m_game_state.player->get_game_status() && !m_game_state.sound_played) {
            if(m_game_state.orb->get_hit_orb()){
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
    m_game_state.enemies->render(program);
    
    //only render the orb when all enemies are killed
    if(ENEMY_COUNT == stomped){
        m_game_state.orb->render(program);
    }
    int lives = m_game_state.player->get_lives();
    glm::vec3 player_pos = m_game_state.player->get_position();
    
    //spotlight logic
    program->set_spotlight(1);
    program->set_light_position_matrix(player_pos);
    
    if(m_game_state.player->get_game_status()){ //true = over
        if(m_game_state.player->get_win_status()){
            Utility::draw_text(program, Utility::load_texture(FONTSHEET_FILEPATH), "You Win", 0.5f, 0.005f, glm::vec3(player_pos.x-0.75f, player_pos.y +1.0f, 0.0f));
        }else{
            Utility::draw_text(program, Utility::load_texture(FONTSHEET_FILEPATH), "You Lose", 0.5f, 0.005f, glm::vec3(player_pos.x-0.75f, player_pos.y +1.0f, 0.0f));
        }
    }else{
        Utility::draw_text(program, Utility::load_texture(FONTSHEET_FILEPATH), "Lives:" + std::to_string(lives), 0.3f, 0.005f, glm::vec3(player_pos.x-0.75f, player_pos.y +1.0f, 0.0f)); //lives above the players head
    }
}

