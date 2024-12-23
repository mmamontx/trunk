#include <memory.h>
#include <stdio.h>
#include <queue>
#include <set>

struct counter {
    int id;
    int t;

    bool operator<(const counter& x) const
    {
        //return t < x.t;
        return t > x.t;
    }
};

struct client {
    int t;
    int dt;
    bool inset;
    int cntid;
};

int main()
{
    int n, d;
    scanf("%d %d", &n, &d);
    //printf("There are %d clients and %d counters.\n", n, d);
    std::priority_queue<counter> cnt;
    for (int i = 0; i < d; i++) {
        struct counter c;
        c.id = i;
        c.t = 0;
        cnt.push(c);
    }
    struct client cl[n];
    for (int i = 0; i < n; i++) {
        scanf("%d %d", &(cl[i].t), &(cl[i].dt));
        //printf("(%d) t = %d dt = %d\n", i + 1, cl[i].t, cl[i].dt);
        cl[i].inset = false;
        cl[i].cntid = 0;
    }
    int pairs;
    scanf("%d", &pairs);
    //printf("There are %d pairs:\n", pairs);
    std::vector<std::set<int> > sets;
    for (int i = 0; i < pairs; i++) {
        int c1, c2;
        scanf("%d %d", &c1, &c2);
        //printf("%d & %d\n", c1, c2);
        cl[c1 - 1].inset = true;
        cl[c2 - 1].inset = true;
        bool flag = false;
        for (int j = 0; j < sets.size(); j++) {
            if ((sets[j].find(c1) != sets[j].end()) || (sets[j].find(c2) != sets[j].end())) {
                sets[j].insert(c1);
                sets[j].insert(c2);
                flag = true;
                break;
            }
        }
        if (!flag) {
            std::set<int> newset;
            newset.insert(c1);
            newset.insert(c2);
            sets.push_back(newset);
        }
    }
    /*for (int i = 0; i < sets.size(); i++) {
        printf("Set %d is:", i);
        std::set<int>::iterator it;
        for (it = sets[i].begin(); it != sets[i].end(); it++) {
            printf(" %d", *it);
        }
        printf("\n");
    }*/
    std::priority_queue<int> waited;
    struct counter c = cnt.top();
    for (int i = 0; i < n; i++) {
        if ((cl[i].inset) && (cl[i].cntid == 0))
            for (int j = 0; j < sets.size(); j++)
                if (sets[j].find(i + 1) != sets[j].end()) {
                    std::set<int>::iterator it;
                    for (it = sets[j].begin(); it != sets[j].end(); it++)
                        cl[*it - 1].cntid = c.id + 1;
                    break;
                }

        if (cl[i].inset && cl[i].cntid) {
            std::priority_queue<counter> tmpq;
            while (c.id != (cl[i].cntid - 1)) {
                tmpq.push(c);
                cnt.pop();
                c = cnt.top();
            }

            if (c.t > cl[i].t) {
                //printf("The delta is %d!\n", c.t - cl[i].t);
                waited.push(c.t - cl[i].t);
                c.t += cl[i].dt;
            } else {
                c.t = cl[i].t + cl[i].dt;
            }
            //printf("Counter %d is now %d! (in pair)\n", c.id + 1, c.t);
            cnt.pop();
            cnt.push(c);

            while (tmpq.empty() != true) {
                struct counter ctmp;
                ctmp = tmpq.top();
                cnt.push(ctmp);
                tmpq.pop();
            }

            c = cnt.top();

            continue;
        }

        if (c.t <= cl[i].t) {
            c.t = cl[i].t + cl[i].dt;
            //printf("Counter %d is now %d!\n", c.id + 1, c.t);

            cnt.pop();
            cnt.push(c);

            c = cnt.top();
        } else {
            //printf("Waited %d\n", c.t - cl[i].t);
            waited.push(c.t - cl[i].t);

            c.t += cl[i].dt;
            //printf("Counter %d is now %d!\n", c.id + 1, c.t);

            cnt.pop();
            cnt.push(c);

            c = cnt.top();
        }
    }
    int max = 0;
    while (cnt.size()) {
        struct counter foo;
        foo = cnt.top();
        if (max < foo.t)
            max = foo.t;
        cnt.pop();
    }
    printf("%d", max - 1);
    int remaining = 3;
    while (waited.size() && (remaining > 0)) {
        int w = waited.top();
        printf(" %d", w);
        waited.pop();
        remaining--;
    }
    while (remaining > 0) {
        printf(" 0");
        remaining--;
    }
    printf("\n");
    return 0;
}

