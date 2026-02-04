#include <iostream>
#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>

// Mock Tileset class
struct Tileset {
    std::string name;
    int data[100]; // simulate some payload
};

using MapContainer = std::map<std::string, Tileset*>;
using UnorderedMapContainer = std::unordered_map<std::string, Tileset*>;

std::string random_string(size_t length) {
    auto randchar = []() -> char {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[rand() % max_index];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), length, randchar);
    return str;
}

int main() {
    const int num_tilesets = 1000;
    const int num_lookups = 1000000;

    std::vector<std::string> keys;
    for (int i = 0; i < num_tilesets; ++i) {
        keys.push_back("tileset_" + random_string(10));
    }

    MapContainer map_c;
    UnorderedMapContainer umap_c;

    // Populate containers
    for (const auto& key : keys) {
        map_c[key] = new Tileset{key};
        umap_c[key] = new Tileset{key};
    }

    // Benchmark Map
    auto start = std::chrono::high_resolution_clock::now();
    volatile int dummy = 0;
    for (int i = 0; i < num_lookups; ++i) {
        std::string key = keys[i % num_tilesets];
        auto it = map_c.find(key);
        if (it != map_c.end()) {
            dummy += it->second->name.length();
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "std::map lookups: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms" << std::endl;

    // Benchmark Unordered Map
    start = std::chrono::high_resolution_clock::now();
    dummy = 0;
    for (int i = 0; i < num_lookups; ++i) {
        std::string key = keys[i % num_tilesets];
        auto it = umap_c.find(key);
        if (it != umap_c.end()) {
            dummy += it->second->name.length();
        }
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "std::unordered_map lookups: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms" << std::endl;

    // Cleanup
    for (auto& pair : map_c) delete pair.second;
    // Pointers are shared, so no need to delete twice, but need to be careful not to double free if real code.
    // In this benchmark, we allocated distinct objects? No, wait.
    // map_c[key] = new Tileset{key};
    // umap_c[key] = new Tileset{key};
    // They are distinct objects here because of how I wrote the loop.
    for (auto& pair : umap_c) delete pair.second;

    return 0;
}
