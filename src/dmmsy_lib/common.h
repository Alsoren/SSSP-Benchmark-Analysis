#ifndef DMMSY_COMMON_H
#define DMMSY_COMMON_H

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t node_t;
typedef double weight_t;

// Infinity equivalent for double
#define WEIGHT_MAX __builtin_inf()
#define NODE_MAX ((node_t)-1)

// Edge structure
typedef struct {
    node_t v;
    weight_t w;
} Edge;

// CSR Graph structure
typedef struct {
    node_t n;
    node_t m;
    node_t *offset;   // size n + 1
    Edge *edges;      // size m
    weight_t mean_weight;
} CSRGraph;

// Fast 4-Ary Heap Node
typedef struct {
    weight_t v;
    node_t i;
} HeapNode;

// Fast 4-Ary Heap
typedef struct {
    HeapNode *nodes; // 1-indexed, size n + 1
    node_t *pos;     // size n
    node_t *dirty;   // size n
    node_t max_size;
} Fast4AryHeap;

// Algorithm parameters structure
typedef struct {
    node_t k;
    node_t t;
} Params;

// Function prototypes
Params get_params(node_t n);

// Heap functions
void init_heap(Fast4AryHeap *h, node_t n);
void free_heap(Fast4AryHeap *h);
void push_dec(Fast4AryHeap *h, node_t *sz, node_t *dcnt, node_t n, weight_t d);
void pop_min(Fast4AryHeap *h, node_t *sz, weight_t *mv, node_t *mn);

// Graph Generation
CSRGraph random_graph(node_t n, node_t m, weight_t max_w);
void free_graph(CSRGraph *g);

#endif // DMMSY_COMMON_H
