#ifndef HASH_TABLE_HPP
#define HASH_TABLE_HPP

#include <iostream>
#include <array>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>
#include <cstring>

// Key concept that requires hash, equality, and null check
template <typename K>
concept Key = requires(K k1, K k2) {
    { k1.hash() } -> std::convertible_to<size_t>;
    { k1 == k2 } -> std::convertible_to<bool>;
    { k1.isNull() } -> std::convertible_to<bool>;
};

// Generic key template for integral types
template <typename T>
requires std::is_integral_v<T>
struct IntKey {
    T value;
    constexpr size_t hash() const { return static_cast<size_t>(value); }
    constexpr bool operator==(const IntKey& other) const { return value == other.value; }
    constexpr bool isNull() const { return value == 0; }
};

// Key template for C-strings
struct CStringKey {
    const char* str;
    constexpr size_t hash() const {
        size_t hash = 0;
        for (const char* c = str; *c; ++c) {
            hash = hash * 31 + static_cast<size_t>(*c);
        }
        return hash;
    }
    constexpr bool operator==(const CStringKey& other) const {
        return std::string_view(str) == std::string_view(other.str);
    }
    constexpr bool isNull() const { return str == nullptr || *str == '\0'; }
};

// Key template for std::string
struct StringKey {
    std::string str;
    size_t hash() const {
        return std::hash<std::string>{}(str);
    }
    bool operator==(const StringKey& other) const { return str == other.str; }
    bool isNull() const { return str.empty(); }
};


// HeapArray structure with pointer + length
template <typename T>
struct HeapArray {
    T* data = nullptr;
    size_t length = 0;

    HeapArray() = default;
    HeapArray(size_t size) : length(size), data(new T[size]()) {}  // ✅ This is enough


    ~HeapArray() { delete[] data; }

    T& operator[](size_t index) { return data[index]; }
    const T& operator[](size_t index) const { return data[index]; }

    //  // Free memory WITHOUT calling destructors
    // void freeMemory() {
    //     if (data) {
    //         ::operator delete(data, sizeof(T) * length);
    //         data = nullptr;
    //         length = 0;
    //     }
    // }

};

// HashNode structure
template <typename K, typename V>
struct HashNode {
    K key = {};
    V value = {};
    std::unique_ptr<HashNode<K, V>> next = nullptr;

    // Constructor for convenient initialization
    HashNode(const K& k, const V& v, std::nullptr_t) : key(k), value(v), next(nullptr) {}

    // Default constructor
    HashNode() = default;

    // Deep copy constructor: Fully copies all linked nodes
    HashNode(const HashNode& other) : key(other.key), value(other.value) {
        if (other.next) {
            HashNode<K, V>* current = this;  // Start from `this`
            const HashNode<K, V>* otherCurrent = &other;  // Traverse `other`

            while (otherCurrent->next) {  // Traverse the original linked list
                otherCurrent = otherCurrent->next.get();
                current->next = std::make_unique<HashNode<K, V>>(otherCurrent->key, otherCurrent->value, nullptr);
                current = current->next.get();  // Move to the next node
            }
        }
    }

    // Copy assignment operator: Proper deep copy using a loop
    HashNode& operator=(const HashNode& other) {
        if (this == &other) return *this; // Prevent self-assignment
        key = other.key;
        value = other.value;

        // Copy the entire linked list
        if (other.next) {
            HashNode<K, V>* current = this;
            const HashNode<K, V>* otherCurrent = &other;

            while (otherCurrent->next) {
                otherCurrent = otherCurrent->next.get();
                current->next = std::make_unique<HashNode<K, V>>(otherCurrent->key, otherCurrent->value, nullptr);
                current = current->next.get();  // Move to the next node
            }
        } else {
            next.reset();  // If `other` has no `next`, reset it
        }

        return *this;
    }

    // **Move Constructor**: Transfers ownership without deep copy
    HashNode(HashNode&& other) noexcept : key(std::move(other.key)), value(std::move(other.value)), next(std::move(other.next)) {}

    // **Move Assignment Operator**: Efficiently moves data without unnecessary copies
    HashNode& operator=(HashNode&& other) noexcept {
        if (this == &other) return *this; // Prevent self-assignment
        key = std::move(other.key);
        value = std::move(other.value);
        next = std::move(other.next);  // Move ownership of `next`
        return *this;
    }

    bool isOccupied() const { return !key.isNull(); }
};



// HashTable structure
template <typename K, typename V, size_t BaseSize>
struct HashTable {
    std::array<HashNode<K, V>, BaseSize> base = {};
    HeapArray<HashNode<K, V>> heap = {};
    size_t entryCount = 0;

    // Default constructor
    HashTable() = default;

    // Deep copy constructor
    HashTable(const HashTable& other) :  entryCount(other.entryCount) {
        base = other.base;  // Uses HashNode's copy constructor
        if (heap.length > 0) {
            heap = HeapArray<HashNode<K, V>>(heap.length);
            for (size_t i = 0; i < heap.length; ++i) {
                heap[i] = other.heap[i];  // Calls HashNode copy constructor
            }
        }
    }

    // Copy assignment operator
    HashTable& operator=(const HashTable& other) {
        if (this == &other) return *this; // Prevent self-assignment
        base = other.base;
        entryCount = other.entryCount;
        if (heap.length > 0) {
            heap = HeapArray<HashNode<K, V>>(heap.length);
            for (size_t i = 0; i < heap.length; ++i) {
                heap[i] = other.heap[i];
            }
        }
        return *this;
    }

// Move Constructor
HashTable(HashTable&& other) noexcept
    : base(std::move(other.base)), heap(std::move(other.heap)), entryCount(other.entryCount) {
    other.entryCount = 0;
}

// Move Assignment Operator
HashTable& operator=(HashTable&& other) noexcept {
    if (this == &other) return *this; // Prevent self-assignment

    heap.freeMemory();  // ✅ Ensure old heap is properly released before moving
    base = std::move(other.base);
    heap = std::move(other.heap);
    entryCount = other.entryCount;
    
    // Reset other table to avoid dangling references
    other.entryCount = 0;
    return *this;
}




void insert(const K& key, const V& value) {
    ++entryCount;  // Move count update to the beginning

    if (entryCount > (BaseSize + heap.length) * 0.75) {
        grow();
    }

    size_t totalSize = BaseSize + heap.length;
    size_t index = key.hash() % totalSize;

    if (index < BaseSize) { 
        // ✅ Write to base array
        if (!base[index].isOccupied()) {
            base[index] = {key, value, nullptr};
            return;
        }

        HashNode<K, V>* node = &base[index];
        while (node->next) node = node->next.get();

        node->next = std::make_unique<HashNode<K, V>>(HashNode<K, V>{key, value, nullptr});
    } else { 
        // ✅ Write to heap array
        size_t heapIndex = index - BaseSize;
        if (!heap[heapIndex].isOccupied()) {
            heap[heapIndex] = {key, value, nullptr};
            return;
        }

        HashNode<K, V>* node = &heap[heapIndex];
        while (node->next) node = node->next.get();

        node->next = std::make_unique<HashNode<K, V>>(HashNode<K, V>{key, value, nullptr});
    }
}


void remove(const K& key) {
    if (entryCount == 0) return;  // Nothing to remove

    --entryCount;

    size_t totalSize = BaseSize + heap.length;
    size_t index = key.hash() % totalSize;

    HashNode<K, V>* node = (index < BaseSize) ? &base[index] : &heap[index - BaseSize];
    HashNode<K, V>* prev = nullptr;

    while (node && node->key != key) {
        prev = node;
        node = node->next.get();
    }

    if (node) {
        if (prev) {
            prev->next = std::move(node->next);
        } else {
            if (node->next) {
                *node = std::move(*node->next);
                node->next.reset();
            } else {
                *node = {};  // Reset node
            }
        }
    }
}
    


constexpr const V* get(const K& key) const {
    size_t totalSize = BaseSize + heap.length;
    size_t index = key.hash() % totalSize;

    const HashNode<K, V>* node = (index < BaseSize) ? &base[index] : &heap[index - BaseSize];

    while (node) {
        if (node->isOccupied() && node->key == key) {
            return &node->value;
        }
        node = node->next.get();
    }
    return nullptr;
}

void grow() {
    std::cout << "growing" << std::endl;
    size_t newheapLen = (heap.length == 0) ? 4 : heap.length * 2;

    // Step 1: Move all elements into a temporary vector
    std::vector<std::pair<K, V>> allEntries;
    allEntries.reserve(entryCount);  // Set capacity to avoid extra allocations

    // Move elements from base table
    for (size_t i = 0; i < BaseSize; ++i) {
        HashNode<K, V>* node = &base[i];
        while (node && node->isOccupied()) {
            allEntries.emplace_back(std::move(node->key), std::move(node->value));
            node = node->next.get();
        }
    }

    // Move elements from heap table
    for (size_t i = 0; i < heap.length; ++i) {
        if (heap[i].isOccupied()) {
            allEntries.emplace_back(std::move(heap[i].key), std::move(heap[i].value));
        }
    }


    heap = HeapArray<HashNode<K, V>>(newheapLen);

    entryCount = 0;
    for (auto& [key, value] : allEntries) {
        insert(std::move(key), std::move(value));  // Move back into the table
    }
}

template <size_t N>
constexpr static HashTable make_table(const std::array<std::pair<K, V>, N>& entries) {
    HashTable table;
    for (const auto& [key, value] : entries) {
        size_t index = key.hash() % BaseSize;
        if (table.base[index].isOccupied()) {
            throw std::logic_error("Compile-time collision detected!");
        }
        table.base[index] = {key, value, nullptr};
    }
    return table;
}
};


#endif // HASH_TABLE_HPP
