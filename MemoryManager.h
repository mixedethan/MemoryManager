#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <functional>

/*
 *
 *               By Ethan Wilson
 *                  3/22/2024
 *                 Verison 1.0
 */


struct MemoryBlock { // struct to represent memory blocks (linked list)
    size_t start; // starting address of the block
    size_t size; // size of block in words
    bool isFree; // determines where the block is free of allocated
    MemoryBlock *next; // pointer to next memory block
    MemoryBlock *prev; // pointer to previous block

    MemoryBlock(size_t start, size_t size, bool isFree, MemoryBlock *next = nullptr, MemoryBlock *prev = nullptr)
            : start(start), size(size), isFree(isFree), next(next), prev(prev) {}
};

class MemoryManager {
public:
    MemoryManager(unsigned wordSize, std::function<int(int, void *)> allocator);

    ~MemoryManager();

    void initialize(size_t sizeInWords);

    void shutdown();

    void *allocate(size_t sizeInBytes);

    void free(void *address);

    void *getList(); // Gets list of free blocks

    void *getBitmap(); // bitmap representation of memoryu allocation

    void mergeHoles(MemoryBlock *block); // merges adjacent free memory blocks

    void setAllocator(std::function<int(int, void *)> allocator);

    int dumpMemoryMap(char *filename);

    unsigned getWordSize();

    void *getMemoryStart();

    unsigned getMemoryLimit();

private:
    unsigned wordSize_;
    size_t sizeInWords_;
    void *memoryStart_; // start address of memory block
    std::function<int(int, void *)> allocator_;
    MemoryBlock *blocksHead; // pointer to head of the memory block
};

int bestFit(int sizeInWords, void *list);

int worstFit(int sizeInWords, void *list);


#endif // MEMORY_MANAGER_H

