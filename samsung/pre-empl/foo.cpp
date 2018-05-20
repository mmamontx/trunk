#include <iostream>
#include <deque>
#include <vector>

#include <memory.h>

std::vector<int> dfs(std::vector<int> *vtx, int n)
{
    std::deque<int> vqueue;
    char visited[n];
    char color[n];
    int u;
    std::vector<int> white;
    memset(visited, 0, n);
    memset(color, 0, n);
    vqueue.push_back(1);
    while (vqueue.empty() != true) {
        int x = vqueue.back();
        vqueue.pop_back();
        //printf("Popped vertex %d:\n", x);
        visited[x - 1] = true;
        for (int i = 0; i < vtx[x - 1].size(); i++) {
            u = vtx[x - 1][i];
            //printf("Vertex %d is got adjacent %d!\n", x, u);
            if (visited[u - 1]) {
                //printf("%d is visited", u);
                if (color[x - 1] == color[u - 1]) {
                    //printf(" and the colors are same!\n");
                    return white;
                } else {
                    //printf(" and the colors are different.\n");
                }
            } else {
                vqueue.push_back(u);
                color[u - 1] = color[x - 1] ? 0 : 1;
            }
        }
    }
    for (int i = 0; i < n; i++) {
        if (color[i])
            white.push_back(i + 1);
    }
    return white;
}

int main()
{
    int e, v, v1, v2;
    setbuf(stdout, 0);
    scanf("%d %d", &v, &e);
    //printf("v = %d e = %d\n", v, e);
    std::vector<int> vtx[v];
    for (int i = 0; i < e; i++) {
        scanf("%d %d", &v1, &v2);
        //printf("(%d) v1 = %d v2 = %d\n", i + 1, v1, v2);
        vtx[v1 - 1].push_back(v2);
        vtx[v2 - 1].push_back(v1);
    }
    /*for (int i = 0; i < v; i++) {
        printf("Vertex %d got:", i + 1);
        for (int j = 0; j < vtx[i].size(); j++) {
            printf(" %d", vtx[i][j]);
        }
        printf("\n");
    }*/
    std::vector<int> white = dfs(vtx, v);
    if (white.empty() != true) {
        printf("%d", white.size());
        for (int i = 0; i < white.size(); i++)
            printf(" %d", white[i]);
        printf("\n");
    } else {
        printf("-1\n");
    }
    return 0;
}

