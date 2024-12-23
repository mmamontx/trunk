#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <unordered_set>
#include <vector>

struct pair_hash {
    inline std::size_t operator()(const std::pair<int, int> &p) const
    {
        return p.first * 31 + p.second;
    }
};

bool unordered_set_intersection(std::unordered_set<std::pair<int, int>, pair_hash> s1,
                                std::unordered_set<std::pair<int, int>, pair_hash> s2)
{
    std::unordered_set<std::pair<int, int>, pair_hash>::iterator it;
    for (it = s1.begin(); it != s1.end(); it++) {
        if (std::find(s2.begin(), s2.end(), *it) != s2.end())
            return true;
    }
    return false;
}

void solve()
{
    int n;
    scanf("%d", &n);
    //printf("There are %d paths.\n", n);
    std::pair<int, int> c(0, 0);
    std::vector<std::unordered_set<std::pair<int, int>, pair_hash> > paths;
    int total = 0;
    for (int i = 0; i < n; i++) {
        int d, steps;
        //printf("\n");
        scanf("%d %d", &d, &steps);
        //total += steps;
        //printf("There are %d steps.\n", steps);
        std::unordered_set<std::pair<int, int>, pair_hash> s;
        switch (d) {
        case 1:
            //printf("The direction is north.\n");
            for (int j = 1; j <= (steps - 1); j++) {
                c.second++;
                //printf("Stepped to (%d,%d)\n", c.first, c.second);
                s.insert(c);
            }
            c.second++;
            break;
        case 2:
            //printf("The direction is south.\n");
            for (int j = 1; j <= (steps - 1); j++) {
                c.second--;
                //printf("Stepped to (%d,%d)\n", c.x, c.y);
                s.insert(c);
            }
            c.second--;
            break;
        case 3:
            //printf("The direction is east.\n");
            for (int j = 1; j <= (steps - 1); j++) {
                c.first--;
                //printf("Stepped to (%d,%d)\n", c.x, c.y);
                s.insert(c);
            }
            c.first--;
            break;
        case 4:
            //printf("The direction is west.\n");
            for (int j = 1; j <= (steps - 1); j++) {
                c.first++;
                //printf("Stepped to (%d,%d)\n", c.x, c.y);
                s.insert(c);
            }
            c.first++;
            break;
        default:
            exit(-1);
        }
        //printf("The size of the set is %d.\n", s.size());
        paths.push_back(s);
    }

    //printf("total is %d\n", total);
    //printf("The number of paths is %d.\n", paths.size());
    std::unordered_set<std::pair<int, int>, pair_hash> fullpath;
    for (int i = 0; i < paths.size(); i++) {
        if (unordered_set_intersection(paths[i], fullpath)) {
            printf("%d\n", i + 1);
            /*std::unordered_set<std::pair<int, int>, pair_hash>::iterator it;
            printf("fullpath is:\n");
            for (it = fullpath.begin(); it != fullpath.end(); it++) {
                printf("(%d,%d) ", (*it).first, (*it).second);
            }
            printf("\n");
            printf("paths[%d] is:\n", i);
            for (it = paths[i].begin(); it != paths[i].end(); it++) {
                printf("(%d,%d) ", (*it).first, (*it).second);
            }
            printf("\n");*/
            return;
        } else {
            fullpath.insert(paths[i].begin(), paths[i].end());
        }
        //printf("The size of the fullpath is %d!\n", fullpath.size());
    }
    printf("-1\n");
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

