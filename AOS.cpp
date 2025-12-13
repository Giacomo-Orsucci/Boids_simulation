#include "AOS_helper.h"

#include <cmath>
#include <iostream>
#include <memory>
#include <SFML/Graphics.hpp>
#include <bits/stdc++.h>
#include <omp.h>

/**
 * This is the Array Of Structure version of boids simulation with graphics.
 * The code is optimized and designed for a parallel execution with exception for graphics.
 * The measurements, to ensure a fair comparison, are done on the "core" of boids simulation.
 *  The execution of this code with only 1 thread has been empirically proven to be equivalent to
 *  the sequential one.
 * **/

int main(int argc, char* argv[]) {

    Config cfg;
    cfg.parse(argc, argv);
    const int N = cfg.N;
    const int FRAMES = cfg.frames;

    //cfg.threads = 8; // to test
    omp_set_num_threads(cfg.threads);
    std::cout<<"Threads set: " <<cfg.threads<<"\n";


#ifdef _OPENMP
    std::cout << "OPEN_MP working" << "\n";
#endif


    std::vector<Boid> boids(N);
    std::vector<std::unique_ptr<sf::CircleShape>> shapes(N);

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


    int iterations = 0;

    std::chrono::milliseconds total_duration = std::chrono::milliseconds::zero();

    //boids initialization
    for (int i=0; i < N; i++) {
        boids[i].x = random_float(LEFT_MARGIN+MARGIN, RIGHT_MARGIN-MARGIN);
        boids[i].y = random_float(BOT_MARGIN+MARGIN, TOP_MARGIN-MARGIN);

        boids[i].vx = random_float(-MAX_SPEED, MAX_SPEED);
        boids[i].vy = random_float(-MAX_SPEED, MAX_SPEED);

        shapes[i] = std::make_unique<sf::CircleShape>(3.f, 3);

    }

    //Graphical window creation
    const int X_SIZE = RIGHT_MARGIN + MARGIN;
    const int Y_SIZE =  TOP_MARGIN + MARGIN;

    sf::RenderWindow window(sf::VideoMode({X_SIZE, Y_SIZE}), "Boids simulation");
    window.setFramerateLimit(60); // call it once after creating the window


    std::vector<Boid> boids_next = boids;

    while (window.isOpen() && iterations < FRAMES) {
        window.clear(sf::Color::Black);
        while (const std::optional event = window.pollEvent())
        {

            if (event->is<sf::Event::Closed>())
                window.close();
        }


        // without counting the graphic, pure boids performance
        const auto start = std::chrono::high_resolution_clock::now();

        // Here starts the parallelization
#pragma omp parallel default(none) shared(N, boids, boids_next)
        {
#pragma omp for schedule(static)
        for (int i=0; i<N; i++) { //To scan every boid

            //Variables definition and inizialization
            float dx =0, dy = 0, sqd=0, close_dx=0, close_dy=0, x_avg=0,
            y_avg=0, xv_avg=0, yv_avg=0, n_neighbours=0;

            float xi = boids[i].x;
            float yi = boids[i].y;
            float vxi = boids[i].vx;
            float vyi = boids[i].vy;

            //To compare every boid with everyone else
            for (int j = 0; j<N; j++){
                if (j==i)
                    continue;

                dx = xi - boids[j].x;
                dy = yi - boids[j].y;

                if (std::fabs(dx)<VISUAL_RANGE && std::fabs(dy)<VISUAL_RANGE) {

                    sqd = dx*dx + dy*dy;

                    if (sqd < PROTECTED_RANGE*PROTECTED_RANGE) {
                        //Distance from near boids
                        close_dx += (xi - boids[j].x);
                        close_dy += (yi - boids[j].y);

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

                vxi = vxi + (x_avg-xi)*CENTERING_FACTOR + (xv_avg - vxi)*MATCHING_FACTOR;
                vyi = vyi + (y_avg-yi)*CENTERING_FACTOR + (yv_avg - vyi)*MATCHING_FACTOR;
            }
            vxi = vxi + close_dx*AVOID_FACTOR;
            vyi = vyi + close_dy*AVOID_FACTOR;

            //Verification of edges condition
            if (yi > TOP_MARGIN-MARGIN)
                vyi -= TURN_FACTOR;
            if (yi < BOT_MARGIN + MARGIN)
                vyi += TURN_FACTOR;
            if (xi < LEFT_MARGIN + MARGIN)
                vxi += TURN_FACTOR;
            if (xi > RIGHT_MARGIN - MARGIN)
                vxi -= TURN_FACTOR;

            float speed = sqrt(vxi*vxi + vyi*vyi);

            if (speed > 0 && speed < MIN_SPEED) {
                vxi = (vxi/speed)*MIN_SPEED;
                vyi = (vyi/speed)*MIN_SPEED;
            }
            else if (speed > 0 && speed > MAX_SPEED) {
                vxi = (vxi/speed)*MAX_SPEED;
                vyi = (vyi/speed)*MAX_SPEED;
            }

            boids_next[i].x  = xi + vxi;
            boids_next[i].y  = yi + vyi;
            boids_next[i].vx = vxi;
            boids_next[i].vy = vyi;

        }
        }

        boids.swap(boids_next);

        iterations++;

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        total_duration += duration;

        //Graphical part not parallelized, so outside the measurement

        print_boids(boids, shapes, window);

        window.display();

    }
    total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(total_duration);
    append_csv("AOS_parallel_results.csv",
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


void print_boids(const std::vector<Boid>& boids, std::vector<std::unique_ptr<sf::CircleShape>>& shapes,
                 sf::RenderWindow& window)
{
    for (int i = 0; i < boids.size(); ++i) {
        shapes[i]->setPosition({boids[i].x, boids[i].y});
        window.draw(*shapes[i]);
    }

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


