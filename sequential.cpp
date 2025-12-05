#include "sequential_helper.h"


#include <cmath>
#include <iostream>
#include <memory>
#include <SFML/Graphics.hpp>
#include <bits/stdc++.h>

/**
 * This is the sequential version of boids simulation with graphics.
 * The code has a pure sequential nature, so is not optimized and not designed for
 * A parallel execution.
 * **/


int main(int argc, char* argv[]) {

    Config cfg;
    cfg.parse(argc, argv);
    const int N = cfg.N;
    const int FRAMES = cfg.frames;


    std::vector<Boid> boids(N);

    //Constants definition
    constexpr float TURN_FACTOR = 0.2;
    constexpr float VISUAL_RANGE = 40;
    constexpr float PROTECTED_RANGE = 8;
    constexpr float CENTERING_FACTOR = 0.0005;
    constexpr float AVOID_FACTOR= 0.05;
    constexpr float MATCHING_FACTOR= 0.05;
    constexpr float MAX_SPEED= 6;
    constexpr float MIN_SPEED= 3;
    constexpr int TOP_MARGIN = 600;
    constexpr float BOT_MARGIN = 0;
    constexpr float LEFT_MARGIN = 0;
    constexpr int RIGHT_MARGIN = 800;
    constexpr float MARGIN = 80;

    float speed = 0;

    int iterations = 0;

    std::chrono::milliseconds total_duration = std::chrono::milliseconds::zero();

    //boids initialization
    for (int i=0; i < N; i++) {
        boids[i].x = random_float(LEFT_MARGIN+MARGIN, RIGHT_MARGIN-MARGIN);
        boids[i].y = random_float(BOT_MARGIN+MARGIN, TOP_MARGIN-MARGIN);

        boids[i].vx = random_float(-MAX_SPEED, MAX_SPEED);
        boids[i].vy = random_float(-MAX_SPEED, MAX_SPEED);

        boids[i].shape = std::make_unique<sf::CircleShape>(3.f, 3);
    }

    //Graphical window creation
    const int X_SIZE = RIGHT_MARGIN + MARGIN;
    const int Y_SIZE =  TOP_MARGIN + MARGIN;

    sf::RenderWindow window(sf::VideoMode({X_SIZE, Y_SIZE}), "Boids simulation");
    window.setFramerateLimit(60); // call it once after creating the window

    while (window.isOpen() && iterations < FRAMES) {
        window.clear(sf::Color::Black);
        while (const std::optional event = window.pollEvent())
        {

            if (event->is<sf::Event::Closed>())
                window.close();
        }

        for (int i=0; i<N; i++)
            print_boid(boids[i], window);

        // without counting the graphic, pure boids performance
        const auto start = std::chrono::high_resolution_clock::now();
        //To scan every boid
        for (int i=0; i<N; i++) {

            //Variables definition
            float dx =0, dy = 0, sqd=0, close_dx=0, close_dy=0, x_avg=0,
            y_avg=0, xv_avg=0, yv_avg=0, n_neighbours=0;

            //To compare every boid with everyone else
            for (int j = 0; j<N; j++){
                if (j==i)
                    continue;

                dx = boids[i].x - boids[j].x;
                dy = boids[i].y - boids[j].y;

                if (std::fabs(dx)<VISUAL_RANGE && std::fabs(dy)<VISUAL_RANGE) {

                    sqd = squared_distance((boids[i]), boids[j]);

                    if (sqd < PROTECTED_RANGE*PROTECTED_RANGE) {
                        //Distance from near boids
                        close_dx += (boids[i].x - boids[j].x);
                        close_dy += (boids[i].y - boids[j].y);

                        //if not in protected range, check the visual one
                    }else if (sqd < VISUAL_RANGE*VISUAL_RANGE) {
                        x_avg += boids[j].x;
                        y_avg += boids[j].y;
                        xv_avg += boids[j].vx;
                        yv_avg += boids[j].vy;
                        n_neighbours++;
                    }

                }
            }
            //If there are boids in the visual range, make the boids go to their center
            if (n_neighbours > 0) {
                x_avg /= n_neighbours;
                y_avg /= n_neighbours;
                xv_avg /= n_neighbours;
                yv_avg /= n_neighbours;

                boids[i].vx = boids[i].vx + (x_avg-boids[i].x)*CENTERING_FACTOR + (xv_avg - boids[i].vx)*MATCHING_FACTOR;
                boids[i].vy = boids[i].vy + (y_avg-boids[i].y)*CENTERING_FACTOR + (yv_avg - boids[i].vy)*MATCHING_FACTOR;
            }
            boids[i].vx = boids[i].vx + close_dx*AVOID_FACTOR;
            boids[i].vy = boids[i].vy + close_dy*AVOID_FACTOR;

            //Verification of edges condition
            if (boids[i].y > TOP_MARGIN-MARGIN)
                boids[i].vy -= TURN_FACTOR;
            if (boids[i].y < BOT_MARGIN + MARGIN)
                boids[i].vy += TURN_FACTOR;
            if (boids[i].x < LEFT_MARGIN + MARGIN)
                boids[i].vx += TURN_FACTOR;
            if (boids[i].x > RIGHT_MARGIN - MARGIN)
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
        iterations++;

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        total_duration += duration;

        window.display();


    }

    total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(total_duration);

    append_csv("sequential_results.csv",
           cfg.N,
           cfg.frames,
           cfg.threads,
           total_duration.count());

    printf("Frame %d duration: %lld milliseconds\n", iterations, total_duration.count());

    return 0;
}

float random_float(float min, float max) {

    static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(gen);
}

float squared_distance(const Boid& a, const Boid& b){
    return static_cast<float>(pow((a.x - b.x), 2) + pow(a.y - b.y, 2));
}
void print_boid(Boid& boid, sf::RenderWindow& window) {
    boid.shape->setPosition({boid.x, boid.y});
    window.draw(*boid.shape);
}
void append_csv(const std::string& filename,
                int N, int frames, int threads,
                long long time_ms)
{
    static bool first = true;
    std::ofstream out(filename, std::ios::app);

    if (first) {
        out << "N,frames,threads,time_ms\n";
        first = false;
    }

    out << N << ","
        << frames << ","
        << threads << ","
        << time_ms << "\n";
}



