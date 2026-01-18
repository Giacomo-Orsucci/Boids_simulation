//
// Created by giacomo on 18/01/26.
//
#include "SOA_helper_SIMD.h"

#include <cmath>
#include <iostream>
#include <memory>
#include <SFML/Graphics.hpp>
#include <vector>
#include <omp.h>
#include <fstream>
#include <random>
#include <optional>

/**
 * This is the SoA + SIMD version.
 * It combines the cache efficiency of Structure of Arrays with the
 * computational throughput of SIMD (AVX) instructions using branchless logic.
 **/

int main(int argc, char* argv[]) {

    Config cfg;
    cfg.parse(argc, argv);
    const int N = cfg.N;
    const int FRAMES = cfg.frames;

    omp_set_num_threads(cfg.threads);
    std::cout << "Threads set: " << cfg.threads << "\n";

#ifdef _OPENMP
    std::cout << "OPEN_MP working" << "\n";
#endif

    //Aligned Allocation
    Boids boids = allocate_aligned_boids(N);
    Boids boids_next = allocate_aligned_boids(N);
    std::vector<std::unique_ptr<sf::CircleShape>> shapes(N);

    // Constants definition
    constexpr float TURN_FACTOR = 0.2f;
    constexpr float VISUAL_RANGE = 40.0f;
    constexpr float PROTECTED_RANGE = 8.0f;
    constexpr float CENTERING_FACTOR = 0.0005f;
    constexpr float AVOID_FACTOR = 0.05f;
    constexpr float MATCHING_FACTOR = 0.05f;
    constexpr float MAX_SPEED = 6.0f;
    constexpr float MIN_SPEED = 3.0f;
    constexpr int TOP_MARGIN = 600;
    constexpr float BOT_MARGIN = 0;
    constexpr float LEFT_MARGIN = 0;
    constexpr int RIGHT_MARGIN = 800;
    constexpr float MARGIN = 80.0f;


    constexpr float SQ_PROTECTED_RANGE = PROTECTED_RANGE * PROTECTED_RANGE;
    constexpr float SQ_VISUAL_RANGE = VISUAL_RANGE * VISUAL_RANGE;

    int iterations = 0;
    std::chrono::milliseconds total_duration = std::chrono::milliseconds::zero();

    // Boids initialization
    for (int i = 0; i < N; i++) {
        boids.x[i] = random_float(LEFT_MARGIN + MARGIN, RIGHT_MARGIN - MARGIN);
        boids.y[i] = random_float(BOT_MARGIN + MARGIN, TOP_MARGIN - MARGIN);
        boids.vx[i] = random_float(-MAX_SPEED, MAX_SPEED);
        boids.vy[i] = random_float(-MAX_SPEED, MAX_SPEED);

        shapes[i] = std::make_unique<sf::CircleShape>(3.f, 3);
    }

    // Graphical window creation
    const int X_SIZE = RIGHT_MARGIN + (int)MARGIN;
    const int Y_SIZE = TOP_MARGIN + (int)MARGIN;

    sf::RenderWindow window(sf::VideoMode({X_SIZE, Y_SIZE}), "Boids simulation");
    window.setFramerateLimit(60);

    while (window.isOpen() && iterations < FRAMES) {
        window.clear(sf::Color::Black);
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        const auto start = std::chrono::high_resolution_clock::now();

        // --- Parallel Region ---
#pragma omp parallel default(none) shared(N, boids, boids_next)
        {
#pragma omp for schedule(static)
            for (int i = 0; i < N; i++) {

                //Variables definition and inizialization
                float xi = boids.x[i];
                float yi = boids.y[i];
                float vxi = boids.vx[i];
                float vyi = boids.vy[i];
                float x_avg = 0.0f;
                float y_avg = 0.0f;
                float xv_avg = 0.0f;
                float yv_avg = 0.0f;
                float n_neighbours = 0.0f;
                float close_dx = 0.0f;
                float close_dy = 0.0f;

                //To compare every boid with everyone else
#pragma omp simd
                for (int j = 0; j < N; j++) {

                    float dx = xi - boids.x[j];
                    float dy = yi - boids.y[j];
                    float dist_sq = dx*dx + dy*dy;

                    // Branchless logic
                    float is_protected = (dist_sq < SQ_PROTECTED_RANGE) ? 1.0f : 0.0f;
                    float is_visible   = (dist_sq < SQ_VISUAL_RANGE) ? 1.0f : 0.0f;

                    // A boid aligns only if it's visible BUT NOT protected
                    float is_alignment = is_visible - is_protected;


                    close_dx += dx * is_protected;
                    close_dy += dy * is_protected;


                    xv_avg += boids.vx[j] * is_alignment;
                    yv_avg += boids.vy[j] * is_alignment;
                    x_avg  += boids.x[j]  * is_alignment;
                    y_avg  += boids.y[j]  * is_alignment;
                    n_neighbours += is_alignment;
                }

                // --- End SIMD Loop ---

                //If there are boids in the visual range, make the boids go to their center
                if (n_neighbours > 0.0f) {
                    x_avg /= n_neighbours;
                    y_avg /= n_neighbours;
                    xv_avg /= n_neighbours;
                    yv_avg /= n_neighbours;

                    vxi += (x_avg - xi) * CENTERING_FACTOR + (xv_avg - vxi) * MATCHING_FACTOR;
                    vyi += (y_avg - yi) * CENTERING_FACTOR + (yv_avg - vyi) * MATCHING_FACTOR;
                }

                vxi += close_dx * AVOID_FACTOR;
                vyi += close_dy * AVOID_FACTOR;

                //Verification of edges condition
                if (yi > TOP_MARGIN - MARGIN)
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
                } else if (speed > MAX_SPEED) {
                    float scale = MAX_SPEED / speed;
                    vxi *= scale;
                    vyi *= scale;
                }


                boids_next.x[i]  = xi + vxi;
                boids_next.y[i]  = yi + vyi;
                boids_next.vx[i] = vxi;
                boids_next.vy[i] = vyi;
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

    append_csv("SOA_SIMD_noPadding_results.csv",
          cfg.N,
          cfg.frames,
          cfg.threads,
          total_duration.count());

    printf("Frame %d duration: %lld milliseconds\n", iterations, total_duration.count());


    free_boids_aligned(boids);
    free_boids_aligned(boids_next);

    return 0;
}


float random_float(float min, float max) {
    static std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(gen);
}

void print_boids(const Boids& boids, int N,
                 std::vector<std::unique_ptr<sf::CircleShape>>& shapes,
                 sf::RenderWindow& window)
{
    for (int i = 0; i < N; ++i) {
        shapes[i]->setPosition({boids.x[i], boids.y[i]});
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
inline Boids allocate_aligned_boids(int N) {
    Boids boids;
    const size_t ALIGNMENT = 32;
    size_t size = N * sizeof(float);

    // Padding: size passed to aligned_alloc must be a multiple of alignment
    if (size % ALIGNMENT != 0) {
        size += ALIGNMENT - (size % ALIGNMENT);
    }


    boids.x  = static_cast<float*>(std::aligned_alloc(ALIGNMENT, size));
    boids.y  = static_cast<float*>(std::aligned_alloc(ALIGNMENT, size));
    boids.vx = static_cast<float*>(std::aligned_alloc(ALIGNMENT, size));
    boids.vy = static_cast<float*>(std::aligned_alloc(ALIGNMENT, size));

    if (!boids.x || !boids.y || !boids.vx || !boids.vy) {
        std::cerr << "Aligned allocation failed!" << std::endl;
        exit(EXIT_FAILURE);
    }

    return boids;
}