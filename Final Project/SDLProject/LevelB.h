#include "Scene.h"

class LevelB : public Scene {
public:
    int ENEMY_COUNT = 3;
    int lives = 3;
    bool reset_level = false;
    int stomped = 0;
    ~LevelB();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
