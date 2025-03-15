#pragma once
#include "util.h"
#include <atomic>
#include <cmath>
#include <cassert>
#include <random>
using namespace std;

class AlgorithmD {
private:
    enum {
        MARKED_MASK = (int) 0x80000000,     // most significant bit of a 32-bit key
        TOMBSTONE = (int) 0x7FFFFFFF,       // largest value that doesn't use bit MARKED_MASK
        EMPTY = (int) 0
    }; // with these definitions, the largest "real" key we allow in the table is 0x7FFFFFFE, and the smallest is 1 !!

    struct table {
        // data types
        char padding0[PADDING_BYTES];

        atomic<int>* data;
        atomic<int>* old;
        int capacity;
        int oldCapacity;
        // count the number of keys in the set
        counter* approxCounter;
        // count the number of tombstones in the set
        counter* tombCounter;
        atomic<int> chunksClaimed;
        atomic<int> chunksDone;

        char padding2[PADDING_BYTES];

        // constructor
        table(int _numThreads, int _capacity) {
            data = new atomic<int>[_capacity];
            capacity = _capacity;

            for(int i = 0; i < capacity; i++) {
                data[i].store(EMPTY, memory_order_relaxed);
            }

            old = NULL;
            oldCapacity = 0;
            chunksClaimed = 0;
            chunksDone = 0;
            approxCounter = new counter(_numThreads);
            tombCounter = new counter(_numThreads);
        }
        
        // destructor
        ~table() {
            delete tombCounter;
            delete approxCounter;
            delete[] data;
        }
    };
    
    bool expandAsNeeded(const int tid, table * t, int i);
    void helpExpansion(const int tid, table * t);
    void startExpansion(const int tid, table * t, int newCapacity);
    void migrate(const int tid, table * t, int myChunk);
    table* createNewTableStruct(const int tid, table* t, int newCapacity);

    template<typename T>
    inline bool CAS(atomic<T>* ptr, T oldVal, T newVal) {
        return (std::atomic_compare_exchange_weak(ptr, &oldVal, newVal)); 
    }
    
    char padding0[PADDING_BYTES];

    int numThreads;
    int initCapacity;
    
#if defined MULTI_START_EXPANSION
#warning MULTI_START_EXPANSION is defined
    atomic<table*> currentTable;
#else
    table* currentTable;
#endif

    char padding2[PADDING_BYTES];
    
public:
    AlgorithmD(const int _numThreads, const int _capacity);
    ~AlgorithmD();
    bool insertIfAbsent(const int tid, const int & key, bool disableExpansion);
    bool erase(const int tid, const int & key);
    long getSumOfKeys();
    void printDebuggingDetails(); 
};

/**
 * constructor: initialize the hash table's internals
 * 
 * @param _numThreads maximum number of threads that will ever use the hash table (i.e., at least tid+1, where tid is the largest thread ID passed to any function of this class)
 * @param _capacity is the INITIAL size of the hash table (maximum number of elements it can contain WITHOUT expansion)
 */
AlgorithmD::AlgorithmD(const int _numThreads, const int _capacity)
: numThreads(_numThreads), initCapacity(_capacity) {
    currentTable = new table(_numThreads, _capacity);
}

// destructor: clean up any allocated memory, etc.
AlgorithmD::~AlgorithmD() {
    delete currentTable;
}

AlgorithmD::table* AlgorithmD::createNewTableStruct(const int tid, table* t, int newCapacity) {
    table* tNew = new table(numThreads, newCapacity);

    tNew->old = t->data;
    tNew->oldCapacity = t->capacity;

    return tNew;
}

bool AlgorithmD::expandAsNeeded(const int tid, table * t, int i) {
    helpExpansion(tid, t);

#if not defined MULTI_START_EXPANSION
    // to avoid creating tables and deleting them right away
    // due to a contention, we only allow the first thread to
    // start the expansion; other threads only *help* in this process
    if(tid != 0) {
        return false;
    }
#endif

    const int approxTombSize = t->tombCounter->get();
    const int approxSize = t->approxCounter->get() - approxTombSize;

    // the threshold to check if we need to expand the table
    const int expandThresh = t->capacity / 2;
    // the threshold to check if we need to shrink the table
    const int shrinkThresh = t->capacity / 8;
    // the threshold to check if we need to just start the expansion
    // with the almost same size to remove tombstones and hopefully
    // redistribute keys
    const int tombThresh = t->capacity / 4;

    // the new capacity after the expansion
    const int expandCapacity = t->capacity * 4;
    // the new capacity after the shrinkage
    const int shrinkCapacity = t->capacity / 2;
    // the new capacity after redistribution
    const int steadyCapacity = t->capacity + 100;

    // expand if threshold is reached
    if(approxSize > expandThresh) {
        startExpansion(tid, t, expandCapacity);

        return true;
    }

    // too much probing, safe to calculate the actual size
    // as we suffer poor performance due to the probing anyway
    if(i > 100) {
        int size = t->approxCounter->getAccurate() - t->tombCounter->getAccurate();

        // expand if the real size has reached the threshold
        if(size > expandThresh) {
            startExpansion(tid, t, expandCapacity);

            return true;
        }

        // we should be really careful while shrinking the table.
        // If the size was just an approximation, we might shrink too early
        // and lose some of our keys. Always shrink if the real size
        // meets the requirement
        if(initCapacity < shrinkCapacity 
            && size > 0 && size < shrinkThresh) {
            startExpansion(tid, t, shrinkCapacity);

            return true;
        }
    }

    // too many tombstones, start the expansion to remove them
    // and redistribute all the keys
    if(approxTombSize > tombThresh) {
        startExpansion(tid, t, steadyCapacity);

        return true;
    }

    return false;
}

void AlgorithmD::helpExpansion(const int tid, table * t) {
    int totalOldChunks = ceil(double(t->oldCapacity) / 4096.0);

    while(t->chunksClaimed < totalOldChunks) {
        int myChunk = t->chunksClaimed++;
        
        if(myChunk < totalOldChunks) {
            migrate(tid, t, myChunk);

            t->chunksDone++;
        }
    }

    while(t->chunksDone < totalOldChunks) {}
}

void AlgorithmD::startExpansion(const int tid, table * t, int newCapacity) {
#if defined MULTI_START_EXPANSION
    if(currentTable == t) {
        table* tNew = createNewTableStruct(tid, t, newCapacity);

        if(!CAS<table*>(&currentTable, t, tNew)) {
            delete tNew;
        } 
    }
#else
    table* tNew = createNewTableStruct(tid, t, newCapacity);

    // only one thread can start the expansion; no need
    // for CASing the new table into the current table
    currentTable = tNew;
#endif
    
    helpExpansion(tid, currentTable);
}

void AlgorithmD::migrate(const int tid, table * t, int myChunk) {
    int startIndex = myChunk * 4096;
    int endIndex = min(startIndex + 4096, t->oldCapacity);

    for(int i = startIndex; i < endIndex; i++) {
        int found = t->old[i];

        if(found == TOMBSTONE) {            
            continue;
        }

        // no other thread should claim my chunk
        if(found & MARKED_MASK) {
            assert(false);
        }

        int masked = found | MARKED_MASK;

        if(CAS<int>(&t->old[i], found, masked) && found != EMPTY) {
            // should always succeed as I'm the onwer of the chunk
            if(!insertIfAbsent(tid, found, true)) {
                assert(false);
            }
        }
    }
}

// semantics: try to insert key. return true if successful (if key doesn't already exist), and false otherwise
bool AlgorithmD::insertIfAbsent(const int tid, const int & key, bool disableExpansion = false) {
    table* t = currentTable;

    uint32_t h = murmur3(key);

    for(int i = 0; i < t->capacity; i++) {
        // if an expansion is going on, avoid re-expanding
        if(!disableExpansion && expandAsNeeded(tid, t, i)) {
            return insertIfAbsent(tid, key);
        }

        int index = (h + i) % t->capacity;

        int found = t->data[index];

        if(!disableExpansion && (found & MARKED_MASK)) {
            return insertIfAbsent(tid, key);
        } else if(found == key) {
            return false;
        } else if(found == EMPTY) {
            if(CAS<int>(&t->data[index], EMPTY, key)) {
                // a new key
                t->approxCounter->inc(tid);

                return true;
            } else {
                found = t->data[index];

                if (!disableExpansion && (found & MARKED_MASK)) {
                    return insertIfAbsent(tid, key);
                } else if(found == key) {
                    return false;
                }
            }
        }
    }

    return false;
}

// semantics: try to erase key. return true if successful, and false otherwise
bool AlgorithmD::erase(const int tid, const int & key) {
    table* t = currentTable;
    
    uint32_t h = murmur3(key);

    for(int i = 0; i < t->capacity; i++) {
        if(expandAsNeeded(tid, t, i)) {
            return erase(tid, key);
        }

        int index = (h + i) % t->capacity;

        int found = t->data[index];

        if(found & MARKED_MASK) {
            return erase(tid, key);
        } else if(found == EMPTY) {
            return false;
        } else if(found == key) {
            if(CAS<int>(&t->data[index], key, TOMBSTONE)) {
                // a new tombstone
                t->tombCounter->inc(tid);

                return true;
            } else {
                found = t->data[index];

                if(found & MARKED_MASK) {
                    return erase(tid, key);
                } else if(found == TOMBSTONE) {
                    return false;
                }
            }
        }
    }

    return false;
}

// semantics: return the sum of all KEYS in the set
int64_t AlgorithmD::getSumOfKeys() {
    int64_t sumOfKeys = 0;

    table* t = currentTable;

    for(int i = 0; i < t->capacity; i++) {
        if(t->data[i] == TOMBSTONE) {
            continue;
        }

        if(t->data[i] & MARKED_MASK) {
            // no expansion should be left behind
            assert(false);
        }

        sumOfKeys += t->data[i];
    }

    return sumOfKeys;
}

// print any debugging details you want at the end of a trial in this function
void AlgorithmD::printDebuggingDetails() {}
