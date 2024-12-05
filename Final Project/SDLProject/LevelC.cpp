#include "LevelC.h"
#include "Utility.h"

#define LEVEL_WIDTH 20
#define LEVEL_HEIGHT 8

constexpr char SPRITESHEET_FILEPATH[] = "assets/images/combined.png",
           ENEMY1_FILEPATH[]       = "assets/images/rat.png",
            ENEMY2_FILEPATH[]       = "assets/images/drone1.png",
            ENEMY3_FILEPATH[]       = "assets/images/robot.png",
            ORB_FILEPATH[]       = "assets/images/orb.png",
            LASER_FILEPATH[]       = "assets/images/laser.png",
            FONTSHEET_FILEPATH[]         = "assets/fonts/font1.png";

//three zones with three separate AI in varying difficulty that the player needs to fight off
//first enemy is guard --> rat
//second enemy is the cyclone --> drone
//third enemy is the shooter --> robot
unsigned int LEVELC_DATA[] = {
    5, 2, 1, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2, 3, 4, 5, 1, 2,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    5, 4, 1, 2, 3, 2, 1, 5, 4, 2, 1, 3, 1, 3, 4, 5, 4, 2, 0, 4,
    4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5,
    3, 0, 2, 3, 1, 2, 3, 4, 1, 2, 3, 4, 4, 2, 1, 3, 5, 0, 0, 1,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 3,
    5, 3, 4, 5, 4, 3, 4, 5, 4, 2, 1, 3, 4, 5, 1, 3, 2, 4, 5, 3,
};
int stomped = 0;
LevelC::~LevelC()
{
    Mix_FreeMusic(m_game_state.bgm);
    Mix_FreeChunk(m_game_state.stomp_sfx);
    Mix_FreeChunk(m_game_state.lose_sfx);
    Mix_FreeChunk(m_game_state.win_sfx);
    delete [] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    delete    m_game_state.laser;
}

void LevelC::initialise()
{
    m_game_state.next_scene_id = -1;
    
    GLuint map_texture_id = Utility::load_texture("assets/images/cyber_tiles.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELC_DATA, map_texture_id, 1.0f, 4, 1);
    
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
    m_game_state.player->set_position(glm::vec3(1.0f, -1.0f, 0.0f));
    m_game_state.player->set_scale(glm::vec3(0.4f, 0.4f, 0.0f));
    m_game_state.player->set_lives(m_game_state.lives);
    
    /**Enemies' stuff */
    m_game_state.enemies = new Entity[ENEMY_COUNT];
    
    GLuint enemy1_texture_id = Utility::load_texture(ENEMY1_FILEPATH);
    GLuint enemy2_texture_id = Utility::load_texture(ENEMY2_FILEPATH);
    GLuint enemy3_texture_id = Utility::load_texture(ENEMY3_FILEPATH);
    
    //rats
    for(int i = 0; i < 7; i++){
        m_game_state.enemies[i] =  Entity(enemy1_texture_id, 4.0f, 1.0f, 1.0f, ENEMY, GUARD, IDLE);
        m_game_state.enemies[i].set_movement(glm::vec3(0.0f));
        m_game_state.enemies[i].set_lives(10.0f);
    }
    
    m_game_state.enemies[0].set_position(glm::vec3(2.0f, -1.25f, 0.0f));
    m_game_state.enemies[1].set_position(glm::vec3(6.0f, -1.25f, 0.0f));
    m_game_state.enemies[2].set_position(glm::vec3(15.0f, -1.25f, 0.0f));
    m_game_state.enemies[3].set_position(glm::vec3(16.0f, -3.25f, 0.0f));
    m_game_state.enemies[4].set_position(glm::vec3(13.0f, -3.25f, 0.0f));
    m_game_state.enemies[5].set_position(glm::vec3(10.0f, -3.25f, 0.0f));
    m_game_state.enemies[6].set_position(glm::vec3(4.0f, -3.25f, 0.0f));
    //drones
    for(int i = 7; i < ENEMY_COUNT-1; i++){
        m_game_state.enemies[i] =  Entity(enemy2_texture_id, 5.0f, 1.0f, 1.0f, ENEMY, DRONE, IDLE);
        m_game_state.enemies[i].set_movement(glm::vec3(0.0f));
        m_game_state.enemies[i].set_lives(12.0f);
        m_game_state.enemies[i].set_scale(glm::vec3(1.0f, 1.0f, 0.0f));
    }
    m_game_state.enemies[7].set_position(glm::vec3(18.0f, -3.0f, 0.0f));
    m_game_state.enemies[8].set_position(glm::vec3(18.0f, -4.0f, 0.0f));
    m_game_state.enemies[9].set_position(glm::vec3(18.0f, -6.0f, 0.0f));
    m_game_state.enemies[10].set_position(glm::vec3(1.0f, -4.0f, 0.0f));
    //robot
    m_game_state.enemies[11] =  Entity(enemy3_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, SHOOTER, ATTACKING);
    m_game_state.enemies[11].set_movement(glm::vec3(0.0f));
    m_game_state.enemies[11].set_lives(15.0f);
    m_game_state.enemies[11].set_scale(glm::vec3(1.0f, 1.0f, 0.0f));
    m_game_state.enemies[11].set_position(glm::vec3(10.0f, -5.25f, 0.0f));
    
    //laser
    GLuint laser_texture_id = Utility::load_texture(LASER_FILEPATH);
    m_game_state.laser =  new Entity(laser_texture_id, 10.0f, 1.0f, 1.0f, LASER);
    m_game_state.laser->set_position(glm::vec3(10.0f, -5.25f, 0.0f));
    m_game_state.laser->set_start_position(glm::vec3(10.0f, -5.25f, 0.0f)); //starts at the robot
    m_game_state.laser->set_scale(glm::vec3(0.2f, 0.2f, 0.0f));
    
    
    /**ORB*/ //only spawn the orb if all of the enemies are defeated
    GLuint orb_texture_id = Utility::load_texture(ORB_FILEPATH);
    m_game_state.orb= new Entity(orb_texture_id, 0.0f, 0.5f, 0.5f, ORB);
    m_game_state.orb->set_position(glm::vec3(2.0f, -1.0f, 0.0f)); //spawn at the start of the maze
    m_game_state.orb->set_scale(glm::vec3(0.2f, 0.2f, 0.0f));
    
    /**BGM and SFX*/
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("assets/audio/bgmC.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1); //-1 = loop forever
    Mix_VolumeMusic(20.0f);
    
    m_game_state.stomp_sfx = Mix_LoadWAV("assets/audio/enemy.wav");
    m_game_state.lose_sfx= Mix_LoadWAV("assets/audio/lose.wav");
    m_game_state.win_sfx= Mix_LoadWAV("assets/audio/win.wav");
}

void LevelC::update(float delta_time)
{//lose if the player runs out of lives before hitting the orb
    if(!m_game_state.pause_screen){

        m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT + 1, m_game_state.map, m_game_state.orb, m_game_state.laser);
        
       m_game_state.laser->update(delta_time, m_game_state.player, m_game_state.player, 1, m_game_state.map, m_game_state.orb, m_game_state.laser);
        
        if(ENEMY_COUNT == m_game_state.player->get_stomp_count()){
            m_game_state.orb->update(delta_time, m_game_state.player, m_game_state.player, 1, m_game_state.map, m_game_state.orb, m_game_state.laser);
        }
        
        if(ENEMY_COUNT == stomped){
            m_game_state.orb->update(delta_time, m_game_state.player, m_game_state.player, 1, m_game_state.map, m_game_state.orb, m_game_state.laser);
        }
        
        for (int i = 0; i < ENEMY_COUNT; i++){
            m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map, m_game_state.orb, m_game_state.laser);
            if(m_game_state.enemies[i].get_lives()<=0 && !m_game_state.enemies[i].get_stomped()){
                stomped++;
                m_game_state.player->inc_lives(0.5f);
                Mix_PlayChannel(-1, m_game_state.stomp_sfx, 0);
                m_game_state.enemies[i].set_stomped(true);
            }
        }
        m_game_state.lives = m_game_state.player->get_lives();
        
        if (!m_game_state.sound_played) {
            if(m_game_state.orb->get_hit_orb()){
                Mix_PlayChannel(-1, m_game_state.win_sfx, 0);
                m_game_state.sound_played = true;
                m_game_state.player->set_win_status(true);
                m_game_state.player->set_game_status(true);
            }else if(!m_game_state.orb->get_hit_orb() && m_game_state.player->get_lives() <= 0){
                Mix_PlayChannel(-1, m_game_state.lose_sfx, 0);
                m_game_state.sound_played = true;
            }
        }
    }
}

void LevelC::render(ShaderProgram *program)
{
    m_game_state.map->render(program);
    if(m_game_state.enemies[11].get_ai_state() != DEAD){
        m_game_state.laser->render(program);
    }
    m_game_state.player->render(program);
    for (int i = 0; i < ENEMY_COUNT; i++)    m_game_state.enemies[i].render(program);
  
    //change the color to show the player is invincible/protected
    if(m_game_state.player->get_invincible()){
        program->set_attack(1);
    }else{//default clor
        program->set_attack(0);
    }
    
    //only render the orb when all enemies are killed
    if(ENEMY_COUNT == stomped){
        m_game_state.orb->render(program);
    }
    int lives = m_game_state.player->get_lives();
    glm::vec3 player_pos = m_game_state.player->get_position();
    
    //spotlight logic
    program->set_spotlight(1);
    program->set_light_position_matrix(player_pos);
    
    //round to tenths
    float roundedLives = std::round(lives * 10.0f) / 10.0f;
    std::string livesStr = std::to_string(roundedLives);
    livesStr.erase(livesStr.find('.') + 2);
    
    if(m_game_state.player->get_game_status()){ //true = over
        if(m_game_state.player->get_win_status()){
            program->set_spotlight(0);
            Utility::draw_text(program, Utility::load_texture(FONTSHEET_FILEPATH), "You Win", 0.5f, 0.005f, glm::vec3(player_pos.x, player_pos.y, 0.0f));
        }else{
            program->set_spotlight(0);
            Utility::draw_text(program, Utility::load_texture(FONTSHEET_FILEPATH), "You Lose", 0.5f, 0.005f, glm::vec3(player_pos.x, player_pos.y , 0.0f));
        }
    }else{
        Utility::draw_text(program, Utility::load_texture(FONTSHEET_FILEPATH), "Health:" + livesStr, 0.3f, 0.005f, glm::vec3(player_pos.x-0.75f, player_pos.y +0.5f, 0.0f)); //lives above the players head
    }
}

