#include "LevelB.h"
#include "Utility.h"

#define LEVEL_WIDTH 20
#define LEVEL_HEIGHT 8

constexpr char SPRITESHEET_FILEPATH[] = "assets/images/black.png",
           AI1_FILEPATH[]       = "assets/images/onion.png",
            ORB_FILEPATH[]       = "assets/images/orb.png",
            FONTSHEET_FILEPATH[]         = "assets/fonts/font1.png";


//orb spawns in the center of the city when all the enemies are killed in a certain order
//the hint is
unsigned int LEVELB_DATA[] = {
    5, 2, 1, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,
    4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    5, 3, 4, 5, 4, 3, 4, 5, 4, 2, 1, 3, 4, 5, 1, 3, 2, 4, 5, 3,
};

std::vector<int> expected_kill_order = {0, 1, 2};
std::vector<int> actual_kill_order;

LevelB::~LevelB()
{
    Mix_FreeMusic(m_game_state.bgm);
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeChunk(m_game_state.lose_sfx);
    delete [] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    delete    m_game_state.ammo;
}

void LevelB::initialise()
{
    m_game_state.next_scene_id = -1;
    
    GLuint map_texture_id = Utility::load_texture("assets/images/cyber_tiles.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELB_DATA, map_texture_id, 1.0f, 4, 1);
    
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
    
    GLuint enemy_texture_id = Utility::load_texture(AI1_FILEPATH);
    
    for(int i = 0; i < ENEMY_COUNT; i++){
        m_game_state.enemies[i] =  Entity(enemy_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, CYCLONE, IDLE);
        m_game_state.enemies[i].set_movement(glm::vec3(0.0f));
        m_game_state.enemies[i].set_lives(5);
    }
    m_game_state.enemies[0].set_position(glm::vec3(1.0f, -4.0f, 0.0f)); //spawn on platforms
    m_game_state.enemies[1].set_position(glm::vec3(10.0f, -1.0f, 0.0f));
    m_game_state.enemies[2].set_position(glm::vec3(10.0f, -4.0f, 0.0f));
    
    /**ORB*/ //only spawn the orb if all of the enemies are defeated
    GLuint orb_texture_id = Utility::load_texture(ORB_FILEPATH);
    m_game_state.orb= new Entity(orb_texture_id, 0.0f, 0.5f, 0.5f, ORB);
    m_game_state.orb->set_position(glm::vec3(18.0f, -1.0f, 0.0f)); //spawn at the end of maze
    m_game_state.orb->set_scale(glm::vec3(0.4f, 0.4f, 0.0f));
    
    /**BGM and SFX*/
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("assets/audio/bgm1.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1); //-1 = loop forever
    Mix_VolumeMusic(20.0f);
    
    m_game_state.jump_sfx = Mix_LoadWAV("assets/audio/jump.wav");
    m_game_state.lose_sfx= Mix_LoadWAV("assets/audio/lose.wav");
}

void LevelB::update(float delta_time)
{
    if(!m_game_state.pause_screen){
        m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT + 1, m_game_state.map, m_game_state.orb);
        
        if(ENEMY_COUNT == m_game_state.player->get_stomp_count()){
            m_game_state.orb->update(delta_time, m_game_state.player, m_game_state.player, 1, m_game_state.map, m_game_state.orb);
        }
        
        if(ENEMY_COUNT == stomped){
            m_game_state.orb->update(delta_time, m_game_state.player, m_game_state.player, 1, m_game_state.map, m_game_state.orb);
        }
        
        for (int i = 0; i < ENEMY_COUNT; i++) {
            m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map, m_game_state.orb);
            if (m_game_state.enemies[i].get_lives() <= 0 && !m_game_state.enemies[i].get_stomped()) {
                stomped++;
                m_game_state.enemies[i].set_stomped(true);
                actual_kill_order.push_back(i);
                //check kill order
                if (actual_kill_order.size() <= expected_kill_order.size() && actual_kill_order.back() != expected_kill_order[actual_kill_order.size() - 1]) {
                    reset_level = true;
                }
            }         
        }
        
        
        if (reset_level) { //reset the level if not killed in order
            reset_level = false;
            stomped = 0;
            actual_kill_order.clear();
            initialise();
        }
        
        m_game_state.lives = m_game_state.player->get_lives();
        if (m_game_state.orb->get_hit_orb()) { //only advance the player when it hits the orb
            m_game_state.player->set_hit_orb(false);
            m_game_state.next_scene_id = 1;
        }
        
        if (m_game_state.player->get_game_status() && !m_game_state.sound_played) {
            Mix_PlayChannel(-1, m_game_state.lose_sfx, 0);
            m_game_state.sound_played = true;
        }
    }
}

void LevelB::render(ShaderProgram *program)
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
        Utility::draw_text(program, Utility::load_texture(FONTSHEET_FILEPATH), "You Lose", 0.5f, 0.005f, glm::vec3(player_pos.x-0.75f, player_pos.y +1.5f, 0.0f)); //lives above the players head
    }else{
        Utility::draw_text(program, Utility::load_texture(FONTSHEET_FILEPATH), "Lives:" + std::to_string(lives), 0.3f, 0.005f, glm::vec3(player_pos.x-0.75f, player_pos.y +1.0f, 0.0f)); //lives above the players head
    }
}
