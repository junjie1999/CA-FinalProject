#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 4096 * 2
#define CAPACITY_THRESHOLD 4096 * 16
#define LOAD_FACTOR_THRESHOLD 0.9

const char *SEP = "******************************";

int hit = 0;
int insertion = 0;

// -------------------- Structs --------------------

typedef struct {
    int *data;
    int size;
    int capacity;
} IntArray;

typedef struct Entry {
    char *key;
    IntArray value;
    struct Entry *next;
} Entry;

typedef struct HashMap HashMap;

struct HashMap {
    HashMap **Maps;

    Entry **buckets;
    int size;       // number of entries
    int capacity;   // number of buckets
};

// -------------------- IntArray Functions --------------------

void init_int_array(IntArray *arr) {
    arr->size = 0;
    arr->capacity = 4;
    arr->data = malloc(arr->capacity * sizeof(int));
}

void add_to_int_array(IntArray *arr, int value) {
    // prevent duplication
    for (int i = 0; i < arr->size; i++)
        if (arr->data[i] == value) return;
    if (arr->size >= arr->capacity) {
        arr->capacity += 4;
        arr->data = realloc(arr->data, arr->capacity * sizeof(int));
    }
    arr->data[arr->size++] = value;
}

// -------------------- Hash Functions --------------------

uint32_t char_to_bits(char c) {
    switch (c) {
        case 'A': return 0;
        case 'C': return 1;
        case 'G': return 2;
        default:
            return 3;
    }
}

unsigned int hash(const char *str, int capacity) {
    /** /
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash % capacity;
    /**/
    unsigned long hash = 0;
    int c;
    while ((c = *str++))
        hash = (hash << 2) | char_to_bits(c);
    return hash % capacity;
    /**/
}

// -------------------- HashMap Functions --------------------

char **prefix_table;

int get_prefix_idx(const char *data) {
    int ret = 0;
    for (int i = 0; i < 4; i++) {
        ret = ret << 2; //multiply 4
        switch (data[i]) {
            case 'A': ret += 1; break;
            case 'C': ret += 2; break;
            case 'T': ret += 3; break;
            default:;
        }
        //printf("%d ", ret);
    }
    //printf("\n");
    return ret;
}

HashMap* create_prefix_hashmap(bool use_prefix) {
    HashMap *map = malloc(sizeof(HashMap));
    if (use_prefix) {
        printf("using prefix.\n");
        prefix_table = calloc(256, sizeof(char*));
        char *bases = "ACTG";
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                for (int k = 0; k < 4; k++) {
                    for (int l = 0; l < 4; l++) {
                        char *prefix = malloc(5);
                        prefix[0] = bases[i];
                        prefix[1] = bases[j];
                        prefix[2] = bases[k];
                        prefix[3] = bases[l];
                        prefix[4] = '\0';
                        //printf("prefix id: %d\n", get_prefix_idx(prefix));
                        prefix_table[get_prefix_idx(prefix)] = prefix;
                    }
                }
            }
        }

        HashMap **Maps = calloc(256, sizeof(HashMap*));
        for (int i = 0; i < 256; i++) {
            Maps[i] = create_prefix_hashmap(false);
        }
        map->Maps = Maps;
        map->capacity = 0;
        map->size = 0;
        map->buckets = NULL;
    } else {
        map->Maps = NULL;
        map->capacity = INITIAL_CAPACITY;
        map->size = 0;
        map->buckets = calloc(map->capacity, sizeof(Entry*));
    }
    printf("create map done.\n");
    return map;
}

HashMap* create_hashmap() {
    return create_prefix_hashmap(false);
}

void free_entry_list(Entry *entry) {
    while (entry) {
        Entry *next = entry->next;
        free(entry->key);
        free(entry->value.data);
        free(entry);
        entry = next;
    }
}

void free_hashmap(HashMap *map) {
    if (map->Maps != NULL) {
        for (int i = 0; i < 256; i++) {
            free_hashmap(map->Maps[i]);
        }
    }
    for (int i = 0; i < map->capacity; i++) {
        free_entry_list(map->buckets[i]);
    }
    free(map->buckets);
    free(map);
}

// Rehashes all entries into a new bucket array
void resize_hashmap(HashMap *map) {
    printf("%s\n", SEP);
    printf("resizing hashmap...\n");

    int oldCapacity = map->capacity;
    Entry **oldBuckets = map->buckets;

    printf("old capacity: %d\n", oldCapacity);

    //map->capacity += INITIAL_CAPACITY;
    //if (oldCapacity < CAPACITY_THRESHOLD)
    //    map->capacity = oldCapacity * 2;
    //else
    //    map->capacity += CAPACITY_THRESHOLD;
    map->capacity = oldCapacity * 2;

    map->buckets = calloc(map->capacity, sizeof(Entry*));
    map->size = 0;

    int maxDepth = 0;
    int emptyEntCount = 0;
    for (int i = 0; i < oldCapacity; i++) {
        Entry *entry = oldBuckets[i];
        if (!entry) emptyEntCount++;

        int depth = 0;
        while (entry) {
            Entry *next = entry->next;

            // Reinsert key-value into new table
            unsigned int index = hash(entry->key, map->capacity);
            entry->next = map->buckets[index];
            map->buckets[index] = entry;

            map->size++;
            entry = next;

            depth++;
        }
        if (depth > maxDepth) maxDepth = depth;
    }

    printf("old hit rate: %.3lf\n", 1.0 * hit / insertion);
    printf("old empty rate: %.3lf\n", 1.0 * emptyEntCount / oldCapacity);
    printf("old max depth: %d\n", maxDepth);
    printf("new capacity: %d\n", map->capacity);
    printf("%s\n", SEP);

    free(oldBuckets);

    insertion = 0;
    hit = 0;
}

// Insert or update a key with a new value
void insert(HashMap *map, const char *key, int value) {
    if (map->Maps != NULL) {
        insert(map->Maps[ get_prefix_idx(key) ], &(key[4]), value);
        return;
    }

    double loadFactor = (double)(map->size + 1) / map->capacity;
    if (loadFactor > LOAD_FACTOR_THRESHOLD) {
        resize_hashmap(map);
    }

    unsigned int index = hash(key, map->capacity);
    Entry *entry = map->buckets[index];

    insertion++;

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            add_to_int_array(&entry->value, value);
            hit++;
            return;
        }
        entry = entry->next;
    }

    // Key not found, create new entry
    Entry *newEntry = malloc(sizeof(Entry));
    newEntry->key = strdup(key);
    init_int_array(&newEntry->value);
    add_to_int_array(&newEntry->value, value);
    newEntry->next = map->buckets[index];
    map->buckets[index] = newEntry;
    map->size++;
}

void print_values(HashMap *map, const char *key) {
    if (map->Maps != NULL) {
        //TODO
        return;
    }

    unsigned int index = hash(key, map->capacity);
    Entry *entry = map->buckets[index];

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            printf("%s : ", key);
            for (int i = 0; i < entry->value.size; i++) {
                printf("%d ", entry->value.data[i]);
            }
            printf("\n");
            return;
        }
        entry = entry->next;
    }

    printf("Key \"%s\" not found.\n", key);
}

void get_values(HashMap *map, const char *key, IntArray *values) {
    if (map->Maps != NULL) {
        //TODO
        return;
    }

    unsigned int index = hash(key, map->capacity);
    Entry *entry = map->buckets[index];

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            for (int i = 0; i < entry->value.size; i++) {
                add_to_int_array(values, entry->value.data[i]);
            }
        }
        entry = entry->next;
    }
}

void write_prefix_hashmap_to_file(HashMap *map, FILE *file, const int prefix_idx) {
    char *prefix = prefix_table[prefix_idx];
    for (int i = 0; i < map->capacity; i++) {
        Entry *entry = map->buckets[i];
        while (entry) {
            fprintf(file, "%s%s :", prefix_table[prefix_idx], entry->key);
            for (int j = 0; j < entry->value.size; j++) {
                fprintf(file, " %d", entry->value.data[j]);
            }
            fprintf(file, "\n");
            entry = entry->next;
        }
    }
}

void write_hashmap_to_file(HashMap *map, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Could not open file for writing");
        return;
    }

    if (map->Maps != NULL) {
        for (int i = 0; i < 256; i++) {
            //printf("writting %d map...\n", i);
            write_prefix_hashmap_to_file(map->Maps[i], file, i);
        }
    }

    for (int i = 0; i < map->capacity; i++) {
        Entry *entry = map->buckets[i];
        while (entry) {
            fprintf(file, "%s :", entry->key);
            for (int j = 0; j < entry->value.size; j++) {
                fprintf(file, " %d", entry->value.data[j]);
            }
            fprintf(file, "\n");
            entry = entry->next;
        }
    }

    fclose(file);
}

void read_hashmap_from_file(HashMap *map, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Could not open file for reading");
        return;
    }

    char line[10240];

    while (fgets(line, sizeof(line), file)) {
        char *key = strtok(line, " :\t\n");
        if (!key) continue;

        char *token = strtok(NULL, " \t\n");
        while (token) {
            int value = atoi(token);
            insert(map, key, value);  // Reuse existing insert logic
            token = strtok(NULL, " \t\n");
        }
    }

    fclose(file);
}
