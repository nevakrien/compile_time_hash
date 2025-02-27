#include "hash_table.hpp"
#include <cassert>
#include <iostream>
#include <array>

// Compile-time HashTable creation
constexpr std::array<std::pair<IntKey<int>, int>, 4> predefined_entries = {
    std::make_pair(IntKey<int>{1}, 100),
    std::make_pair(IntKey<int>{2}, 200),
    std::make_pair(IntKey<int>{3}, 300),
    std::make_pair(IntKey<int>{4}, 400)
};

auto globalTable = HashTable<IntKey<int>, int, 10>::make_table(predefined_entries);

int main() {
    // Verify compile-time retrieval
    assert(globalTable.get(IntKey<int>{1}) != nullptr && "Compile-time retrieval failed!");
    assert(*globalTable.get(IntKey<int>{1}) == 100 && "Incorrect value at compile-time!");


    // Runtime HashTable
    HashTable<IntKey<int>, int, 10> table = globalTable;

    // Check initial values
    for (int i = 1; i <= 4; ++i) {
        if (auto* value = table.get(IntKey<int>{i})) {
            std::cout << "Key " << i << ": " << *value << std::endl;
        } else {
            std::cout << "Key " << i << " not found!" << std::endl;
        }
    }


    // Insert multiple new values to trigger growth
    for (int i = 5; i <= 150; ++i) {
        std::cout << "inserting " << i << std::endl;
        table.insert(IntKey<int>{i}, i * 100);
    }

    // Verify inserted values
    for (int i = 5; i <= 150; ++i) {
        if (auto* value = table.get(IntKey<int>{i})) {
            std::cout << "Key " << i << ": " << *value << std::endl;
        } else {
            std::cout << "Key " << i << " not found!" << std::endl;
        }
    }

    // Remove some keys
    for (int i = 2; i <= 4; ++i) {
        table.remove(IntKey<int>{i});
    }

    // Ensure keys are removed
    for (int i = 2; i <= 4; ++i) {
        if (table.get(IntKey<int>{i}) == nullptr) {
            std::cout << "Key " << i << " successfully removed!" << std::endl;
        } else {
            std::cout << "Key " << i << " still exists!" << std::endl;
        }
    }

    return 0;
}
