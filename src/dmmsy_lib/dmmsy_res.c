#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  weight_t *d;
  node_t *pr;
  HeapNode *h_nodes;
  node_t *h_pos;
  node_t *dirty_h;
  node_t *dirty_d;
  node_t ds_cnt;
  node_t **piv_bufs;
  node_t max_depth;
  node_t *active_buf;
  node_t n;
} ResearchWorkspace;

static __thread ResearchWorkspace *ws_res = NULL;

static ResearchWorkspace *get_research_ws(node_t n, Params p) {
  if (ws_res == NULL) {
    ws_res = (ResearchWorkspace *)malloc(sizeof(ResearchWorkspace));
    ws_res->n = n;
    ws_res->d = (weight_t *)malloc(sizeof(weight_t) * n);
    ws_res->pr = (node_t *)malloc(sizeof(node_t) * n);
    ws_res->h_nodes = (HeapNode *)malloc(sizeof(HeapNode) * (n + 1));
    ws_res->h_pos = (node_t *)calloc(n, sizeof(node_t)); // zeroed
    ws_res->dirty_h = (node_t *)malloc(sizeof(node_t) * n);
    ws_res->dirty_d = (node_t *)malloc(sizeof(node_t) * n);
    ws_res->ds_cnt = 0;
    ws_res->active_buf = (node_t *)malloc(sizeof(node_t) * n);

    ws_res->max_depth = p.t + 2;
    ws_res->piv_bufs = (node_t **)malloc(sizeof(node_t *) * ws_res->max_depth);
    node_t buf_size = p.k > 4 ? p.k : 4;

    for (node_t i = 0; i < ws_res->max_depth; i++) {
      ws_res->piv_bufs[i] = (node_t *)malloc(sizeof(node_t) * buf_size);
    }
    for (node_t i = 0; i < n; i++)
      ws_res->d[i] = WEIGHT_MAX;
  } else if (ws_res->n != n) {
    for (node_t i = 0; i < ws_res->max_depth; i++)
      free(ws_res->piv_bufs[i]);
    free(ws_res->piv_bufs);
    free(ws_res->d);
    free(ws_res->pr);
    free(ws_res->h_nodes);
    free(ws_res->h_pos);
    free(ws_res->dirty_h);
    free(ws_res->dirty_d);
    free(ws_res->active_buf);
    free(ws_res);
    ws_res = NULL;
    return get_research_ws(n, p);
  }
  return ws_res;
}

static inline void bmsp_research(CSRGraph *g, node_t *src, node_t src_len,
                                 weight_t B, node_t dp, ResearchWorkspace *ws,
                                 Params p) {
  if (dp >= p.t || src_len <= p.k) {
    memset(ws->h_pos, 0, sizeof(node_t) * g->n);
    Fast4AryHeap h;
    h.nodes = ws->h_nodes;
    h.pos = ws->h_pos;
    h.dirty = ws->dirty_h;
    h.max_size = g->n;

    node_t sz = 0, dcnt = 0;

    for (node_t i = 0; i < src_len; i++) {
      node_t s = src[i];
      push_dec(&h, &sz, &dcnt, s, ws->d[s]);
    }

    while (sz > 0) {
      weight_t du;
      node_t u;
      pop_min(&h, &sz, &du, &u);

      if (du > ws->d[u])
        continue;

      node_t si = g->offset[u], ei = g->offset[u + 1];
      for (node_t i = si; i < ei; i++) {
        node_t v = g->edges[i].v;
        weight_t w = g->edges[i].w;
        weight_t nd = du + w;

        if (nd <= ws->d[v]) {
          if (ws->d[v] == WEIGHT_MAX) {
            ws->dirty_d[ws->ds_cnt++] = v;
          }
          ws->d[v] = nd;
          ws->pr[v] = u;
          push_dec(&h, &sz, &dcnt, v, nd);
        }
      }
    }
    return;
  }

  node_t np = (src_len < p.k) ? src_len : p.k;
  node_t *pivots = ws->piv_bufs[dp + 2];
  node_t step = src_len / np;
  if (step == 0)
    step = 1;
  node_t curr_np = 0;
  node_t bound = src_len < (step * p.k) ? src_len : (step * p.k);

  for (node_t i = 0; i < bound; i += step) {
    pivots[curr_np++] = src[i];
  }

  bmsp_research(g, pivots, curr_np, B * 0.5, dp + 1, ws, p);

  memset(ws->h_pos, 0, sizeof(node_t) * g->n);
  Fast4AryHeap h;
  h.nodes = ws->h_nodes;
  h.pos = ws->h_pos;
  h.dirty = ws->dirty_h;
  h.max_size = g->n;

  node_t sz = 0, dcnt = 0;
  for (node_t i = 0; i < src_len; i++) {
    node_t s = src[i];
    if (ws->d[s] < B) {
      push_dec(&h, &sz, &dcnt, s, ws->d[s]);
    }
  }

  while (sz > 0) {
    weight_t du;
    node_t u;
    pop_min(&h, &sz, &du, &u);

    if (du > ws->d[u])
      continue;

    node_t si = g->offset[u], ei = g->offset[u + 1];
    for (node_t i = si; i < ei; i++) {
      node_t v = g->edges[i].v;
      weight_t w = g->edges[i].w;
      weight_t nd = du + w;

      if (nd <= ws->d[v]) {
        if (ws->d[v] == WEIGHT_MAX) {
          ws->dirty_d[ws->ds_cnt++] = v;
        }
        ws->d[v] = nd;
        ws->pr[v] = u;
        if (nd < B) {
          push_dec(&h, &sz, &dcnt, v, nd);
        }
      }
    }
  }
}

void ssp_duan_research(CSRGraph *g, node_t src, weight_t *d_out,
                       node_t *pr_out) {
  if (g->n == 0)
    return;

  Params p = get_params(g->n);
  ResearchWorkspace *ws = get_research_ws(g->n, p);

  if (ws->ds_cnt > (g->n >> 2)) {
    for (node_t i = 0; i < g->n; i++)
      ws->d[i] = WEIGHT_MAX;
    for (node_t i = 0; i < g->n; i++)
      ws->pr[i] = NODE_MAX;
  } else {
    for (node_t i = 0; i < ws->ds_cnt; i++) {
      node_t idx = ws->dirty_d[i];
      ws->d[idx] = WEIGHT_MAX;
      ws->pr[idx] = NODE_MAX;
    }
  }

  ws->ds_cnt = 1;
  ws->d[src] = 0.0;
  ws->dirty_d[0] = src;

  double log2_n1 = log2((double)(g->n + 1));
  weight_t B = g->mean_weight * log2_n1 * 4.0;

  ws->active_buf[0] = src;

  bmsp_research(g, ws->active_buf, 1, B, 0, ws, p);

  if (d_out)
    memcpy(d_out, ws->d, sizeof(weight_t) * g->n);
  if (pr_out)
    memcpy(pr_out, ws->pr, sizeof(node_t) * g->n);
}
