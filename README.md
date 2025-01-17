# Memory Manager

## Overview
The `MemoryManager` is a custom implementation of a dynamic memory management system. It simulates memory allocation, deallocation, and memory block management with support for different allocation strategies like **Best Fit** and **Worst Fit**.

## Features
- **Dynamic Memory Allocation**:
    - Allocate and free memory blocks dynamically.
    - Merge adjacent free blocks to reduce fragmentation.

- **Allocation Strategies**:
    - **Best Fit**: Allocates from the smallest free block that fits the request.
    - **Worst Fit**: Allocates from the largest free block available.

- **Memory State Tracking**:
    - Bitmap representation of allocated/free memory.
    - List of free blocks with their start and size.
    - Memory map dump to a file for debugging.

---

## Key Methods

### Initialization and Shutdown
- `initialize(sizeInWords)`  
  Initializes a memory block with the specified size in words.

- `shutdown()`  
  Releases all allocated memory and resets the memory manager.

### Allocation and Deallocation
- `allocate(sizeInBytes)`  
  Allocates a block of memory of the given size in bytes.

- `free(address)`  
  Frees the memory block at the specified address.

### State Inspection
- `getBitmap()`  
  Returns a bitmap showing allocated and free memory.

- `getList()`  
  Returns a list of free memory blocks.

- `dumpMemoryMap(char* filename)`  
  Dumps the current memory map to a specified file.

---

## Example Usage

```cpp
#include "MemoryManager.h"
#include <iostream>

int main() {
    unsigned int wordSize = 8; // 8 bytes per word
    size_t memorySize = 100;  // 100 words

    // Initialize memory manager with Best Fit strategy
    MemoryManager memoryManager(wordSize, bestFit);
    memoryManager.initialize(memorySize);

    // Allocate memory blocks
    void* block1 = memoryManager.allocate(32); // Allocate 32 bytes
    void* block2 = memoryManager.allocate(16); // Allocate 16 bytes

    // Free a block
    memoryManager.free(block1);

    // Dump memory map to a file
    memoryManager.dumpMemoryMap("memory_map.txt");

    // Shut down the memory manager
    memoryManager.shutdown();

    return 0;
}
```

## Thanks for reading.