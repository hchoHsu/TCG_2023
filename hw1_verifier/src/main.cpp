#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <queue>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include "ewn.h"
#define DICE 6
#define MAX_BUFFER 5000000
// #define DBG
using namespace std;

vector<EWN> buffer(MAX_BUFFER);
long long bidx = 0;
int best = 0;

unordered_map<size_t, int> vis;
// unordered_set<size_t> vis_calc;

struct game_cmp {
    bool operator()(const size_t &a, const size_t &b) const {
        return (buffer[vis[a]].state_value) > (buffer[vis[b]].state_value);
    }
};
priority_queue<size_t, vector<size_t>, game_cmp> pq;


int f_solve()
{
    auto start = chrono::steady_clock::now();
    auto end = chrono::steady_clock::now();
    
    int cur;
    size_t cur_hash;
    int n_move;
    int moves[MAX_MOVES];

    best = 0;
    ewnHash ehash;

    cur_hash = ehash(buffer[0]);
    pq.push(cur_hash);
    vis[cur_hash] = 0;
    
    while (!pq.empty())
    {
        cur_hash = pq.top();
        // vis_calc.insert(cur_hash);
        cur = vis[cur_hash];
        pq.pop();

        n_move = buffer[cur].move_gen_all(moves);
        // cout << "---\n";
        // buffer[cur].print_board();
        for (int i = 0; i < n_move; i++) {
            buffer[cur].do_move(moves[i]);
            // buffer[cur].print_board();
            
            end = chrono::steady_clock::now();
            if (buffer[cur].is_goal() && (buffer[cur].n_plies < buffer[best].n_plies || best == 0)){
                best = bidx;
                buffer[bidx++] = buffer[cur];
                cout << "Find solution at: " << setw(10) << chrono::duration_cast<chrono::nanoseconds>(end-start).count() << " ns" << endl;
                buffer[cur].undo();
                break;
            }

            if (chrono::duration_cast<chrono::nanoseconds>(end - start).count() >= 4900000000)
                return buffer[best].print_history();

            cur_hash = ehash(buffer[cur]);
            if (vis.find(cur_hash) == vis.end()) {
                vis[cur_hash] = bidx;
                buffer[bidx++] = buffer[cur];
                pq.push(cur_hash);
            }

            buffer[cur].undo();
        }
    }
    
    return 0;
}

int main(int argc, char *argv[])
{
    buffer[bidx++].scan_board();
    f_solve();
    return 0;
}