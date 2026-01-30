
//
// Created by giacomo on 05/12/25.
//

#pragma once

#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <SFML/Graphics.hpp>
#include <cstdlib>

/**
 * This helper provides the Structure of Arrays (SOA) layout with aligned memory allocation.
 * Alignment (32 bytes) is required for more efficient SIMD (AVX) processing.
 **/

struct Config {

    int N = 1500;
    int frames = 300;
    int threads = 8;
    std::string csv;

    //Parsing params passed via command line
    void parse(int argc, char* argv[]) {
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--N" && i + 1 < argc) {
                N = std::stoi(argv[++i]);
            } else if (arg == "--frames" && i + 1 < argc) {
                frames = std::stoi(argv[++i]);
            } else if (arg == "--threads" && i + 1 < argc) {
                threads = std::stoi(argv[++i]);
            }else if (arg == "--csv" && i + 1 < argc) {
                csv = argv[++i];
            }
            else {
                std::cerr << "Unknown argument: " << arg << std::endl;
            }
        }
    }

    void print() const {
        std::cout << "Config: N=" << N
                  << ", frames=" << frames
                  << ", threads=" << threads
                  << std::endl;
    }
};


struct Boids {
    float *x, *y;
    float *vx, *vy;
};

// Aligned allocation ensures the starting address of each array is a multiple of 32 bytes.
inline Boids allocate_aligned_boids(int N);

inline void free_boids_aligned(Boids& boids) {
    std::free(boids.x);
    std::free(boids.y);
    std::free(boids.vx);
    std::free(boids.vy);
}

float random_float(float min, float max);

void print_boids(const Boids& boids, int N,
                 std::vector<std::unique_ptr<sf::CircleShape>>& shapes,
                 sf::RenderWindow& window);

void append_csv(const std::string& filename,
                int N, int frames, int threads,
                long long time_ms);