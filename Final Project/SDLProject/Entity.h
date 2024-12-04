#ifndef ENTITY_H
#define ENTITY_H

#include "Map.h"
#include "glm/glm.hpp"
#include "ShaderProgram.h"
enum EntityType { PLATFORM, PLAYER, ENEMY, LIVES, ORB };
enum AIType     { GUARD, DRONE, SHOOTER, LASER};
enum AIState    { WALKING, IDLE, ATTACKING};
enum Lives{THREE, TWO, ONE, ZERO};
enum Animation {WALK, ATTACK};

enum AnimationDirection { LEFT, RIGHT, UP, DOWN};
enum EnemyAnimationDirection {AIR, GROUND};
class Entity
{
private:
    bool m_is_active = true;
    
    int m_walking[4][4];
    int m_attacking_animation[4][4];
    
    EntityType m_entity_type;
    AIType     m_ai_type;
    AIState    m_ai_state;
    // ————— TRANSFORMATIONS ————— //
    glm::vec3 m_movement;
    glm::vec3 m_position;
    glm::vec3 m_scale;
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;
    glm::vec3 m_start_pos;
    float m_rotation_angle = 0.0f;
    glm::mat4 m_model_matrix;

    float     m_speed,
              m_jumping_power;
    
    bool m_attacking;
    bool m_is_jumping;

    // ————— TEXTURES ————— //
    GLuint    m_texture_id;

    // ————— ANIMATION ————— //
    int m_animation_cols;
    int m_animation_frames,
        m_animation_index,
        m_animation_rows;

    int* m_animation_indices = nullptr;
    float m_animation_time = 0.0f;
    AnimationDirection m_current_direction = DOWN;

    float m_width = 1.0f,
          m_height = 1.0f;
    // ————— COLLISIONS ————— //
    bool m_collided_top    = false;
    bool m_collided_bottom = false;
    bool m_collided_left   = false;
    bool m_collided_right  = false;
    bool is_about_to_fall_left = false;
    bool is_about_to_fall_right = false;
    int m_stomp_count = 0;
    bool game_over = false;
    bool player_wins = false;
    bool m_on_screen = true;
    float m_lives = 15.0f;
    bool m_hit_orb = false;
    
    //kill cool down for player and enemy
    bool m_invincible = false;
    float m_invincible_timer = 0.0f;
    bool m_stomped = false;
    
public:
    // ————— STATIC VARIABLES ————— //
    static constexpr int SECONDS_PER_FRAME = 4;

    // ————— METHODS ————— //
    Entity();
    Entity(GLuint texture_id, float speed, glm::vec3 acceleration, float jump_power, int walking[4][4], int attacking[4][4], float animation_time,
        int animation_frames, int animation_index, int animation_cols,
           int animation_rows, float width, float height, EntityType EntityType);
    Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType); // Simpler constructor
    Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType, int animation_cols,
           int animation_rows,int animation_frames, int animation_index );
    Entity(GLuint texture_id, float speed, float width, float height, EntityType EntityType, AIType AIType, AIState AIState); // AI constructor
    ~Entity();

    void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index);
    bool const check_collision(Entity* other) const;
    
    void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);
    
    // Overloading our methods to check for only the map
    void const check_collision_y(Map *map);
    void const check_collision_x(Map *map);
    
    void update(float delta_time, Entity *player, Entity *collidable_entities, int collidable_entity_count, Map *map, Entity *orb);
    void render(ShaderProgram* program);

    void ai_activate(Entity *player, float delta_time);
    void ai_walk();
    void ai_spin(Entity *player, float delta_time);
    void ai_guard(Entity *player);
    void ai_shooter(Entity* player, float delta_time);
    void pit_detection(Map *map);
    
    void normalise_movement() { m_movement = glm::normalize(m_movement); }

    void face_left() { m_animation_indices = m_walking[LEFT]; m_current_direction = LEFT; set_animation_state(WALK);}
    void face_right() { m_animation_indices = m_walking[RIGHT]; m_current_direction = RIGHT;set_animation_state(WALK);}
    void face_up() { m_animation_indices = m_walking[UP]; m_current_direction = UP;set_animation_state(WALK);}
    void face_down() { m_animation_indices = m_walking[DOWN]; m_current_direction = DOWN;set_animation_state(WALK);}
    
    //direction attacking
    void attack_left()  { m_current_direction = LEFT;  set_animation_state(ATTACK); }
    void attack_right() { m_current_direction = RIGHT; set_animation_state(ATTACK); }
    void attack_up()    { m_current_direction = UP;    set_animation_state(ATTACK); }
    void attack_down()  { m_current_direction = DOWN;  set_animation_state(ATTACK); }

    void move_left() { m_movement.x = -1.0f; face_left(); }
    void move_right() { m_movement.x = 1.0f;  face_right(); }
    void move_up() { m_movement.y = 1.0f;  face_up(); }
    void move_down() { m_movement.y = -1.0f; face_down(); }
    void attacking(bool attack_status){m_attacking = attack_status;}

    // ————— GETTERS ————— //
    EntityType const get_entity_type()    const { return m_entity_type;   };
    AIType     const get_ai_type()        const { return m_ai_type;       };
    AIState    const get_ai_state()       const { return m_ai_state;      };
    glm::vec3 const get_position()     const { return m_position; }
    glm::vec3 const get_start_position()     const { return m_start_pos; }
    glm::vec3 const get_velocity()     const { return m_velocity; }
    glm::vec3 const get_acceleration() const { return m_acceleration; }
    glm::vec3 const get_movement()     const { return m_movement; }
    glm::vec3 const get_scale()        const { return m_scale; }
    GLuint    const get_texture_id()   const { return m_texture_id; }
    float     const get_speed()        const { return m_speed; }
    bool      const get_collided_top() const { return m_collided_top; }
    bool      const get_collided_bottom() const { return m_collided_bottom; }
    bool      const get_collided_right() const { return m_collided_right; }
    bool      const get_collided_left() const { return m_collided_left; }
    bool      const get_hit_orb() const { return m_hit_orb; }
    bool      const get_state() const { return m_is_active; }
    bool      const get_game_status() const { return game_over; }
    bool      const get_stomp_count() const { return m_stomp_count; }
    bool      const get_win_status() const { return player_wins; }
    bool      const get_on_screen() const { return m_on_screen; }
    bool      const get_invincible() const { return m_invincible; }
    float      const get_lives() const { return m_lives; }
    bool     const get_stomped() const { return m_stomped; }
    
    void activate()   { m_is_active = true;  };
    void deactivate() { m_is_active = false; };
    
    // ————— SETTERS ————— //
    void const set_entity_type(EntityType new_entity_type)  { m_entity_type = new_entity_type;};
    void const set_ai_type(AIType new_ai_type){ m_ai_type = new_ai_type;};
    void const set_ai_state(AIState new_state){ m_ai_state = new_state;};
    void const set_position(glm::vec3 new_position) { m_position = new_position; }
    void const update_position_x(float new_amount) { m_position.x += new_amount; }
    void const update_position_y(float new_amount) { m_position.y += new_amount; }
    void const set_start_position(glm::vec3 new_position) { m_start_pos = new_position; }
    void const set_velocity(glm::vec3 new_velocity) { m_velocity = new_velocity; }
    void const set_acceleration(glm::vec3 new_acceleration) { m_acceleration = new_acceleration; }
    void const set_movement(glm::vec3 new_movement) { m_movement = new_movement; }
    void const set_scale(glm::vec3 new_scale) { m_scale = new_scale; }
    void const set_texture_id(GLuint new_texture_id) { m_texture_id = new_texture_id; }
    void const set_speed(float new_speed) { m_speed = new_speed; }
    void const set_animation_cols(int new_cols) { m_animation_cols = new_cols; }
    void const set_animation_rows(int new_rows) { m_animation_rows = new_rows; }
    void const set_animation_frames(int new_frames) { m_animation_frames = new_frames; }
    void const set_animation_index(int new_index) { m_animation_index = new_index; }
    void const set_animation_time(float new_time) { m_animation_time = new_time; }
    void set_animation_state(Animation new_animation);
    void const set_jumping_power(float new_jumping_power) { m_jumping_power = new_jumping_power;}
    void const set_width(float new_width) {m_width = new_width; }
    void const set_height(float new_height) {m_height = new_height; }
    void const inc_stomp_count() {m_stomp_count++; }
    void const set_game_status(bool new_status) {game_over = new_status; }
    void const set_win_status(bool new_status) {player_wins = new_status; }
    void const set_lives(float new_lives){m_lives = new_lives;}
    void const inc_lives(float new_lives){m_lives += new_lives;}
    void const set_hit_orb(bool new_status){m_hit_orb = new_status;}
    void const set_invincible(bool new_status) { m_invincible = new_status; }
    void const set_stomped(bool new_status) {m_stomped = new_status; }
    void dec_lives() {m_lives--;}
    // Setter for m_walking
    void set_walking(int walking[4][4], int attacking[4][4])
    {
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                m_walking[i][j] = walking[i][j];
                m_attacking_animation[i][j] = attacking[i][j];
            }
        }
    }
    
    void set_attacking(int attacking[4][4])
    {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                m_attacking_animation[i][j] = attacking[i][j];
    }
};

#endif // ENTITY_H
