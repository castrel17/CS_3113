
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"

void Entity::ai_activate(Entity *player,float delta_time, Entity *laser)
{
    switch (m_ai_type)
    {
        case GUARD:
            ai_guard(player);
            break;  
        case DRONE:
            ai_spin(player, delta_time);
            break;
        case SHOOTER:
            ai_shooter(player, delta_time, laser);
            break;
        default:
            break;
    }
}

void Entity::ai_walk()
{
    m_movement = glm::vec3(1.0f, 0.0f, 0.0f);
}

void Entity::ai_spin(Entity *player, float delta_time)
{
    m_rotation_angle += 45.0f * delta_time; //always spinning
    switch (m_ai_state) {
        case IDLE:
            if (glm::distance(m_position, player->get_position()) < 3.0f) {
                m_ai_state = WALKING;
            }
            break;
        case ATTACKING:
            break;
        case WALKING:
            {
                glm::vec3 direction_away = m_position - player->get_position();
                float distance = glm::length(direction_away);
                if (distance > 0.0f) {
                    direction_away = glm::normalize(direction_away);
                    m_movement = direction_away * 0.75f;
                }
            }
            break;
        default:
            break;
    }
}

void Entity::ai_guard(Entity *player)
{
    switch (m_ai_state) {
        case IDLE:
            //start walking when player is near
            if(player->get_state()){
                if (glm::distance(m_position, player->get_position()) < 1.5f) {
                    m_ai_state = WALKING;
                } else {
                    m_movement = glm::vec3(0.0f, 0.0f, 0.0f);
                }
            }
            break;
        case WALKING:
            if (m_position.x > player->get_position().x) { //move left
                if(!is_about_to_fall_left){
                    m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
                }else{
                    m_movement = glm::vec3(0.0f, 0.0f, 0.0f);
                }
            } else { //move right
                if(!is_about_to_fall_right){
                    m_movement = glm::vec3(1.0f, 0.0f, 0.0f);
                }else{
                    m_movement = glm::vec3(0.0f, 0.0f, 0.0f);
                }
            }
        
            break;
            
        case ATTACKING:
            break;
            
        default:
            break;
    }
}

void Entity::ai_shooter(Entity* player, float delta_time, Entity* laser) {
    switch (m_ai_state) {
            
        case IDLE:
            break;
        case WALKING:
            break;
            
        case ATTACKING:
            if(player->get_position().x > m_position.x){
                laser->set_acceleration(glm::vec3(150.0f, 0.0f, 0.0f));
            }else{
                laser->set_acceleration(glm::vec3(-150.0f, 0.0f, 0.0f));
            }
            
            if( laser->get_collided_left() || laser->get_collided_right() ){//reset the laser position
                laser->set_position(laser->get_start_position());
            }
            break;
        case DEAD:
            break;
        default:
            break;
    }
}

// Default constructor
Entity::Entity()
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(0.0f), m_animation_cols(0), m_animation_frames(0), m_animation_index(0),
    m_animation_rows(0), m_animation_indices(nullptr), m_animation_time(0.0f),
    m_texture_id(0), m_velocity(0.0f), m_acceleration(0.0f), m_width(0.0f), m_height(0.0f)
{
    // Initialize m_walking with zeros or any default value
    for (int i = 0; i < SECONDS_PER_FRAME; ++i)
        for (int j = 0; j < SECONDS_PER_FRAME; ++j) m_walking[i][j] = 0;
}

// Parameterized constructor
Entity::Entity(GLuint texture_id, float speed, glm::vec3 acceleration, float jump_power, int walking[4][4], int attacking[4][4], float animation_time,
    int animation_frames, int animation_index, int animation_cols,
    int animation_rows, float width, float height, EntityType EntityType)
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(speed),m_acceleration(acceleration), m_jumping_power(jump_power), m_animation_cols(animation_cols),
    m_animation_frames(animation_frames), m_animation_index(animation_index),
    m_animation_rows(animation_rows), m_animation_indices(nullptr),
    m_animation_time(animation_time), m_texture_id(texture_id), m_velocity(0.0f),
    m_width(width), m_height(height), m_entity_type(EntityType)
{
    face_right();
    set_walking(walking, attacking);
}

// Simpler constructor for partial initialization
Entity::Entity(GLuint texture_id, float speed,  float width, float height, EntityType EntityType)
    : m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
    m_speed(speed), m_animation_cols(0), m_animation_frames(0), m_animation_index(0),
    m_animation_rows(0), m_animation_indices(nullptr), m_animation_time(0.0f),
    m_texture_id(texture_id), m_velocity(0.0f), m_acceleration(0.0f), m_width(width), m_height(height),m_entity_type(EntityType)
{
    // Initialize m_walking with zeros or any default value
    for (int i = 0; i < SECONDS_PER_FRAME; ++i)
        for (int j = 0; j < SECONDS_PER_FRAME; ++j) m_walking[i][j] = 0;
}

Entity::Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType, int animation_cols,
               int animation_rows, int animation_frames, int animation_index )
: m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
m_speed(speed), m_animation_cols(animation_cols), m_animation_frames(animation_frames), m_animation_index(animation_index),
m_animation_rows(animation_rows), m_animation_indices(nullptr), m_animation_time(0.0f),
m_texture_id(texture_id), m_velocity(0.0f), m_acceleration(0.0f), m_width(width), m_height(height),m_entity_type(EntityType)
{
    // Initialize m_walking with zeros or any default value
    for (int i = 0; i < SECONDS_PER_FRAME; ++i)
        for (int j = 0; j < SECONDS_PER_FRAME; ++j) m_walking[i][j] = 0;
}
Entity::Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType, AIType AIType, AIState AIState): m_position(0.0f), m_movement(0.0f), m_scale(1.0f, 1.0f, 0.0f), m_model_matrix(1.0f),
m_speed(speed), m_animation_cols(0), m_animation_frames(0), m_animation_index(0),
m_animation_rows(0), m_animation_indices(nullptr), m_animation_time(0.0f),
m_texture_id(texture_id), m_velocity(0.0f), m_acceleration(0.0f), m_width(width), m_height(height),m_entity_type(EntityType), m_ai_type(AIType), m_ai_state(AIState)
{
// Initialize m_walking with zeros or any default value
for (int i = 0; i < SECONDS_PER_FRAME; ++i)
    for (int j = 0; j < SECONDS_PER_FRAME; ++j) m_walking[i][j] = 0;
}

Entity::~Entity() { }

void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index)
{
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float)(index % m_animation_cols) / (float)m_animation_cols;
    float v_coord = (float)(index / m_animation_cols) / (float)m_animation_rows;

    // Step 2: Calculate its UV size
    float width = 1.0f / (float)m_animation_cols;
    float height = 1.0f / (float)m_animation_rows;

    // Step 3: Just as we have done before, match the texture coordinates to the vertices
    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
        u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };

    float vertices[] =
    {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };

    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->get_position_attribute());

    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

void Entity::set_animation_state(Animation new_animation) {
    if (new_animation == ATTACK) {
        m_attacking = true;
        m_animation_indices = m_attacking_animation[m_current_direction];
        m_animation_index = 0;
    } else {
        m_attacking = false;
        m_animation_indices = m_walking[m_current_direction];
    }
}

bool const Entity::check_collision(Entity* other) const
{
    float x_distance = fabs(m_position.x - other->m_position.x) - ((m_width + other->m_width) / 2.0f);
    float y_distance = fabs(m_position.y - other->m_position.y) - ((m_height + other->m_height) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}

void const Entity::check_collision_y(Entity *collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity *collidable_entity = &collidable_entities[i];
        if (check_collision(collidable_entity))
        {
            float y_distance = fabs(m_position.y - collidable_entity->m_position.y);
            float y_overlap = fabs(y_distance - (m_height / 2.0f) - (collidable_entity->m_height / 2.0f));
            if (m_velocity.y > 0)
            {
                m_position.y   -= y_overlap;
                m_velocity.y    = 0;

                // Collision!
                m_collided_top  = true;
                
                //enemy hit player in the head
                if(m_entity_type == PLAYER &&(collidable_entity->get_entity_type() == ENEMY || collidable_entity->get_entity_type() == LASER)){
                    if(m_attacking && collidable_entity->get_entity_type() != LASER){ //can't hit back laser
                        float knockbackDirection = (m_position.x > collidable_entity->m_position.x) ? -1.0f : 1.0f;
                        collidable_entity->update_position_x(knockbackDirection*0.5f);
                        m_invincible = true;
                        m_invincible_timer = 0.5f;
                        collidable_entity->dec_lives();
                    }else if(!m_invincible){
                        dec_lives();
                        if(get_lives() > 0){//knockback based on direction of enemy
                            float knockbackDirection = (m_position.x > collidable_entity->m_position.x) ? 1.0f : -1.0f;
                            m_position.x += knockbackDirection*0.5f;
                            m_invincible = true;
                            m_invincible_timer = 0.5f;
                        }else{
                            game_over = true;
                        }
                    }
                }else if(m_entity_type == ORB &&(collidable_entity->get_entity_type() == PLAYER)){
                    m_hit_orb = true;
                }
                else if(m_entity_type == LASER &&(collidable_entity->get_entity_type() == PLAYER)){
                    collidable_entity->dec_lives();
                }
            } else if (m_velocity.y < -0.09)
            {
                m_position.y      += y_overlap;
                m_velocity.y       = 0;

                // Collision!
                m_collided_bottom  = true;
                
                //if the player is attacking then the enemy dies, if not then the player loses a life
                if(m_entity_type == PLAYER && (collidable_entity->get_entity_type() == ENEMY || collidable_entity->get_entity_type() == LASER)){
                    if(m_attacking && collidable_entity->get_entity_type() != LASER){ //can't hit back laser)
                        float knockbackDirection = (m_position.x > collidable_entity->m_position.x) ? -1.0f : 1.0f;
                        collidable_entity->update_position_x(knockbackDirection*0.5f);
                        m_invincible = true;
                        m_invincible_timer = 0.5f;
                        collidable_entity->dec_lives();
                        m_attacking = false;
                    }else if(!m_invincible){
                        dec_lives();
                        if(get_lives() > 0){ //knockback based on direction of enemy
                            float knockbackDirection = (m_position.x > collidable_entity->m_position.x) ? 1.0f : -1.0f;
                            m_position.x += knockbackDirection*0.5f;
                            m_invincible = true;
                            m_invincible_timer = 0.5f;
                        }else{
                            game_over = true;
                        }
                    }
                }else if(m_entity_type == ORB &&(collidable_entity->get_entity_type() == PLAYER)){
                    m_hit_orb = true;
                }else if(m_entity_type == LASER &&(collidable_entity->get_entity_type() == PLAYER)){
                    collidable_entity->dec_lives();
                }
            }
        }
    }
}

void const Entity::check_collision_x(Entity *collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity *collidable_entity = &collidable_entities[i];
        
        if (check_collision(collidable_entity))
        {
            float x_distance = fabs(m_position.x - collidable_entity->m_position.x);
            float x_overlap = fabs(x_distance - (m_width / 2.0f) - (collidable_entity->m_width / 2.0f));
            if (m_velocity.x >= 0)
            {
                m_position.x     -= x_overlap;
                m_velocity.x      = 0;

                // Collision!
                m_collided_right  = true;
            } else if (m_velocity.x <= 0)
            {
                m_position.x    += x_overlap;
                m_velocity.x     = 0;
 
                // Collision!
                m_collided_left  = true;
            }
            
            //if the player is attacking then the enemy dies, if not then the player loses a life
            if(m_collided_left || m_collided_right){
                if(m_entity_type == PLAYER &&(collidable_entity->get_entity_type() == ENEMY || collidable_entity->get_entity_type() == LASER)){
                    if(m_attacking && collidable_entity->get_entity_type() != LASER){ //can't hit back laser)
                        float knockbackDirection = (m_position.x > collidable_entity->m_position.x) ? -1.0f : 1.0f;
                        collidable_entity->update_position_x(knockbackDirection*0.5f);
                        m_invincible = true;
                        m_invincible_timer = 0.5f;
                        collidable_entity->dec_lives();
                        m_attacking = false;

                    }else if(!m_invincible){
                        dec_lives();
                        if(get_lives() > 0){//knockback based on direction of enemy
                            float knockbackDirection = (m_position.x > collidable_entity->m_position.x) ? 1.0f : -1.0f;
                            m_position.x += knockbackDirection*0.5f;
                            m_invincible = true;
                            m_invincible_timer = 0.5f;
                        }else{
                            game_over = true;
                        }
                    }
                }else if(m_entity_type == ORB &&(collidable_entity->get_entity_type() == PLAYER)){
                    m_hit_orb = true;
                }else if(m_entity_type == LASER &&(collidable_entity->get_entity_type() == PLAYER)){
                    collidable_entity->dec_lives();
                }
                
            }
        }
    }
}

void Entity::pit_detection(Map *map)
{
    float pit_detection_distance = 0.08f;
    float penetration_x = 0;
    float penetration_y = 0;

    //point-to-box left
    glm::vec3 left_check = glm::vec3(m_position.x - ((m_width / 2) + pit_detection_distance), m_position.y - (m_height / 2), m_position.z);
    is_about_to_fall_left = !map->is_solid(left_check, &penetration_x, &penetration_y);
    
    //point-to-box right
    glm::vec3 right_check = glm::vec3(m_position.x + ((m_width / 2) + pit_detection_distance), m_position.y - (m_height / 2), m_position.z);
    is_about_to_fall_right = !map->is_solid(right_check, &penetration_x, &penetration_y);
}

void const Entity::check_collision_y(Map *map)
{
    // Probes for tiles above
    glm::vec3 top = glm::vec3(m_position.x, m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_left = glm::vec3(m_position.x - (m_width / 2), m_position.y + (m_height / 2), m_position.z);
    glm::vec3 top_right = glm::vec3(m_position.x + (m_width / 2), m_position.y + (m_height / 2), m_position.z);
    
    // Probes for tiles below
    glm::vec3 bottom = glm::vec3(m_position.x, m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_left = glm::vec3(m_position.x - (m_width / 2), m_position.y - (m_height / 2), m_position.z);
    glm::vec3 bottom_right = glm::vec3(m_position.x + (m_width / 2), m_position.y - (m_height / 2), m_position.z);
    
    float penetration_x = 0;
    float penetration_y = 0;
    
    // If the map is solid, check the top three points
    if (map->is_solid(top, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    else if (map->is_solid(top_left, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    else if (map->is_solid(top_right, &penetration_x, &penetration_y) && m_velocity.y > 0)
    {
        m_position.y -= penetration_y;
        m_velocity.y = 0;
        m_collided_top = true;
    }
    
    // And the bottom three points
    if (map->is_solid(bottom, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
    }
    else if (map->is_solid(bottom_left, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
            m_position.y += penetration_y;
            m_velocity.y = 0;
            m_collided_bottom = true;
    }
    else if (map->is_solid(bottom_right, &penetration_x, &penetration_y) && m_velocity.y < 0)
    {
        m_position.y += penetration_y;
        m_velocity.y = 0;
        m_collided_bottom = true;
    }
}

void const Entity::check_collision_x(Map *map)
{
    // Probes for tiles; the x-checking is much simpler
    glm::vec3 left  = glm::vec3(m_position.x - (m_width / 2), m_position.y, m_position.z);
    glm::vec3 right = glm::vec3(m_position.x + (m_width / 2), m_position.y, m_position.z);
    
    float penetration_x = 0;
    float penetration_y = 0;
    
    if (map->is_solid(left, &penetration_x, &penetration_y) && m_velocity.x < 0)
    {
        m_position.x += penetration_x;
        m_velocity.x = 0;
        m_collided_left = true;
        
    
    }
    if (map->is_solid(right, &penetration_x, &penetration_y) && m_velocity.x > 0)
    {
        m_position.x -= penetration_x;
        m_velocity.x = 0;
        m_collided_right = true;
    }
}


void Entity::update(float delta_time, Entity *player, Entity *collidable_entities, int collidable_entity_count, Map *map, Entity *orb, Entity *laser)
{
    if(player->get_state()){
        glm::vec3 rotation_matrix = glm::vec3(0.0f, 0.0f, 1.0f);
        if (!m_is_active) return;
     
        m_collided_top    = false;
        m_collided_bottom = false;
        m_collided_left   = false;
        m_collided_right  = false;
        
        
        if (m_entity_type == ENEMY) {
            ai_activate(player, delta_time, laser);
            if(m_ai_type == GUARD){
                pit_detection(map);
            }
            if(m_lives < 0){ //move off screen and deactivate once dead
                set_position(glm::vec3(-100.0f, 0.0f, 0.0f));
                m_ai_state = DEAD;
                deactivate();
                if(m_ai_type == SHOOTER){
                    laser->deactivate();
                }
            }
        }
        
        
        m_velocity.x = m_movement.x * m_speed;
        m_velocity.y = m_movement.y * m_speed;
        m_velocity += m_acceleration * delta_time;

        m_position.y += m_velocity.y * delta_time;
        check_collision_y(collidable_entities, collidable_entity_count);
        check_collision_y(map);
        
        m_position.x += m_velocity.x * delta_time;
        check_collision_x(collidable_entities, collidable_entity_count);
        check_collision_x(map);
        
        //timer for invincibility after getting hit by enemy
        if(m_invincible){
            m_invincible_timer -= delta_time;
            if(m_invincible_timer <= 0){
                m_invincible = false;
            }
        }
        //win lose stuff
        if(m_entity_type == PLAYER){
            if (m_attacking) { //attacking animation for the player
                    m_animation_indices = m_attacking_animation[m_current_direction];
                    m_animation_time += delta_time;

                    float frames_per_second = 1.0f / SECONDS_PER_FRAME;
                    if (m_animation_time >= frames_per_second) {
                        m_animation_time = 0.0f;
                        m_animation_index++;

                        if (m_animation_index >= 4) {
                            m_animation_index = 0;
                            m_attacking = false;
                            m_animation_indices = m_walking[m_current_direction];
                        }
                    }
                } else { //walking
                    if (glm::length(m_movement) > 0.0f) {
                        m_animation_time += delta_time;

                        float frames_per_second = 1.0f / SECONDS_PER_FRAME;
                        if (m_animation_time >= frames_per_second) {
                            m_animation_time = 0.0f;
                            m_animation_index = (m_animation_index + 1) % 4;
                        }
                    }
                }
            //player wins if they are alive and killed all enemies
            if(get_lives() <= 0){
                game_over = true;
                deactivate();
            }
            if(get_win_status()){
                game_over = true;
                deactivate();
            }
        }
        
        if(m_entity_type == ORB){ //orb pulses and rotates
            constexpr float BASE_SCALE = 1.0f,
            MAX_AMPLITUDE = 0.05f;
            m_rotation_angle += 90.0f * delta_time;
            m_model_matrix = glm::translate(m_model_matrix, m_position);
            m_scale.x = BASE_SCALE + MAX_AMPLITUDE * glm::sin(delta_time);
            m_scale.y = BASE_SCALE + MAX_AMPLITUDE * glm::sin(delta_time);
            rotation_matrix = glm::vec3(1.0f, 1.0f, 1.0f);
        }
        //restrict position to stay in bounds
        if(m_is_active){
            if (m_position.x < 0.7) m_position.x = 0.7;
            if (m_position.x > 18.29) m_position.x = 18.29;
            if (m_position.y > -0.7) m_position.y = -0.7;
            if (m_position.y < -6.32) m_position.y = -6.3;
        }
        m_model_matrix = glm::mat4(1.0f);
        m_model_matrix = glm::translate(m_model_matrix, m_position);
        m_model_matrix = glm::rotate(m_model_matrix, glm::radians(m_rotation_angle), rotation_matrix);
        m_model_matrix = glm::scale(m_model_matrix, m_scale);
    }
}


void Entity::render(ShaderProgram* program)
{
    if(m_is_active){
        program->set_model_matrix(m_model_matrix);

        if (m_animation_indices != NULL)
        {
            draw_sprite_from_texture_atlas(program, m_texture_id, m_animation_indices[m_animation_index]);
            return;
        }

        float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
        float tex_coords[] = { 0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };

        glBindTexture(GL_TEXTURE_2D, m_texture_id);

        glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program->get_position_attribute());
        glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, tex_coords);
        glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(program->get_position_attribute());
        glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
        
    }
}
