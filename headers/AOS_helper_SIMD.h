//
// Created by giacomo on 05/12/25.
//

#pragma once //to include the file only once

/**
 * This helper is very similar to the AOS one, but uses padding and SIMD instructions.
 **/


#include <string>
#include <iostream>
#include <memory>
#include <SFML/Graphics.hpp>
#include <bits/stdc++.h>



struct Config {

    int N = 1500;
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
#define CACHE_SIZE  32

//Shape separated from Boid to better parallelize
// comment alignas(CACHE_SIZE) to test without padding
struct /*alignas(CACHE_SIZE)*/ Boid { // CACHE_SIZE = 32 for my CPU.
    float x, y;
    float vx, vy;

    // comment to test without padding
   // char padding = (CACHE_SIZE - sizeof(float)*4); //to do a more "internal" padding and not only alignment
};


Boid* allocate_aligned_boids(int N);

float random_float(float min, float max);
void print_boids(const Boid* boids, int N, std::vector<std::unique_ptr<sf::CircleShape>>& shapes,
                 sf::RenderWindow& window);
void append_csv(const std::string& filename,
                int N, int frames, int threads,
                long long time_ms);
