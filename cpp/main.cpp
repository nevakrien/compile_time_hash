#include "hash_table.hpp"
#include <iostream>
#include <string>
#include <cassert>

void grow_test() {
    HashTable<int, std::string, 2> table;
    table.insert(1, "one");
    table.insert(2, "two");
    table.insert(3, "three");
    table.insert(4, "four");
    table.insert(5, "five");

    // Trigger a grow operation
    table.insert(6, "six");
    table.insert(7, "seven");
}



// Global constexpr table
auto global_table = HashTable<int, std::string, 5>::from_nice_pairs({
    {1, "one"},
    {2, "two"},
    {3, "three"}
});

void testGlobalTable() {
    auto result = global_table.get(2);
    assert(result && result->get() == "two");
    std::cout << "Global table test passed.\n";
}

void testHashTable() {
    auto table = HashTable<int, std::string, 5>::from_pairs({
        {1, "one"},
        {2, "two"},
        {3, "three"},
        {6, "six"}
    });

    auto result = table.get(6);
    assert(result && result->get() == "six");
    std::cout << "Dynamic table test passed.\n";
}

int main() {
    testGlobalTable();
    testHashTable();
    grow_test();
    std::cout << "All tests passed successfully!\n";
    return 0;
}
