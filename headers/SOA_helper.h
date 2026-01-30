//
// Created by giacomo on 17/01/26.
//

#pragma once //to include the file only once


#include <string>
#include <iostream>
#include <memory>
#include <SFML/Graphics.hpp>

struct Config {

    int N = 1000;
    int frames = 300;
    int threads = 8;
    std::string csv;


    //Parsing params passed via command line
    void parse(int argc, char* argv[]) {
        for (int i = 1; i < argc; ++i) { //i = 1 because for i=0 we always have the exe path
            std::string arg = argv[i];
            if (arg == "--N" && i + 1 < argc) {
                N = std::stoi(argv[++i]);
            } else if (arg == "--frames" && i + 1 < argc) {
                frames = std::stoi(argv[++i]);
            }else if (arg == "--threads" && i + 1 < argc) {
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

//Shape separated from Boid to better parallelize
struct Boids {
    float *x, *y;
    float *vx, *vy;
};

inline Boids boids_allocation(int N) {
    Boids boids;

    boids.x  = new float[N];
    boids.y  = new float[N];
    boids.vx = new float[N];
    boids.vy = new float[N];

    return boids;

}
float random_float(float min, float max);

void print_boids(const Boids& boids, int N,
                 std::vector<std::unique_ptr<sf::CircleShape>>& shapes,
                 sf::RenderWindow& window);

void append_csv(const std::string& filename,
                int N, int frames, int threads,
                long long time_ms);

inline void free_boids(Boids& boids) {
    delete [] boids.x;
    delete [] boids.y;
    delete [] boids.vx;
    delete [] boids.vy;
}