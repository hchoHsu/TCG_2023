#include <iostream>
#include <iomanip>
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

struct game_cmp {
    bool operator()(const EWN &a, const EWN &b) const {
        return (a.step_need + a.n_plies - a.valid_move) > (b.step_need + b.n_plies - b.valid_move);
    }
};

vector<EWN> buffer(MAX_BUFFER);
long long idx = 0;

unordered_set<EWN, ewnHash, ewnHashEqual> vis;
priority_queue<EWN, vector<EWN>, game_cmp> pq;

EWN best_cur;

int f_solve(EWN &init_game)
{
    auto start = chrono::steady_clock::now();
    auto end = chrono::steady_clock::now();
    
    EWN cur;
    int n_move;
    int moves[MAX_MOVES];

    best_cur = init_game;
    best_cur.n_plies = MAX_PLIES;

    pq.push(init_game);
    vis.insert(init_game);
    
    while (!pq.empty())
    {
        cur = pq.top();
        pq.pop();

        n_move = cur.move_gen_all(moves);
        for (int i = 0; i < n_move; i++) {
            cur.do_move(moves[i]);
            end = chrono::steady_clock::now();
            if (cur.is_goal() && cur.n_plies < best_cur.n_plies){
                best_cur = cur;
                cout << "Find solution at: " << setw(10) << chrono::duration_cast<chrono::nanoseconds>(end-start).count() << " ns" << endl;
                break;
            }

            if (chrono::duration_cast<chrono::nanoseconds>(end - start).count() >= 4900000000)
                return best_cur.print_history();

            if (vis.find(cur) == vis.end()) {
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