#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <chrono>
#include <unordered_set>
#include "ewn.h"
#define DICE 6
#define MAX_BUFFER 1000000
// #define DBG
using namespace std;

vector<EWN> buffer(MAX_BUFFER);
long long idx = 0;

struct game_cmp {
    bool operator()(const EWN &a, const EWN &b) const {
        return (a.step_need + a.n_plies) > (b.step_need + b.n_plies);
    }
};

int f_solve(EWN &init_game)
{
    unordered_set<EWN, ewnHash, ewnHashEqual> vis;
    priority_queue<EWN, vector<EWN>, game_cmp> pq;
    
    EWN cur;
    int n_move;
    int moves[MAX_MOVES];

    pq.push(init_game);
    vis.insert(init_game);
    // init_game.print_board();
    
    while (!pq.empty())
    {
        cur = pq.top();
        #ifdef DBG
        printf("---\n");
        cur.print_board();
        #endif
        pq.pop();

        n_move = cur.move_gen_all(moves);
        for (int i = 0; i < n_move; i++) {
            cur.do_move(moves[i]);
            if (cur.is_goal())
                return cur.print_history();
            
            // TODO: visited, evaluate
            if (vis.find(cur) == vis.end()) {
                #ifdef DBG
                cur.print_board();
                #endif
                vis.insert(cur);
                pq.push(cur);
            }
            
            cur.undo();
        }
    }
    
    return 0;
}

int main(int argc, char *argv[])
{
    EWN game;
    game.scan_board();

    f_solve(game);

    return 0;
}