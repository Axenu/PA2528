#include "BuddyAllocator.h"
#include <cstring>

# if defined(__WIN32) || defined(WIN32) || defined(_WIN32)
int __builtin_clz(int x) {
	unsigned y;
	int n = 32;
	y = x >> 16;  if (y != 0) { n = n - 16;  x = y; }
	y = x >> 8;  if (y != 0) { n = n - 8;  x = y; }
	y = x >> 4;  if (y != 0) { n = n - 4;  x = y; }
	y = x >> 2;  if (y != 0) { n = n - 2;  x = y; }
	y = x >> 1;  if (y != 0) return n - 2;
	return n - x;
}
#endif

//init
// blockSize: number of bytes to allocate
// size of BuddyHeader = 16 byte.
BuddyAllocator::BuddyAllocator(size_t blockSize) {

    //check if blockSize is power of two
    assert((blockSize & (blockSize - 1)) == 0);

    //make sure every empty block is nullptr
    for (int i = 0; i < 32; i++) {
        _free_lists[i] = nullptr;
    }

    _totalSize = blockSize; // in bytes
    _leaf_size = 16; // in bytes // something does not work if it is 8...
    _origin = malloc(blockSize);
    _num_levels = 32 - __builtin_clz((int)_totalSize/_leaf_size);
    int numberOfIndices = (1 << _num_levels);
	//std::cout << "allocate of level: " << builtin_clz(12) << std::endl;
//    int buddyAndSplitArraySize = numberOfIndices; // since every array requires half a bit per index.
    int sizeOfArray = numberOfIndices >> 3; //bit to byte
    _buddyArray = (int *)_origin;
    _splitArray = (int *)((char *)_origin + sizeOfArray/2);
    _allocated_size = 0;

    // set all memory to 0
    memset(_origin, 0, blockSize);

    //create first block of data of size _totalSize
    BuddyHeader *header = (BuddyHeader *)_origin;
    header->next = nullptr;
    header->prev = nullptr;
    _free_lists[0] = _origin;
//    std::cout << nearestLevel(51200) << std::endl;

    // allocate the leafblocks required by array //alloc_internal can be used since the numberOfInitialblocks is always a power of two
    alloc_internal(sizeOfArray);
    // printMemory(8);
}

void *BuddyAllocator::getBlockAtLevel(int level) {
//     std::cout << "getBlockAtLevel: " << level << std::endl;
    //if the algorithm try to allocate something larger than the whole size
    if (level < 0) {
        return nullptr;
    }
    //get block
    void *newBlock;
    if (_free_lists[level] == nullptr) { // no free blocks
        void *largeBlock = getBlockAtLevel(level-1);
		if (largeBlock == nullptr)
			return nullptr;
        newBlock = split(largeBlock, level);
        // printMemory();
    } else {
        //newBlock = first in free list
        BuddyHeader *b = (BuddyHeader *)_free_lists[level];
        //if there are a next free block in this level, add it to the list
        if (b->next != nullptr) {
            _free_lists[level] = b->next;
            BuddyHeader *ob = (BuddyHeader *)b->next;
            ob->prev = nullptr;
        } else {
            _free_lists[level] = nullptr; // There are no more free blocks at level
        }
        newBlock = b;
    }
    if (level != 0) {

        //xor
        int index = globalBuddyIndex(newBlock, level);
        // std::cout << "buddyindex: " << index << std::endl;
        if (getBit(_buddyArray, index) == 0) {
            setBit(_buddyArray, index);
        } else {
            unsetBit(_buddyArray, index);
        }
    }
    return newBlock;
}

void *BuddyAllocator::split(void *block, int level) {
    //create two new blocks with sizeOf(level) out of block
    size_t newSize = sizeOfLevel(level); // in bytes
    if (newSize < sizeof(BuddyHeader)) {
        return block;
    }
    void *rightBlock = (char *)block + newSize; // jump forward newSize number of bytes
    BuddyHeader *rightHeader = (BuddyHeader *)rightBlock;
    rightHeader->next = nullptr;
    rightHeader->prev = nullptr;
    _free_lists[level] = rightBlock;
    int index = globalSplitIndex(block, level-1); //index of block that is split, i.e. the larger block of size level+1

    //mar block as split
    setBit(_splitArray, index);
//    printf("block: %p other: %p\n", block, rightBlock);
    return block;
}


BuddyAllocator::~BuddyAllocator() {
    // std::cout << "dtor BuddyAllocator" << std::endl;

}

void* BuddyAllocator::alloc_internal(size_t size) {

    int a_size = sizeOfLevel(nearestLevel(size));
    if (a_size + _allocated_size > _totalSize) {
        return nullptr;
    }
    _allocated_size += a_size;
    void * memory = getBlockAtLevel(nearestLevel(size));
    // memset()

    return memory;
}
void BuddyAllocator::dealloc_internal(void *p) {
    int level = findLevel(p);
    //decrease size;
    int a_size = sizeOfLevel(level);
    _allocated_size -= a_size;
//     std::cout << "dealloc level: " << level << std::endl;
    // check if buddy is free:
    if (getBit(_buddyArray, globalBuddyIndex(p, level)) == 1) { // only one of the blocks was allocated, and since this one was, the other is free
        merge(p, level);
//         std::cout << "merge: " << std::endl;
    } else {
        //add this block to free array at level
//         std::cout << "store: " << std::endl;
        BuddyHeader *bh = (BuddyHeader *)p;
        BuddyHeader *obh = (BuddyHeader *)_free_lists[level];
        if (obh != nullptr) {
            obh->prev = p;
        }
        //set bh->next to _free_lists[level]
        bh->next = _free_lists[level];
        bh->prev = nullptr;
        _free_lists[level] = p;
        setBit(_buddyArray, globalBuddyIndex(p, level));
    }
}

void BuddyAllocator::merge(void *p, short level) {
    // printMemory(8);
    //might need to rewrite this chunk
    void *bptr; //should already be free. get its header
    void *smallest;
    int index = indexInLevelOf(p, level);
    if (index % 2 == 0) {
        // std::cout << "larger buddy" << std::endl;
        bptr = pointerForIndex(index + 1, level);
        smallest = p;
    } else {
        bptr = pointerForIndex(index - 1, level);
        smallest = bptr;
    }
//    printf("p: %p bptr: %p\n", p, bptr);


    //remove buddy from _free_lists

    BuddyHeader *bh = (BuddyHeader *)bptr;
    BuddyHeader *bhPrev = (BuddyHeader *)bh->prev;
    BuddyHeader *bhNext = (BuddyHeader *)bh->next;
    if (bhPrev != nullptr) {
        bhPrev->next = bhNext;
    } else {
        _free_lists[level] = bh->next;
    }
    if (bhNext != nullptr) {
        bhNext->prev = bhPrev;
    }
    //remove p from list if in a list. maybe?
//    BuddyHeader *p_header = (BuddyHeader *)p;
//    if (p_header->next != nullptr || p_header->prev != nullptr) { // since header is not null, assume it is in a list.
//        if (p_header->prev == nullptr) { // p_header is first in a list. Point list to p_header->next
//            _free_lists[level] = p_header->next;
//        }
//        if (p_header->next != nullptr) {
//            BuddyHeader *next_header = (BuddyHeader *)p_header->next;
//            next_header->prev = p_header->prev;
//        }
//    }
    //unset split
//    int gi = globalSplitIndex(smallest, level-1);
    // std::cout << gi << std::endl;
    unsetBit(_splitArray, globalSplitIndex(smallest, level-1));
    //unset allocated
//    gi = globalBuddyIndex(smallest, level);
    // std::cout << gi << std::endl;
    unsetBit(_buddyArray, globalBuddyIndex(smallest, level));
    //try if the next level can be merged:
    if (level > 1) {
        if (getBit(_buddyArray, globalBuddyIndex(smallest, level-1)) == 1) {
            merge(smallest, level-1);
        } else {
            //add smallest of p and bptr to _free_lists[level-1]
            BuddyHeader *bhSmall = (BuddyHeader *)smallest;
            BuddyHeader *bh_free = (BuddyHeader *)_free_lists[level-1];
            if (bh_free != nullptr) {
                bh_free->prev = bhSmall;
            }
            bhSmall->next = _free_lists[level-1];
            bhSmall->prev = nullptr;
            _free_lists[level-1] = smallest;
            setBit(_buddyArray, globalBuddyIndex(smallest, level-1));
        }
    }

   // memset(p, 0, sizeOfLevel(level-1));
}

int BuddyAllocator::findLevel(void *p) {
    int n = _num_levels - 1;
    while (n > 0) {
        int gi = globalSplitIndex(p, n-1);
        if (getBit(_splitArray, gi)) { // if is split
            return n;
        }
        n -= 1;
    }
    return 0;
}


int BuddyAllocator::nearestLevel(int size) {
    if (size < _leaf_size)
        size = _leaf_size;
    short level = (short)__builtin_clz(size - 1) - 29 + _num_levels;
    return level;
}

//Debug
void BuddyAllocator::printMemory(int lines) {
    std::cout << "Memory dump:" << std::endl;
    int width = 8;
    if (lines > _totalSize/width)
        lines = _totalSize/width;
    unsigned char *p = (unsigned char *)_origin;
    for (int y = 0; y < lines; y++) {
        for (int x = 0; x < width; x++) {
            printf(" ");
            for (int i = 0; i < 8; ++i) {
                printf("%d", (p[y*width+x] >> i) & 1);
            }
        }
        printf("\n");
    }

    for (int i = 0; i < _num_levels; ++i) {
        printf("%p", _free_lists[i]);
        BuddyHeader *h = (BuddyHeader *)_free_lists[i];
        while (h != nullptr) {
            h = (BuddyHeader *)h->next;
            printf("->%p", (void *)h);
        }
        printf("\n");
    }
}
// buddy
//     ---------------------------------------------------------------------------------
// 512 |                                       0                                       |
//     ---------------------------------------------------------------------------------
// 256 |                   1                   |                   2                   | 1 | 0 = 1
//     ---------------------------------------------------------------------------------
// 128 |         3         |         4         |         5         |         6         | 1|0=1 0|0=0
//     ---------------------------------------------------------------------------------
// 64  |    7    |    8    |    9    |   10    |   11    |   12    |   13    |   14    | 1000
//     ---------------------------------------------------------------------------------
// 32  | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 10000000
//     ---------------------------------------------------------------------------------
// 16 |31|32|33|34|35|36|37|38|39|40|41|42|43|44|45|46|47|48|49|50|51|52|53|54|55|56|57|58|59|60|61|62| 1000000000000000

// split
//     ---------------------------------------------------------------------------------
// 512 |                                       0                                       | 1
//     ---------------------------------------------------------------------------------
// 256 |                   1                   |                   2                   | 10
//     ---------------------------------------------------------------------------------
// 128 |         3         |         4         |         5         |         6         | 1000
//     ---------------------------------------------------------------------------------
// 64  |    7    |    8    |    9    |   10    |   11    |   12    |   13    |   14    | 10000000
//     ---------------------------------------------------------------------------------
// 32  | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 1000000000000000
//     ---------------------------------------------------------------------------------
// 16 |31|32|33|34|35|36|37|38|39|40|41|42|43|44|45|46|47|48|49|50|51|52|53|54|55|56|57|58|59|60|61|62|
