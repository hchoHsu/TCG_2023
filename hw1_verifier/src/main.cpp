#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <queue>
#include <chrono>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>

#define DICE 6
#define MAX_BUFFER 5000000
#define MAX_ROW 9
#define MAX_COL 9
#define MAX_PIECES 6
#define MAX_PERIOD 18
#define MAX_PLIES 100
#define MAX_MOVES 16

#define max(i,j) i>j?i:j
// #define DBG

using namespace std;

int row;
int col;
int period;
int goal_piece;
int dice_seq[MAX_PERIOD];
int dir_value[8];

void set_dir_value() {
    dir_value[0] = -col - 1;
    dir_value[1] = -col;
    dir_value[2] = -col + 1;
    dir_value[3] = -1;
    dir_value[4] = 1;
    dir_value[5] = col - 1;
    dir_value[6] = col;
    dir_value[7] = col + 1;
}

int board[MAX_ROW * MAX_COL];

class EWN{
public:
    int pos[MAX_PIECES + 2];  // pos[0] and pos[MAX_PIECES + 1] are not used
    // int history[MAX_PLIES];
    int n_plies;
    int step_need;
    int valid_move;
    int state_value;
    int parent;
    int restore_last_move;
    int last_move;

    EWN()
    {
        pos[0] = 999;
        pos[MAX_PIECES + 1] = 999;
        for (int i = 1; i <= MAX_PIECES; i++) {
            pos[i] = -1;
        }
        n_plies = 0;
        step_need = 0;
        valid_move = 0;
        state_value = 0;
        parent = -1;
        last_move = 0;
    }
    void scan_board()
    {
        int tmp;
        scanf(" %d %d", &row, &col);
        for (int i = 0; i < row * col; i++) {
            scanf(" %d", &tmp);
            if (tmp > 0)
                pos[tmp] = i;
        }
        scanf(" %d", &period);
        for (int i = 0; i < period; i++) {
            scanf(" %d", &dice_seq[i]);
        }
        scanf(" %d", &goal_piece);

        set_dir_value();
        // printf("hhh");
        calc_step_need();
        calc_valid_move();
        state_value = step_need + n_plies - valid_move;
    }
    void print_board()
    {
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                printf("%4d", board[i * col + j]);
            }
            printf("\n");
        }
        printf("%d,%d\n", n_plies, step_need);
    }
    
    bool is_goal()
    {
        if (goal_piece == 0) {
            if (board[row * col - 1] > 0) return true;
        }
        else {
            if (board[row * col - 1] == goal_piece) return true;
        }
        return false;
    }

    int move_gen_all(int *moves);
    void do_move(int move);
    void undo();

    void calc_step_need();
    void calc_valid_move();
    void copy(const EWN& a, int new_parent);
};

/*
move: an integer using only 12 bits
3~0: store the direction
7~4: store the piece number
11~8: store the eaten piece (used only in history)
*/

int move_gen2(int *moves, int piece, int location) {
    int count = 0;
    int lrow = location / col;
    int lcol = location % col;

    bool left_ok = lcol != 0;
    bool right_ok = lcol != col - 1;
    bool up_ok = lrow != 0;
    bool down_ok = lrow != row - 1;

    if (down_ok && right_ok) moves[count++] = piece << 4 | 7;
    if (down_ok) moves[count++] = piece << 4 | 6;
    if (right_ok) moves[count++] = piece << 4 | 4;

    if (down_ok && left_ok) moves[count++] = piece << 4 | 5;
    if (up_ok && right_ok) moves[count++] = piece << 4 | 2;

    if (up_ok) moves[count++] = piece << 4 | 1;
    if (left_ok) moves[count++] = piece << 4 | 3;
    if (up_ok && left_ok) moves[count++] = piece << 4 | 0;

    return count;
}

int EWN::move_gen_all(int *moves) {
    memset(board, 0, MAX_ROW * MAX_COL);
    for (int i = 0; i < MAX_PIECES + 2; i++) {
        if (pos[i] != -1 && pos[i] != 999)
            board[pos[i]] = i;
        // printf("hi\n");
    }

    int count = 0;
    int dice = dice_seq[n_plies % period];
    if (pos[dice] == -1) {
        int small = dice - 1;
        int large = dice + 1;

        while (pos[small] == -1) small--;
        while (pos[large] == -1) large++;

        if (small >= 1)
            count += move_gen2(moves, small, pos[small]);
        if (large <= MAX_PIECES)
            count += move_gen2(moves + count, large, pos[large]);
    }
    else {
        count = move_gen2(moves, dice, pos[dice]);
    }

    return count;
}

void EWN::do_move(int move) {
    int piece = move >> 4;
    int direction = move & 15;
    int dst = pos[piece] + dir_value[direction];

    if (n_plies == MAX_PLIES) {
        fprintf(stderr, "cannot do anymore moves\n");
        exit(1);
    }
    if (board[dst] > 0) {
        pos[board[dst]] = -1;
        calc_valid_move();
        move |= board[dst] << 8;

        if (board[dst] == goal_piece)
            step_need = MAX_MOVES + 1;
    }
    board[pos[piece]] = 0;
    board[dst] = piece;
    pos[piece] = dst;
    // history[n_plies++] = move;
    n_plies++;
    restore_last_move = last_move;
    last_move = move;

    if (goal_piece == 0 || goal_piece == piece)
        calc_step_need();

    state_value = step_need + n_plies - valid_move;
}

void EWN::undo() {
    if (n_plies == 0) {
        fprintf(stderr, "no history\n");
        exit(1);
    }

    // int move = history[--n_plies];
    n_plies--;
    int move = last_move;
    last_move = restore_last_move;
    int eaten_piece = move >> 8;
    int piece = (move & 255) >> 4;
    int direction = move & 15;
    int dst = pos[piece] - dir_value[direction];

    if (eaten_piece > 0) {
        board[pos[piece]] = eaten_piece;
        pos[eaten_piece] = pos[piece];
        calc_valid_move();
    }
    else board[pos[piece]] = 0;
    board[dst] = piece;
    pos[piece] = dst;

    if (goal_piece == 0 || goal_piece == piece || goal_piece == eaten_piece)
        calc_step_need();

    state_value = step_need + n_plies - valid_move;
}

void EWN::calc_valid_move() {
    if (goal_piece == 0 || pos[goal_piece] < 0) {
        valid_move = 0;
        return;
    }

    valid_move = 0;
    int small = goal_piece - 1;
    int large = goal_piece + 1;

    while (pos[small] == -1) small--;
    while (pos[large] == -1) large++;

    for (int i = 0; i < period; i++) {
        if (dice_seq[i] > small && dice_seq[i] < large)
            valid_move++;
    }

    return;
}

void EWN::calc_step_need() {
    int tmp = 0;
    step_need = MAX_MOVES + 1;
    // target shortest distance
    if (goal_piece != 0) {
        if (pos[goal_piece] < 0) {
            return;
        }
        tmp = max(row - pos[goal_piece]/row - 1, col - pos[goal_piece]%col - 1);
        step_need = tmp;
    }
    else {
        for (int i = 1; i < MAX_PIECES+1; i++) {
            if (pos[i] != -1) {
                tmp = max(row - pos[i]/row - 1, col - pos[i]%col - 1);
                if (step_need > tmp)
                    step_need = tmp;
            }
        }
    }
    return;
}

void EWN::copy(const EWN& a, int new_parent) {
    for (int i = 0; i < MAX_PIECES + 2; i++) {
        pos[i] = a.pos[i];
    }
    n_plies = a.n_plies;
    state_value = a.state_value;
    valid_move = a.valid_move;
    state_value = a.state_value;
    // for (int i = 0; i < a.n_plies; i++) {
    //     history[i] = a.history[i];
    // }
    last_move = a.last_move;
    parent = new_parent;
}

class ewnHash {
public:
    size_t operator()(const EWN& cur)const {
        std::hash<int> hasher;
        size_t seed = 0;
        seed ^= hasher(cur.pos[1]) + 0x9e3779b9 + (seed<<6) + (seed >> 2);
        seed ^= hasher(cur.pos[2]) + 0x9e3779b9 + (seed<<6) + (seed >> 2);
        seed ^= hasher(cur.pos[3]) + 0x9e3779b9 + (seed<<6) + (seed >> 2);
        seed ^= hasher(cur.pos[4]) + 0x9e3779b9 + (seed<<6) + (seed >> 2);
        seed ^= hasher(cur.pos[5]) + 0x9e3779b9 + (seed<<6) + (seed >> 2);
        seed ^= hasher(cur.pos[6]) + 0x9e3779b9 + (seed<<6) + (seed >> 2);
        // TODO: remove n_plies from hash
        // seed ^= hasher(cur.n_plies) + 0x9e3779b9 + (seed<<6) + (seed >> 2);
        return seed;
    }
};

class ewnHashEqual {
public:
    size_t operator()(const EWN& lhs, const EWN& rhs)const {
        return lhs.pos[1] == rhs.pos[1] &&
               lhs.pos[2] == rhs.pos[2] &&
               lhs.pos[3] == rhs.pos[3] &&
               lhs.pos[4] == rhs.pos[4] &&
               lhs.pos[5] == rhs.pos[5] &&
               lhs.pos[6] == rhs.pos[6]; // &&
            //    lhs.n_plies == rhs.n_plies;
    }
};

vector<EWN> buffer(MAX_BUFFER);
long long bidx = 0;
int best = 0;

int print_history_recursive(int i) {
    if (buffer[i].parent == -1)
        return 0;
    else {
        print_history_recursive(buffer[i].parent);
        int piece, dir;
        piece = (buffer[i].last_move & 255) >> 4;
        dir = buffer[i].last_move & 15;
        printf("%d %d\n", piece, dir);
    }
    return 0;
}

int print_history(int cur_idx) {
    cout << buffer[cur_idx].n_plies << '\n';
    return print_history_recursive(cur_idx);
}

unordered_map<size_t, int> vis;

struct game_cmp {
    bool operator()(const size_t &a, const size_t &b) const {
        return (buffer[vis[a]].state_value) > (buffer[vis[b]].state_value);
    }
};
priority_queue<size_t, vector<size_t>, game_cmp> pq;


int f_solve(chrono::time_point<chrono::steady_clock> &start)
{
    // auto start = chrono::steady_clock::now();
    chrono::time_point<chrono::steady_clock> end;

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
        cur = vis[cur_hash];
        pq.pop();
        n_move = buffer[cur].move_gen_all(moves);
        // cout << "---\n";
        // buffer[cur].print_board();
        for (int i = 0; i < n_move; i++) {
            buffer[cur].do_move(moves[i]);
            // buffer[cur].print_board();

            if (buffer[cur].is_goal() && (buffer[cur].n_plies < buffer[best].n_plies || best == 0)){
                best = bidx;
                // buffer[bidx++] = buffer[cur];
                buffer[bidx++].copy(buffer[cur], cur);
		        bidx %= MAX_BUFFER;
                // cout << "Find solution at: " << setw(10) << chrono::duration_cast<chrono::nanoseconds>(end-start).count() << " ns" << endl;
                buffer[cur].undo();
                break;
            }

            cur_hash = ehash(buffer[cur]);
            if (vis.find(cur_hash) == vis.end()) {
                vis[cur_hash] = bidx;
                // buffer[bidx++] = buffer[cur];
                buffer[bidx++].copy(buffer[cur], cur);
                pq.push(cur_hash);
		        bidx %= MAX_BUFFER;
            }
            else {
                int vidx = vis[cur_hash];
                if (buffer[vidx].n_plies > buffer[cur].n_plies) {   // if the new one takes less steps
                    vis[cur_hash] = cur;
                    buffer[bidx++].copy(buffer[cur], cur);
		            bidx %= MAX_BUFFER;
                }
            }

            buffer[cur].undo();
        }

        end = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::nanoseconds>(end - start).count() >= 4200000000)
            return print_history(best);
            // return buffer[best].print_history();
    }
    // return buffer[best].print_history();
    return print_history(best);
}

int main(int argc, char *argv[])
{
    chrono::time_point<chrono::steady_clock> start = chrono::steady_clock::now();
    buffer[bidx++].scan_board();
    f_solve(start);
    return 0;
}
