#include "MemoryManager.h"
#include <cstring>
#include <iostream>
#include <limits>
#include <fcntl.h>
#include <unistd.h>

/*
 *
 *
 *          Designed by Ethan Wilson
 *                  3/22/2024
 *                  Version 1.0
 *
 */

// Constructor
MemoryManager::MemoryManager(unsigned wordSize, std::function<int(int, void *)> allocator)
        : wordSize_(wordSize), sizeInWords_(0), memoryStart_(nullptr), allocator_(allocator), blocksHead(nullptr) {
}

// Destructor
MemoryManager::~MemoryManager() {
    shutdown(); // cleanup of allocated memory on destruction
}

// Initializes the memory block
void MemoryManager::initialize(size_t sizeInWords) {
    shutdown(); // clean up any memory blocks

    sizeInWords_ = sizeInWords; // set the size of the new memory block in words
    memoryStart_ = new char[sizeInWords_ * wordSize_]; // allocate the memory block itself
    std::memset(memoryStart_, 0, sizeInWords_ * wordSize_); // initialize the memory block to zeros

    // Creates one big hole representing the entire memory block
    blocksHead = new MemoryBlock(0, sizeInWords, true); // True means this block is free aka a hole
}

// Shuts down the memory manager, releasing the memory block
void MemoryManager::shutdown() {
    delete[] static_cast<char *>(memoryStart_); // Safely deallocate the memory block
    memoryStart_ = nullptr; // Prevent dangling pointer
    sizeInWords_ = 0; // Reset the size of the memory block

    // Clean up the linked list of memory blocks
    MemoryBlock *current = blocksHead;
    while (current != nullptr) {
        MemoryBlock *next = current->next;
        delete current;
        current = next;
    }
    blocksHead = nullptr;
}

void *MemoryManager::allocate(size_t sizeInBytes) {
    if (!memoryStart_ || sizeInBytes == 0) { // if there's no initial memory block or input size is 0
        return nullptr;
    }

    size_t requiredWords = (sizeInBytes + wordSize_ - 1) / wordSize_; // rounds up to the nearest word size
    void *list = getList(); // grabs the list of holes in the required format
    if (list == nullptr) {
        return nullptr;
    }

    int startWord = allocator_(requiredWords, list); // Use the allocator function
    delete[] static_cast<uint16_t *>(list); // cleans up the list

    if (startWord == -1) {
        return nullptr;
    }

    MemoryBlock *current = blocksHead;
    while (current != nullptr) {
        if (current->isFree && current->start <= startWord &&
            (current->start + current->size) >= (startWord + requiredWords)) {
            // if the hole is larger than required, split it into two blocks.
            if (current->size > requiredWords) {
                MemoryBlock *newBlock = new MemoryBlock(current->start + requiredWords, current->size - requiredWords,
                                                        true, current->next, current);
                if (current->next) {
                    current->next->prev = newBlock;
                }
                current->next = newBlock;
                current->size = requiredWords; // adjust the size of the current block to match the allocation.
            }
            current->isFree = false; // mark the current block as allocated
            return static_cast<char *>(memoryStart_) + (startWord * wordSize_); // return the allocated memory address
        }
        current = current->next;
    }
    std::cout << "NOOOOOOOOOO!!!?!?!?!" << std::endl;
    return nullptr; // this line should never be reached if everything goes as planned :|
}

void MemoryManager::free(void *address) {
    if (!address || !memoryStart_) {
        return;
    }

    size_t offset = static_cast<char *>(address) - static_cast<char *>(memoryStart_);

    bool found = false;

    for (MemoryBlock *current = blocksHead; current; current = current->next) {
        size_t blockStart = current->start * wordSize_;
        size_t blockEnd = blockStart + (current->size * wordSize_);

        if (!current->isFree && offset >= blockStart && offset < blockEnd) {
            current->isFree = true;
            mergeHoles(current);
            found = true;
            break;
        }
    }

    if (!found) {
        // no block found at the given address
    }
}

void MemoryManager::mergeHoles(MemoryBlock *block) {
    // merge with previous block if it's free
    if (block->prev && block->prev->isFree) {
        block->prev->size += block->size;
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
        delete block; // delete the current block after merge
        block = nullptr; // blocks out deleted bloc

        return; // exit the function to avoid double deletion or invalid access aka seg faults
    }

    // merge with next block if it's free
    if (block && block->next && block->next->isFree) {
        block->size += block->next->size;
        MemoryBlock *toDelete = block->next;
        block->next = toDelete->next;
        if (toDelete->next) {
            toDelete->next->prev = block;
        }
        delete toDelete; // delete the next block after the merge

    }
}

void MemoryManager::setAllocator(std::function<int(int, void *)> allocator) {
    allocator_ = std::move(allocator);
}

//Needs implementation
int MemoryManager::dumpMemoryMap(char *filename) {
    // Open the file with write-only access, create it if it doesn't exist, and truncate it to zero length if it does exist
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        // open() failed
        return -1;
    }

    MemoryBlock *current = blocksHead;
    bool firstBlock = true;
    while (current != nullptr) {
        if (current->isFree) {
            char buffer[256]; // Buffer to hold the string representation of the memory block
            int length;
            if (!firstBlock) {
                // Write separator between blocks
                write(fd, " - ", 3);
            }
            // Write the memory block information to the buffer
            length = snprintf(buffer, sizeof(buffer), "[%zu, %zu]", current->start, current->size);
            if (length > 0) {
                // Write the buffer to the file
                write(fd, buffer, length);
            }
            firstBlock = false;
        }
        current = current->next;
    }

    // Close the file
    close(fd);
    return 0; // Success
}
void *MemoryManager::getList() {
    // counts # of free memory blocks
    size_t numberOfHoles = 0;
    MemoryBlock *current = blocksHead;

    while (current) {
        if (current->isFree) {
            ++numberOfHoles;
        }
        current = current->next;
    }
    // if there is no free blocks
    if (numberOfHoles == 0) {
        return nullptr;
    }
    // create a list of free memory blocks
    auto *list = new uint16_t[1 + 2 * numberOfHoles];
    list[0] = static_cast<uint16_t>(numberOfHoles);

    // fill the list with the start and size of each free block
    size_t index = 1;
    current = blocksHead;
    while (current) {
        if (current->isFree) {
            list[index++] = static_cast<uint16_t>(current->start);
            list[index++] = static_cast<uint16_t>(current->size);
        }
        current = current->next;
    }

    return list;
}

void *MemoryManager::getBitmap() {
    //calculate the total bytes needed for the bitmap  including the size bytes
    size_t bitmapByteSize = (sizeInWords_ + 7) / 8;
    uint8_t *bitmap = new uint8_t[2 + bitmapByteSize](); // +2 for size in little endian

    // set the bitmap size in the first two bytes in little endian
    bitmap[0] = static_cast<uint8_t>(bitmapByteSize & 0xFF);
    bitmap[1] = static_cast<uint8_t>((bitmapByteSize >> 8) & 0xFF);

    // iterate through the memory blocks to set the bits in the bitmap
    MemoryBlock *current = blocksHead;
    while (current) {
        if (!current->isFree) { // if the block is allocated
            for (size_t word = current->start; word < current->start + current->size; ++word) {
                size_t byteIndex = 2 + (word / 8); // +2 for the size bytes
                size_t bitPosition = word % 8;
                bitmap[byteIndex] |= (1 << bitPosition); // set the bit to indicate allocation
            }
        }
        current = current->next;
    }

    return bitmap;
}

unsigned MemoryManager::getWordSize() {
    return wordSize_;
}

void *MemoryManager::getMemoryStart() {
    return memoryStart_;
}

unsigned MemoryManager::getMemoryLimit() {
    return sizeInWords_ * wordSize_;
}

int bestFit(int sizeInWords, void *list) {
    uint16_t *holeList = static_cast<uint16_t *>(list);
    uint16_t numberOfHoles = holeList[0];
    int bestStart = -1;
    size_t smallestSize = std::numeric_limits<size_t>::max();

    for (int i = 0; i < numberOfHoles; i++) {
        size_t start = holeList[1 + i * 2];
        size_t size = holeList[2 + i * 2];
        if (size >= sizeInWords && size < smallestSize) {
            smallestSize = size;
            bestStart = start;
        }
    }

    return bestStart;
}

int worstFit(int sizeInWords, void *list) {
    uint16_t *holeList = static_cast<uint16_t *>(list);
    uint16_t numberOfHoles = holeList[0];
    int worstStart = -1;
    size_t largestSize = 0;

    for (int i = 0; i < numberOfHoles; i++) {
        size_t start = holeList[1 + i * 2];
        size_t size = holeList[2 + i * 2];
        if (size >= sizeInWords && size > largestSize) {
            largestSize = size;
            worstStart = start;
        }
    }

    return worstStart;
}

