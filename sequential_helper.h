
#pragma once //to include the file only once

#include <string>
#include <iostream>
#include <memory>
#include <SFML/Graphics.hpp>
#include <bits/stdc++.h>

struct Config {

    int N = 1000;
    int frames = 300;
    int threads = 1;


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

struct Boid {
    float x, y;
    float vx, vy;
    std::unique_ptr<sf::Shape> shape;
};

float squared_distance(const Boid& a, const Boid& b);
float random_float(float min, float max);
void print_boid(Boid& boid, sf::RenderWindow& window);
void append_csv(const std::string& filename,
                int N, int frames, int threads,
                long long time_ms);