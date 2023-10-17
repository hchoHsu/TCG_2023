#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <queue>
#include <chrono>
#include <unordered_map>
#include "ewn.h"
#define DICE 6
#define MAX_BUFFER 5000000
// #define DBG
using namespace std;

vector<EWN> buffer(MAX_BUFFER);
long long bidx = 0;
int best = 0;

unordered_map<EWN, int, ewnHash, ewnHashEqual> vis;

struct game_cmp {
    bool operator()(const int &a, const int &b) const {
        return (buffer[a].state_value) > (buffer[b].state_value);
    }
};
priority_queue<int, vector<int>, game_cmp> pq;


int f_solve()
{
    auto start = chrono::steady_clock::now();
    auto end = chrono::steady_clock::now();
    
    int cur;
    int n_move;
    int moves[MAX_MOVES];

    best = 0;

    pq.push(0);
    vis[buffer[0]] = 0;
    
    while (!pq.empty())
    {
        cur = pq.top();
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

            if (vis.find(buffer[cur]) == vis.end()) {
                vis[buffer[cur]] = bidx;
                buffer[bidx] = buffer[cur];
                pq.push(bidx++);
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