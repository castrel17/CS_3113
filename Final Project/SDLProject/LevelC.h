#include "Scene.h"

class LevelC : public Scene {
public:
    //3 rats, 4 drones, 1 robots
    int ENEMY_COUNT = 8;
    int lives = 3;
    ~LevelC();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
