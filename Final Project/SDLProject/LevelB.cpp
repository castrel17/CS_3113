#include "LevelB.h"
#include "Utility.h"

#define LEVEL_WIDTH 20
#define LEVEL_HEIGHT 8

constexpr char SPRITESHEET_FILEPATH[] = "assets/images/combined.png",
           ENEMY1_FILEPATH[]       = "assets/images/drone1.png",
            ENEMY2_FILEPATH[]       = "assets/images/drone2.png",
            ENEMY3_FILEPATH[]       = "assets/images/drone3.png",
            ORB_FILEPATH[]       = "assets/images/orb.png",
            LASER_FILEPATH[]       = "assets/images/laser.png",
            FONTSHEET_FILEPATH[]         = "assets/fonts/font1.png";


//orb spawns in the center of the city when all the enemies are killed in a certain order
//the level is called name of the game which is the hint, because the enemies will be labeled E, T, C and need to be hit in that order
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
    Mix_FreeChunk(m_game_state.stomp_sfx);
    Mix_FreeChunk(m_game_state.lose_sfx);
    Mix_FreeChunk(m_game_state.next_level_sfx);
    delete [] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    delete    m_game_state.laser;
}

void LevelB::initialise()
{
    m_game_state.next_scene_id = -1;
    
    GLuint map_texture_id = Utility::load_texture("assets/images/cyber_tiles.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELB_DATA, map_texture_id, 1.0f, 4, 1);
    
    int player_walking_animation[4][4] =
    {
        {24,25,26,27},  //left
        {16,17,18,19}, //right
        {8,9,10,11}, //up
        {0,1,2,3} ,//down
    };
    int player_attacking_animation[4][4] =
    {
        {28,29,30,31},  //left
        {20,21,22,23}, //right
        {12,13,14,15}, //up
        {4,5,6,7} ,//down
    };
    glm::vec3 acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
    
    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
    
    m_game_state.player = new Entity(
           player_texture_id,         // texture id
           2.0f,                      // speed
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
    m_game_state.player->set_position(glm::vec3(1.0f, -1.0f, 0.0f));
    m_game_state.player->set_scale(glm::vec3(0.4f, 0.4f, 0.0f));
    m_game_state.player->set_lives(m_game_state.lives);
    
    /**Enemies' stuff */
    m_game_state.enemies = new Entity[ENEMY_COUNT];
    
    GLuint enemy1_texture_id = Utility::load_texture(ENEMY1_FILEPATH); //E
    GLuint enemy2_texture_id = Utility::load_texture(ENEMY2_FILEPATH); //T
    GLuint enemy3_texture_id = Utility::load_texture(ENEMY3_FILEPATH); //C
    m_game_state.enemies[0] =  Entity(enemy1_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, DRONE, IDLE);
    m_game_state.enemies[1] =  Entity(enemy2_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, DRONE, IDLE);
    m_game_state.enemies[2] =  Entity(enemy3_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, DRONE, IDLE);
    
    for(int i = 0; i < ENEMY_COUNT; i++){
        m_game_state.enemies[i].set_movement(glm::vec3(0.0f));
        m_game_state.enemies[i].set_lives(6.0f);
        m_game_state.enemies[i].set_scale(glm::vec3(1.0f, 1.0f, 0.0f));
    }
    
    m_game_state.enemies[2].set_position(glm::vec3(1.5f, -3.5f, 0.0f));
    m_game_state.enemies[1].set_position(glm::vec3(15.5f, -1.0f, 0.0f));
    m_game_state.enemies[0].set_position(glm::vec3(8.5f, -5.5f, 0.0f));
    
    /**ORB*/ //only spawn the orb if all of the enemies are defeated
    GLuint orb_texture_id = Utility::load_texture(ORB_FILEPATH);
    m_game_state.orb= new Entity(orb_texture_id, 0.0f, 0.5f, 0.5f, ORB);
    m_game_state.orb->set_position(glm::vec3(9.0f, -3.0f, 0.0f)); //spawn at the end of maze
    m_game_state.orb->set_scale(glm::vec3(0.4f, 0.4f, 0.0f));
    
    //laser
    GLuint laser_texture_id = Utility::load_texture(LASER_FILEPATH);
    m_game_state.laser =  new Entity(laser_texture_id, 1.0f, 1.0f, 1.0f, LASER);
    m_game_state.laser->set_position(glm::vec3(-10.0f, -1.0f, 0.0f)); //spawn off screen
    
    /**BGM and SFX*/
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("assets/audio/bgm1.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1); //-1 = loop forever
    Mix_VolumeMusic(20.0f);
    
    m_game_state.stomp_sfx = Mix_LoadWAV("assets/audio/enemy.wav");
    m_game_state.lose_sfx= Mix_LoadWAV("assets/audio/lose.wav");
    m_game_state.next_level_sfx= Mix_LoadWAV("assets/audio/next_level.mp3");
}

void LevelB::update(float delta_time)
{
    if(!m_game_state.pause_screen){
        m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT + 1, m_game_state.map, m_game_state.orb, m_game_state.laser);
        
        if(ENEMY_COUNT == stomped){
            m_game_state.orb->update(delta_time, m_game_state.player, m_game_state.player, 1, m_game_state.map, m_game_state.orb, m_game_state.laser);
        }
        
        for (int i = 0; i < ENEMY_COUNT; i++) {
            m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map, m_game_state.orb, m_game_state.laser);
            if (m_game_state.enemies[i].get_lives() <= 0 && !m_game_state.enemies[i].get_stomped()) {
                stomped++;
                m_game_state.player->inc_lives(0.5f);
                Mix_PlayChannel(-1, m_game_state.stomp_sfx, 0);
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
            Mix_PlayChannel(-1, m_game_state.next_level_sfx, 0);
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
    for (int i = 0; i < ENEMY_COUNT; i++)    m_game_state.enemies[i].render(program);
    
    //only render the orb when all enemies are killed
    if(ENEMY_COUNT == stomped){
        m_game_state.orb->render(program);
    }
    
    float lives = m_game_state.player->get_lives();
    glm::vec3 player_pos = m_game_state.player->get_position();
    
    //change the color to show the player is invincible/protected
    if(m_game_state.player->get_invincible()){
        program->set_attack(1);
    }else{//default clor
        program->set_attack(0);
    }
    //spotlight logic
    program->set_spotlight(1);
    program->set_light_position_matrix(player_pos);
    
    //round to tenths
    float roundedLives = std::round(lives * 10.0f) / 10.0f;
    std::string livesStr = std::to_string(roundedLives);
    livesStr.erase(livesStr.find('.') + 2);
    
    Utility::draw_text(program, Utility::load_texture(FONTSHEET_FILEPATH), "It's in the name", 0.1f, 0.005f, glm::vec3(17.0f, -6.5f, 0.0f));//hint
    if(m_game_state.player->get_game_status()){ //true = over
        program->set_spotlight(0);
        Utility::draw_text(program, Utility::load_texture(FONTSHEET_FILEPATH), "You Lose", 0.5f, 0.005f, glm::vec3(player_pos.x, player_pos.y, 0.0f));
    }else{
        Utility::draw_text(program, Utility::load_texture(FONTSHEET_FILEPATH), "Health:" + livesStr, 0.3f, 0.005f, glm::vec3(player_pos.x-0.75f, player_pos.y +0.5f, 0.0f)); //lives above the players head
    }
}
