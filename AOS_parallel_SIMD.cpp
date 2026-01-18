#include "AOS_helper_SIMD.h"

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
 * Differently from AOS_parallel uses SIMD directives and helper with alignment padding in boid struct.
 * To this purpose, I didn't use std::vector<> and I reorganized the code to facilitate SIMD optimization.
 * (where the code uses SIMD directives, there are no different possible paths).
 *
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

    //Aligned allocation
    Boid* boids = allocate_aligned_boids(N);
    Boid* boids_next = allocate_aligned_boids(N);
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

    constexpr float SQ_PROTECTED_RANGE = PROTECTED_RANGE*PROTECTED_RANGE;
    constexpr float SQ_VISUAL_RANGE = VISUAL_RANGE*VISUAL_RANGE;


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




    while (window.isOpen() && iterations < FRAMES) {
        window.clear(sf::Color::Black);
        while (const std::optional event = window.pollEvent())
        {

            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // without counting the graphic, pure boids performance
        const auto start = std::chrono::high_resolution_clock::now();

        // --- Parallel Region ---
#pragma omp parallel default(none) shared(N, boids, boids_next)
        {
#pragma omp for schedule(static)
        for (int i=0; i<N; i++) { //To scan every boid

            //Variables definition and inizialization
            float xi  = boids[i].x;
            float yi  = boids[i].y;
            float vxi = boids[i].vx;
            float vyi = boids[i].vy;
            float x_avg=0;
            float y_avg=0;
            float xv_avg=0;
            float yv_avg=0;
            float n_neighbours=0;
            float close_dx = 0.0f;
            float close_dy = 0.0f;



            //To compare every boid with everyone else
#pragma omp simd
            for (int j = 0; j < N; j++){


                float dx = xi - boids[j].x;
                float dy = yi - boids[j].y;
                float dist_sq = dx*dx + dy*dy;

                // Branchless logic
                float is_protected = (dist_sq < SQ_PROTECTED_RANGE) ? 1.0f : 0.0f;
                float is_visible   = (dist_sq < SQ_VISUAL_RANGE) ? 1.0f : 0.0f;

                // A boid aligns only if it's visible BUT NOT protected
                float is_alignment = is_visible - is_protected;

                close_dx += (dx) * is_protected;
                close_dy += (dy) * is_protected;

                xv_avg += boids[j].vx * is_alignment;
                yv_avg += boids[j].vy * is_alignment;

                x_avg += boids[j].x * is_alignment;
                y_avg += boids[j].y * is_alignment;

                n_neighbours += is_alignment;

            }

            // --- End SIMD Loop ---

            //If there are boids in the visual range, make the boids go to their center
            if (n_neighbours > 0.0f) {
                x_avg /= n_neighbours;
                y_avg /= n_neighbours;
                xv_avg /= n_neighbours;
                yv_avg /= n_neighbours;

                vxi +=  (x_avg - xi) * CENTERING_FACTOR + (xv_avg - vxi) * MATCHING_FACTOR;
                vyi += (y_avg - yi) * CENTERING_FACTOR + (yv_avg - vyi) * MATCHING_FACTOR;
            }
            vxi += close_dx * AVOID_FACTOR;
            vyi += close_dy * AVOID_FACTOR;


            //Verification of edges condition
            if (yi > TOP_MARGIN-MARGIN)
                vyi -= TURN_FACTOR;
            if (yi < BOT_MARGIN + MARGIN)
                vyi += TURN_FACTOR;
            if (xi < LEFT_MARGIN + MARGIN)
                vxi += TURN_FACTOR;
            if (xi > RIGHT_MARGIN - MARGIN)
               vxi -= TURN_FACTOR;

            float speed = std::sqrt(vxi*vxi + vyi*vyi);

            if (speed > 0 && speed < MIN_SPEED) {
                float scale = MIN_SPEED / speed;
                vxi *= scale;
                vyi *= scale;
            }
            else if (speed > MAX_SPEED) {
                float scale = MAX_SPEED / speed;
                vxi *= scale;
                vyi *= scale;
            }

            boids_next[i].x  = xi + vxi;
            boids_next[i].y  = yi + vyi;
            boids_next[i].vx = vxi;
            boids_next[i].vy = vyi;

        }
        }

        std::swap(boids, boids_next);

        iterations++;

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        total_duration += duration;

        //Graphical part not parallelized, so outside the measurement

        print_boids(boids, N, shapes, window);

        window.display();

    }
    total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(total_duration);
    append_csv("AOS_parallel_SIMD_noPadding.csv",
          cfg.N,
          cfg.frames,
          cfg.threads,
          total_duration.count());

    printf("Frame %d duration: %lld milliseconds\n", iterations, total_duration.count());

    free(boids);
    free(boids_next);
    return 0;
}

float random_float(float min, float max) {

    static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(gen);
}


void print_boids(const Boid* boids, int N, std::vector<std::unique_ptr<sf::CircleShape>>& shapes,
                 sf::RenderWindow& window)
{
    for (int i = 0; i < N; ++i) {
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
// Aligned allocation ensures the starting address of each array is a multiple of 32 bytes.
Boid* allocate_aligned_boids(int N) {

    const size_t ALIGNMENT = 32;

    size_t total_size = N * sizeof(Boid);

    // Padding: size passed to aligned_alloc must be a multiple of alignment
    if (total_size % ALIGNMENT != 0) {
        total_size += ALIGNMENT - (total_size % ALIGNMENT);
    }

    void* ptr = std::aligned_alloc(ALIGNMENT, total_size);

    if (!ptr) {
        std::cerr << "Aligned allocation failed!" << std::endl;
        exit(EXIT_FAILURE);
    }

    return static_cast<Boid*>(ptr);
}


