#include <cmath>
#include <iostream>
#include <memory>
#include <SFML/Window.hpp>

struct Boid {
    float x, y;
    float vx, vy;
};

float squared_distance(Boid a, Boid b);

int main() {
    constexpr int N = 4;

    std::unique_ptr<Boid[]> boids(new Boid[N]);

    // FIX ME: all necessary?
    constexpr float TURN_FACTOR = 0.2f;
    constexpr float VISUAL_RANGE = 40;
    constexpr float PROTECTED_RANGE = 8;
    constexpr float CENTERING_FACTOR = 0.0005;
    constexpr float AVOID_FACTOR= 0.05;
    constexpr float MATCHING_FACTOR= 0.05;
    constexpr float MAX_SPEED= 6;
    constexpr float MIN_SPEED= 3;
    constexpr float MAX_BIAS= 0.01;
    constexpr float BIAS_INC=0.00004;
    constexpr float BIAS_VAL=0.001;
    constexpr float TOP_MARGIN = 600;
    constexpr float BOT_MARGIN = 800;
    constexpr float LEFT_MARGIN = 0;
    constexpr float RIGHT_MARGIN = 0;

    float speed = 0;



    sf::Window window(sf::VideoMode({800, 600}), "Boids simulation");
    window.setFramerateLimit(60); // call it once after creating the window

    while (window.isOpen())
    {
        for (int i=0; i<N; i++) {
            float dx =0, dy = 0, sqd=0, close_dx=0, close_dy=0, x_avg=0,
            y_avg=0, xv_avg=0, yv_avg=0, n_neighbours=0;

            for (int j=(i+1); j != i; j = (j+1) % N) {
                dx = boids[i].x - boids[j].x;
                dy = boids[i].y - boids[j].y;

                if (std::fabs(dx)<VISUAL_RANGE && std::fabs(dy)<VISUAL_RANGE) {

                    sqd = squared_distance(boids[i], boids[j]);
                    if (sqd < PROTECTED_RANGE*PROTECTED_RANGE) {
                        close_dx += dx;
                        close_dy += dy;

                    }else {
                        x_avg += dx;
                        y_avg += dy;
                        xv_avg += boids[j].vx;
                        yv_avg += boids[j].vy;
                        n_neighbours++;
                    }

                }
            }
            if (n_neighbours > 0) {
                x_avg /= n_neighbours;
                y_avg /= n_neighbours;
                xv_avg /= n_neighbours;
                yv_avg /= n_neighbours;

                boids[i].vx = boids[i].vx + (x_avg-boids[i].x)*CENTERING_FACTOR + (xv_avg - boids[i].vx)*MATCHING_FACTOR;
                boids[i].vy = boids[i].vy + (y_avg-boids[i].y)*CENTERING_FACTOR + (yv_avg - boids[i].vy)*MATCHING_FACTOR;
            }
            boids[i].vx += boids[i].vx + close_dx*AVOID_FACTOR;
            boids[i].vy += boids[i].vy + close_dy*AVOID_FACTOR;

            if (boids[i].y > TOP_MARGIN)
                boids[i].vy += TURN_FACTOR;
            if (boids[i].y < BOT_MARGIN)
                boids[i].vy -= TURN_FACTOR;
            if (boids[i].x > LEFT_MARGIN)
                boids[i].vx += TURN_FACTOR;
            if (boids[i].x < RIGHT_MARGIN)
                boids[i].vx -= TURN_FACTOR;
            speed = sqrt(pow(boids[i].vx, 2) + pow(boids[i].vy, 2));

            if (speed < MIN_SPEED) {
                boids[i].vx = (boids[i].vx/speed)*MIN_SPEED;
                boids[i].vy = (boids[i].vy/speed)*MIN_SPEED;
            }
            else if (speed > MAX_SPEED) {
                boids[i].vx = (boids[i].vx/speed)*MAX_SPEED;
                boids[i].vy = (boids[i].vy/speed)*MAX_SPEED;
            }

            boids[i].x += boids[i].vx;
            boids[i].y += boids[i].vy;
        }
}

    return 0;
    // TIP See CLion help at <a href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>. Also, you can try interactive lessons for CLion by selecting 'Help | Learn IDE Features' from the main menu.
}
float squared_distance(Boid a, Boid b){
    return static_cast<float>(pow((a.x - b.x), 2) + pow(a.y - b.y, 2));
}