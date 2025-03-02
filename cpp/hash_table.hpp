#ifndef HASH_TABLE_HPP
#define HASH_TABLE_HPP

#include <iostream>
#include <array>
#include <memory>
#include <utility>
#include <optional>


template <typename K, typename V>
struct HashNode {
    K key;
    V value;
    std::unique_ptr<HashNode<K, V>> next;

    // Default constructor
    HashNode() : key(), value(), next(nullptr) {}

    // Constructor with key and value
    HashNode(const K& k, const V& v) : key(k), value(v), next(nullptr) {}

    // Constructor with key, value, and next pointer
    HashNode(const K& k, const V& v, std::unique_ptr<HashNode<K, V>> nextNode)
        : key(k), value(v), next(std::move(nextNode)) {}

    // Move constructor
    HashNode(HashNode&& other) noexcept
        : key(std::move(other.key)),
          value(std::move(other.value)),
          next(std::move(other.next)) {}

    // Move assignment operator
    HashNode& operator=(HashNode&& other) noexcept {
        if (this != &other) {
            key = std::move(other.key);
            value = std::move(other.value);
            next = std::move(other.next);
        }
        return *this;
    }

    // Copy constructor (deleted to prevent accidental copies)
    HashNode(const HashNode&) = delete;

    // Copy assignment operator (deleted to prevent accidental copies)
    HashNode& operator=(const HashNode&) = delete;
};


template <typename K, typename V, size_t base_size>
class HashTable {
    std::array<std::optional<HashNode<K, V>>, base_size> base_array;//this can be constexpr
    std::unique_ptr<std::unique_ptr<HashNode<K, V>>[]> heap_array = nullptr;
    size_t heap_size = 0;
    size_t num_entries = 0;

public:
    // Constructor
    HashTable() {}


    // Grow function to resize heap array
    void grow() {
        size_t new_heap_size = heap_size * 2 + 10;

        // Allocate new larger array
        std::unique_ptr<std::unique_ptr<HashNode<K, V>>[]> new_heap_array(new std::unique_ptr<HashNode<K, V>>[new_heap_size]());

        // Move entries from old heap array to new array
        for (size_t i = 0; i < heap_size; ++i) {
            if (heap_array[i]) {
                new_heap_array[i] = std::move(heap_array[i]);
            }
        }

        for (size_t i = 0; i < base_size; ++i) {
            if (base_array[i]) {
                new_heap_array[i] = std::make_unique<HashNode<K, V>>(std::move(base_array[i]).value());

            }
        }

        // Clean up old array and update pointers
        heap_array = std::move(new_heap_array);
        heap_size = new_heap_size;
    }

// Insert a new key-value pair (in-place for the base array)
void insert(const K& key, const V& value) {
    // Grow if load factor exceeds 0.7
    if (++num_entries > 0.7 * (base_size + heap_size)) {
        grow();
    }

    // Compute the index for insertion over both arrays.
    size_t index = std::hash<K>{}(key) % (base_size + heap_size);

    if (index < base_size) {
        // For the base array, construct in place.
        if (!base_array[index]) {
            base_array[index].emplace(key, value);
        } else {
            // Collision in base array: allocate a new node on the heap and chain it.
            auto new_node = std::make_unique<HashNode<K, V>>(key, value, nullptr);
            new_node->next = std::move(base_array[index]->next);
            base_array[index]->next = std::move(new_node);
        }
    } else {
        // For the heap array, compute the proper heap index.
        size_t heap_index = index - base_size;
        if (!heap_array[heap_index]) {
            heap_array[heap_index] = std::make_unique<HashNode<K, V>>(key, value, nullptr);
        } else {
            // Collision in heap array: chain using heap allocation.
            auto new_node = std::make_unique<HashNode<K, V>>(key, value, nullptr);
            new_node->next = std::move(heap_array[heap_index]);
            heap_array[heap_index] = std::move(new_node);
        }
    }
}


};


#endif // HASH_TABLE_HPP
