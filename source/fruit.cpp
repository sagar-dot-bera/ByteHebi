#include "fruit.h"
#include <chrono>

Fruit::Fruit(int width, int height)
    : width(width), height(height)
{
    auto seed = static_cast<unsigned>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());
    rng.seed(seed);
}
