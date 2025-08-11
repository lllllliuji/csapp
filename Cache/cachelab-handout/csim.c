#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cachelab.h"

unsigned long long TFLAG;
unsigned long long SFLAG;
unsigned long long BFLAG;

int v;
int s, e, b;
int hits, misses, evictions;

struct CacheLine {
    char* block;
    unsigned len;
    int valid;
    unsigned long long tag;
    struct CacheLine* prev;
    struct CacheLine* next;
};

struct Set {
    struct CacheLine head;
    struct CacheLine tail;
};

struct Cache {
    struct Set* sets;
} cache;

void insertHead(struct Set* set, struct CacheLine* cacheLine) {
    cacheLine->prev = &set->head;
    cacheLine->next = set->head.next;
    set->head.next->prev = cacheLine;
    set->head.next = cacheLine;
}

void removeCacheLine(struct CacheLine* cacheLine) {
    cacheLine->prev->next = cacheLine->next;
    cacheLine->next->prev = cacheLine->prev;
    cacheLine->prev = cacheLine->next = NULL;
}

void accessAddr(struct Cache* cache, unsigned long long addr) {
    unsigned long long tag = (addr & TFLAG) >> (s + b);
    unsigned long long setIndex = (addr & SFLAG) >> b;
    unsigned long long blockOffset = addr & BFLAG;
    printf("tag %llu setIndex %llu blockOffset %llu\n", tag, setIndex, blockOffset);

    int found = 0;
    int evict = 0;
    struct Set* set = &cache->sets[setIndex];
    struct CacheLine* targetCacheLine = NULL;
    for (struct CacheLine* cacheLine = set->head.next; cacheLine != &set->tail; cacheLine = cacheLine->next) {
        // fast fail
        if (!cacheLine->valid) {
            break;
        }
        if (cacheLine->tag == tag) {
            found = 1;
            targetCacheLine = cacheLine;
            break;
        }
    }
    if (found) {
        hits += 1;
    } else {
        misses += 1;
        // need eviction ?
        if (set->tail.prev->valid) {
            evict = 1;
            evictions += 1;
        }
        // choose the tail sentinel's prev
        targetCacheLine = set->tail.prev;
        targetCacheLine->valid = 1;
        targetCacheLine->tag = tag;
    }
    printf("found %d eviction %d\n", found, evict);
    removeCacheLine(targetCacheLine);
    insertHead(set, targetCacheLine);
}

void initCache(struct Cache* cache, int s, int e, int b) {
    BFLAG = (1 << b) - 1;
    SFLAG = ((1 << (b + s)) - 1) ^ BFLAG;
    TFLAG = ~0 ^ BFLAG ^ SFLAG;
    cache->sets = (struct Set*)malloc(sizeof(struct Set) * (1 << s));
    for (int i = 0; i < (1 << s); i++) {
        cache->sets[i].head.prev = NULL;
        cache->sets[i].head.next = &cache->sets[i].tail;
        cache->sets[i].tail.prev = &cache->sets[i].head;
        cache->sets[i].tail.next = NULL;
        for (int j = 0; j < e; j++) {
            struct CacheLine* cacheLine = (struct CacheLine*)malloc(sizeof(struct CacheLine));
            cacheLine->valid = 0;
            cacheLine->len = 1 << b;
            cacheLine->block = (char*)malloc(sizeof(char) * (cacheLine->len));
            insertHead(&cache->sets[i], cacheLine);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 9) {
        fprintf(stderr, "csim: Incorrect number of arguments %d\n", argc);
        return -1;
    }
    char* filePath;
    // argv[0] is the program name, so we start at index 1
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            v = 1;
        }
        if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            s = atoi(argv[i + 1]);
            i++;
        }
        if (strcmp(argv[i], "-E") == 0 && i + 1 < argc) {
            e = atoi(argv[i + 1]);
            i++;
        }
        if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            b = atoi(argv[i + 1]);
            i++;
        }
        if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            filePath = argv[i + 1];
            i++;
        }
    }
    printf("v: %d s: %d e: %d b: %d filePath: %s\n", v, s, e, b, filePath);
    // init cache
    initCache(&cache, s, e, b);

    for (int i = 0; i < 1 << s; i++) {
        printf("Set %d\n", i);
        for (struct CacheLine* cacheLine = cache.sets[i].head.next; cacheLine != &cache.sets[i].tail;
             cacheLine = cacheLine->next) {
                printf("valid %d tag %llu\n", cacheLine->valid, cacheLine->tag);
        }
    }

    FILE* file;
    file = fopen(filePath, "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    char op;
    unsigned long long addr;
    char hexStr[16];
    char line[64];
    while (fgets(line, sizeof line, file) != NULL) {
        if (line[0] == 'I') continue;
        sscanf(line + 1, "%c %[^,]", &op, hexStr);
        addr = strtoull(hexStr, NULL, 16);
        printf("%c %llu\n", op, addr);
        switch (op) {
            // load
            case 'L':
                accessAddr(&cache, addr);
                break;
            // store
            case 'S':
                accessAddr(&cache, addr);
                break;
            // load and store
            case 'M':
                accessAddr(&cache, addr);
                accessAddr(&cache, addr);
                break;
            default:
                break;
        }
    }
    fclose(file);

    printSummary(hits, misses, evictions);
    return 0;
}
