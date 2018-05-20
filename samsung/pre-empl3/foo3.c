#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <sys/param.h>

struct vector {
    int *v;
    int n;
    int size;
};

int vector_push_back(struct vector *v, int value)
{
    if (v->v == NULL) {
        v->v = malloc(sizeof(int) * 2);
        if (v->v != NULL) {
            v->size = 2;
            v->n = 1;
            v->v[0] = value;
            return 0;
        } else {
            printf("malloc() returned NULL\n");
            return -1;
        }
    }

    if ((v->n + 1) <= v->size) {
        v->v[v->n] = value;
        v->n += 1;
    } else {
        v->v = realloc(v->v, v->size * 2 * sizeof(int));
        if (v->v != NULL) {
            v->size *= 2;
            v->v[v->n] = value;
            v->n += 1;
        } else {
            printf("realloc() returned NULL\n");
            return -1;
        }
    }
    return 0;
}

void vector_rmbyval(struct vector *v, int value)
{
    for (int i = 0; i < v->n; i++)
        if (v->v[i] == value) {
            v->v[i] = v->v[v->n - 1];
            v->n -= 1;
            i--;
        }
}

void vector_copy(struct vector *newv, struct vector *v)
{
    if (v->size) {
        newv->v = malloc(sizeof(int) * v->size);
        if (newv->v == NULL) {
            printf("malloc() returned NULL\n");
            return;
        }
        memcpy(newv->v, v->v, v->size * sizeof(int));
        newv->size = v->size;
    }
    newv->n = v->n;
}

struct edge {
    int v1;
    int v2;
    int w;
};

int fwalg(struct edge *e, int nedges)
{
    int nvertices = nedges + 1;
    int W[nvertices][nvertices];

    for (int i = 0; i < nvertices; i++)
        for (int j = 0; j < nvertices; j++)
            W[i][j] = INT_MAX / 2;

    for (int i = 0; i < nvertices; i++)
        W[i][i] = 0;

    for (int i = 0; i < nedges; i++) {
        W[e[i].v1 - 1][e[i].v2 - 1] = e[i].w;
        W[e[i].v2 - 1][e[i].v1 - 1] = e[i].w;
    }

    for (int k = 0; k < nvertices; k++)
        for (int i = 0; i < nvertices; i++)
            for (int j = 0; j < nvertices; j++)
                W[i][j] = MIN(W[i][j], W[i][k] + W[k][j]);

    int max = 0;
    for (int i = 0; i < nvertices; i++)
        for (int j = 0; j < nvertices; j++)
            if ((W[i][j] != (INT_MAX / 2)) && (W[i][j] > max))
                max = W[i][j];

    return max;
}

struct edge *rmedge(struct edge *e, int nedges, int eid)
{
    struct edge *newe = malloc(sizeof(struct edge) * nedges);
    memcpy(newe, e, sizeof(struct edge) * nedges);
    memcpy(newe + eid, newe + (nedges - 1), sizeof(struct edge));
    return newe;
}

void addedge(struct edge *e, int nedges, struct edge ex)
{
    memcpy(e + nedges, &ex, sizeof(struct edge));
}

char *explore(int i, char *visited, struct vector *vtx)
{
    visited[i] = 1;
    for (int j = 0; j < vtx[i].n; j++) {
        if (visited[vtx[i].v[j] - 1] == 0)
            explore(vtx[i].v[j] - 1, visited, vtx);
    }
    return visited;
}

int mainloop(struct edge *e, int nedges)
{
    int nvertices = nedges + 1;
    struct vector *vtx = calloc(nvertices, sizeof(struct vector));
    if (vtx == NULL) {
        printf("malloc() returned NULL\n");
        return -1;
    }
    memset(vtx, 0, sizeof(vtx));

    for (int i = 0; i < nedges; i++) {
        vector_push_back(&(vtx[e[i].v1 - 1]), e[i].v2);
        vector_push_back(&(vtx[e[i].v2 - 1]), e[i].v1);
    }

    /*for (int i = 0; i < nvertices; i++) {
        printf("(%d)", i + 1);
        for (int j = 0; j < vtx[i].n; j++)
            printf(" %d", vtx[i].v[j]);
        printf("\n");
    }*/

    int max = 0;
    for (int i = 0; i < nedges; i++) {
        struct edge *newe = rmedge(e, nedges, i);
        int n = nedges - 1;/* n is the number of edges in the new graph */
        /*for (int j = 0; j < n; j++) {
            printf("(%d,%d) ", newe[j].v1, newe[j].v2);
        }
        printf("\n");*/

        if ((vtx[e[i].v1 - 1].n > 1) && (vtx[e[i].v2 - 1].n > 1)) { /* The edge is middle */
            //printf("Got middle edge that is %d to %d.\n", e[i].v1, e[i].v2);
            struct vector *newvtx;
            newvtx = calloc(nvertices, sizeof(struct vector));
            if (newvtx == NULL) {
                printf("calloc() returned NULL\n");
                return -1;
            }
            //memcpy(newvtx, vtx, sizeof(struct vector) * nvertices);
            for (int j = 0; j < nvertices; j++)
                vector_copy(&(newvtx[j]), &(vtx[j]));

            /*printf("New siblings are:\n");
            for (int j = 0; j < nvertices; j++) {
                printf("(%d)", j + 1);
                for (int k = 0; k < newvtx[j].n; k++)
                    printf(" %d", newvtx[j].v[k]);
                printf("\n");
            }*/

            vector_rmbyval(&(newvtx[e[i].v1 - 1]), e[i].v2);
            vector_rmbyval(&(newvtx[e[i].v2 - 1]), e[i].v1);

            /*printf("!New siblings are:\n");
            for (int j = 0; j < nvertices; j++) {
                printf("(%d)", j + 1);
                for (int k = 0; k < newvtx[j].n; k++)
                    printf(" %d", newvtx[j].v[k]);
                printf("\n");
            }*/

            char *visited1;
            visited1 = calloc(nvertices, sizeof(char));
            if (visited1 == NULL) {
                printf("calloc() returned NULL\n");
                return -1;
            }

            explore(e[i].v1 - 1, visited1, newvtx);

            /*printf("First part is:");
            for (int j = 0; j < nvertices; j++)
                printf(" %d", visited1[j]);
            printf("\n");*/

            char *visited2;
            visited2 = calloc(nvertices, sizeof(char));
            if (visited1 == NULL) {
                printf("calloc() returned NULL\n");
                return -1;
            }

            explore(e[i].v2 - 1, visited2, newvtx);

            /*printf("Second part is:");
            for (int j = 0; j < nvertices; j++)
                printf(" %d", visited2[j]);
            printf("\n");*/

            for (int j = 0; j < nvertices; j++)
                if (visited1[j])
                    for (int k = 0; k < nvertices; k++)
                        if (visited2[k]) { /* TODO: Add restrictions */
                            struct edge ex;
                            ex.v1 = j + 1;
                            ex.v2 = k + 1;
                            ex.w = e[i].w;
                            addedge(newe, n, ex);
                            int rv = fwalg(newe, n + 1);
                            if (rv > max)
                                max = rv;
                            //printf("New max is %d.\n", rv);
                        }

            free(newvtx);
            free(visited1);
            free(visited2);
        } else { /* The edge is not middle */
            if (vtx[e[i].v1 - 1].n > 1) {
                for (int j = 0; j < nvertices; j++) {
                    if ((j != (e[i].v1 - 1)) && (j != (e[i].v2 - 1))) {
                        struct edge ex;
                        ex.v1 = j + 1;
                        ex.v2 = e[i].v2;
                        ex.w = e[i].w;
                        addedge(newe, n, ex);
                        int rv = fwalg(newe, n + 1);
                        //printf("New max is %d.\n", rv);
                        if (rv > max)
                            max = rv;
                    }
                }
            } else if (vtx[e[i].v2 - 1].n > 1) {
                for (int j = 0; j < nvertices; j++) {
                    if ((j != (e[i].v1 - 1)) && (j != (e[i].v2 - 1))) {
                        struct edge ex;
                        ex.v1 = e[i].v1;
                        ex.v2 = j + 1;
                        ex.w = e[i].w;
                        addedge(newe, n, ex);
                        int rv = fwalg(newe, n + 1);
                        //printf("New max is %d.\n", rv);
                        if (rv > max)
                            max = rv;
                    }
                }
            } else {
                /* The only edge? */
                /* TODO */
            }
        }
    }
    return max;
}

void solve()
{
    int nedges;
    scanf("%d", &nedges);
    nedges--;
    //printf("There are %d edges.\n", nedges);
    struct edge *e = calloc(nedges, sizeof(struct edge));
    if (e == NULL) {
        printf("calloc() returned NULL\n");
        return;
    }
    for (int i = 0; i < nedges; i++)
        scanf("%d %d %d", &(e[i].w), &(e[i].v1), &(e[i].v2));

    int max = fwalg(e, nedges);
    //printf("The original maximum is %d.\n", max);

    int rv = mainloop(e, nedges);
    max = MAX(max, rv);
    //printf("The new maximum is %d.\n", rv);
    printf("%d\n", max);
}

int main()
{
    int ntests;
    scanf("%d", &ntests);
    //printf("There are %d tests.\n", ntests);

    for (int i = 0; i < ntests; i++) {
        printf("#%d ", i + 1);
        solve();
    }

    return 0;
}

