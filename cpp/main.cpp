#include "hash_table.hpp"

int main() {
    HashTable<int, std::string, 2> table;
    table.insert(1, "one");
    table.insert(2, "two");
    table.insert(3, "three");
    table.insert(4, "four");
    table.insert(5, "five");

    // Trigger a grow operation
    table.insert(6, "six");
    table.insert(7, "seven");

    return 0;
}
