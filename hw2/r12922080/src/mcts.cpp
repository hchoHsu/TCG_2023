#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <cstring>
#include "mcts.h"

#define parent(ptr) (nodes[ptr].p_id)
#define child(ptr, i) (nodes[ptr].c_id[i])

#define is_red_cube(x) ((x) >= 0 && (x) < (MAX_CUBES))
#define is_blue_cube(x) ((x) >= (MAX_CUBES) && (x) < (MAX_CUBES) << 1)
#define is_red_cube_fast(x) ((x) < (MAX_CUBES))
#define is_blue_cube_fast(x) ((x) >= (MAX_CUBES))
#define is_empty_cube(x) ((x) == 0xf)
#define change_player(x) ((x) ^= 1)

static const int dir_val[2][4] = {{1, COL, COL+1, -COL+1}, {-1, -COL, -COL-1, COL-1}};
static const int init_pos[2][MAX_CUBES] = {
    {0, 1, 2, COL, COL+1, COL*2},
    {(ROW-2)*COL-1, (ROW-1)*COL-2, (ROW-1)*COL-1, ROW*COL-3, ROW*COL-2, ROW*COL-1}
};

size_t size_of_pos;
size_t size_of_board;

int dice_seq[PERIOD];

node nodes[2500000];
int func_expand_moves[MAX_MOVES];
int func_random_moves[MAX_MOVES];
int total_nodes = 0;

min_board simulate_01, simulate_02;

int root = 0;

double SR = 0;

long long simulate_deltaN;
long long simulate_deltaW;

void EWN::init_board() {
    memset(board, 0xff, sizeof(board));
    memset(pos, 0xff, sizeof(pos));

    size_of_board = sizeof(board);
    size_of_pos = sizeof(pos);
    num_cubes[0] = MAX_CUBES;
    num_cubes[1] = MAX_CUBES;
    next = RED;
    n_plies = 0;

    int offset = 0;
    for (int player = 0; player < 2; player++) {
        for (int i = 0; i < MAX_CUBES; i++) {
            int cube = getchar() - '0';
            board[init_pos[player][i]] = cube + offset;
            pos[cube + offset] = init_pos[player][i];
        }
        offset += MAX_CUBES;
    }
    fprintf(stderr, "dice_seq:");
    for (int i = 0; i < PERIOD; i++) {
        dice_seq[i] = getchar() - '0';
        fprintf(stderr, "%d, ", dice_seq[i]);
    } fprintf(stderr, "\n\n");
}

bool EWN::is_over() {
    if (num_cubes[0] == 0 || num_cubes[1] == 0) return true;
    if (is_blue_cube(board[0])) return true;
    if (is_red_cube(board[ROW * COL - 1])) return true;
    return false;
}

/*
move: an integer using only 12 bits
3~0: store the direction
7~4: store the cube number
11~8: store the eaten cube (used only in history)
*/

int move_gen(int *move_arr, int cube, int location) {
    int count = 0;
    const int row = location / COL;
    const int col = location % COL;
    bool h_ok;  // horizontal
    bool v_ok;  // vertical
    bool rev_v_ok;  // reverse, vertical

    if (is_red_cube_fast(cube)) {
        h_ok = col != COL - 1;
        v_ok = row != ROW - 1;
        rev_v_ok = row != 0;
    }
    else {
        h_ok = col != 0;
        v_ok = row != 0;
        rev_v_ok = row != ROW - 1;
    }
    if (h_ok) move_arr[count++] = cube << 4;
    if (v_ok) move_arr[count++] = cube << 4 | 1;
    if (h_ok && v_ok) move_arr[count++] = cube << 4 | 2;
    if (h_ok && rev_v_ok) move_arr[count++] = cube << 4 | 3;

    return count;
}

int EWN::move_gen_all(int *move_arr) {
    int count = 0;
    const int dice = dice_seq[n_plies % PERIOD];
    const int offset = next == BLUE ? MAX_CUBES : 0;
    int * const self_pos = pos + offset;

    if (self_pos[dice] == -1) {
        int small = dice - 1;
        int large = dice + 1;

        while (small >= 0 && self_pos[small] == -1) small--;
        while (large < MAX_CUBES && self_pos[large] == -1) large++;

        if (small >= 0)
            count += move_gen(move_arr, small + offset, self_pos[small]);
        if (large < MAX_CUBES)
            count += move_gen(move_arr + count, large + offset, self_pos[large]);
    }
    else {
        count = move_gen(move_arr, dice + offset, self_pos[dice]);
    }

    return count;
}

void EWN::do_move(int move) {
    int cube = move >> 4;
    int direction = move & 0xf;
    int dst = pos[cube] + dir_val[next][direction];
    // fprintf(stderr, "> Do move: %2d, %2d\n", cube, direction);

    if (n_plies == MAX_PLIES) {
        fprintf(stderr, "cannot do anymore moves\n");
        exit(1);
    }
    if (board[dst] >= 0) {
        if (is_red_cube_fast(board[dst])) num_cubes[RED]--;
        else num_cubes[BLUE]--;
        pos[board[dst]] = -1;
        move |= board[dst] << 8;
    }
    else move |= 0xf00;
    board[pos[cube]] = -1;
    board[dst] = cube;
    pos[cube] = dst;
    history[n_plies++] = move;
    change_player(next);
}

void EWN::undo() {
    if (n_plies == 0) {
        fprintf(stderr, "no history\n");
        exit(1);
    }
    change_player(next);

    int move = history[--n_plies];
    int eaten_cube = move >> 8;
    int cube = (move & 0xff) >> 4;
    int direction = move & 0xf;
    int src = pos[cube] - dir_val[next][direction];

    if (!is_empty_cube(eaten_cube)) {
        if (is_red_cube_fast(eaten_cube)) num_cubes[RED]++;
        else num_cubes[BLUE]++;
        board[pos[cube]] = eaten_cube;
        pos[eaten_cube] = pos[cube];
    }
    else board[pos[cube]] = -1;
    board[src] = cube;
    pos[cube] = src;
}

void EWN::print_board() {
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++)
            fprintf(stderr, "%2d ", board[i * COL + j]);
        fprintf(stderr, "\n");
    }
}

void min_board::print_board() {
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            if (board[i * COL + j] == -1)
                fprintf(stderr, " _ ");
            else
                fprintf(stderr, "%2d ", board[i * COL + j]);
        }
        fprintf(stderr, "\n");
    }
}

bool min_board::is_over() {
    if (num_cubes[0] == 0 || num_cubes[1] == 0) return true;
    if (is_blue_cube(board[0])) return true;
    if (is_red_cube(board[ROW * COL - 1])) return true;
    return false;
}

int min_board::is_over_rtresult() {
    if (num_cubes[RED] == 0) return 1;
    if (num_cubes[BLUE] == 0) return -1;
    if (board[0] >= MAX_CUBES) return 1;
    if (is_red_cube(board[41])) return -1;
    return 0;
}

int min_board::move_gen_all(int *move_arr) {
    int count = 0;
    const int dice = dice_seq[n_plies % PERIOD];
    const int offset = next == BLUE ? MAX_CUBES : 0;
    int * const self_pos = pos + offset;

    if (self_pos[dice] == -1) {
        int small = dice - 1;
        int large = dice + 1;

        while (small >= 0 && self_pos[small] == -1) small--;
        while (large < MAX_CUBES && self_pos[large] == -1) large++;

        if (small >= 0)
            count += move_gen(move_arr, small + offset, self_pos[small]);
        if (large < MAX_CUBES)
            count += move_gen(move_arr + count, large + offset, self_pos[large]);
    }
    else {
        count = move_gen(move_arr, dice + offset, self_pos[dice]);
    }

    return count;
}

void min_board::do_move(int move) {
    int cube = move >> 4;
    int direction = move & 0xf;
    int dst = pos[cube] + dir_val[next][direction];

    if (board[dst] >= 0) {
        if (is_red_cube_fast(board[dst])) num_cubes[RED]--;
        else num_cubes[BLUE]--;
        pos[board[dst]] = -1;
    }
    board[pos[cube]] = -1;
    board[dst] = cube;
    pos[cube] = dst;
    n_plies++;
    change_player(next);
}

int min_board::do_move_simulate(int move) {
    int cube = move >> 4;
    int direction = move & 0xf;
    int dst = pos[cube] + dir_val[next][direction];

    if (board[dst] >= 0) {
        if (is_red_cube_fast(board[dst])) num_cubes[RED]--;
        else num_cubes[BLUE]--;
        pos[board[dst]] = -1;
        move |= board[dst] << 8;
    }
    else move |= 0xf00;
    board[pos[cube]] = -1;
    board[dst] = cube;
    pos[cube] = dst;
    n_plies++;
    change_player(next);

    return move;
}

void min_board::undo(int move) {
    change_player(next);

    n_plies--;
    int eaten_cube = move >> 8;
    int cube = (move & 0xff) >> 4;
    int direction = move & 0xf;
    int src = pos[cube] - dir_val[next][direction];

    if (!is_empty_cube(eaten_cube)) {
        if (is_red_cube_fast(eaten_cube)) num_cubes[RED]++;
        else num_cubes[BLUE]++;
        board[pos[cube]] = eaten_cube;
        pos[eaten_cube] = pos[cube];
    }
    else board[pos[cube]] = -1;
    board[src] = cube;
    pos[cube] = src;
}

void min_board::copy(const min_board &game) {
    memcpy(pos, game.pos, size_of_pos);
    num_cubes[0] = game.num_cubes[0];
    num_cubes[1] = game.num_cubes[1];
    next = game.next;
    memcpy(board, game.board, size_of_board);
    n_plies = game.n_plies;
}

void min_board::copy(const EWN &game) {
    memcpy(pos, game.pos, size_of_pos);
    num_cubes[0] = game.num_cubes[0];
    num_cubes[1] = game.num_cubes[1];
    next = game.next;
    memcpy(board, game.board, size_of_board);
    n_plies = game.n_plies;
}

int MCTS(EWN &game) {
    total_nodes = 0;
    clock_t a, time_limit;
    a = clock();
    time_limit = a + 1.7 * CLOCKS_PER_SEC;
    // game.print_board();
    root = 0;
    int ptr = root;
    nodes[root].next = game.next;
    nodes[root].p_id=root;
    nodes[root].depth=0;
    nodes[root].Nchild=0;
    nodes[root].Ntotal=0;
    nodes[root].CsqrtlogN=0;
    nodes[root].sqrtN=0;
    nodes[root].sum1=0;
    nodes[root].Average=0;

    min_board pvb;
    pvb.copy(game);

    int findpv_cnt = 0, rtover;
    
    while (clock() < time_limit)
    {
        pvb.copy(game);
        ptr = FindPV(root, pvb);
        findpv_cnt++;
        // pvb.print_board();
        rtover = pvb.is_over_rtresult();
        if (rtover) {
            simulate_is_over(ptr, rtover);
            backpropagation_single(ptr);
        }
        else {
            if (total_nodes < 2499990) {
                expand(ptr, pvb);
                simulate(ptr, pvb);
                backpropagation(ptr);
            }
            else {
                // Todo: simulate will destroy the rate below this node
                fprintf(stderr, "Buffer out of range!");
                simulate(ptr, pvb);
                backpropagation_single(ptr);
            }
        }
    }

    // Find best move
    int maxchild, ctmp;
    // long long total_simulates=0;
    double maxV, tmp;
    maxchild = child(root, 0);
    maxV = nodes[maxchild].Average;
    // total_simulates += nodes[maxchild].Ntotal;
    for (int i = 1; i < nodes[root].Nchild; i++) {
        ctmp = child(root, i);
        tmp = nodes[ctmp].Average;
        if (maxV < tmp) {
            maxV = tmp;
            maxchild = ctmp;
        }
        // fprintf(stderr, "-- single simulates: %lld\n", nodes[ctmp].Ntotal);
        // total_simulates += nodes[ctmp].Ntotal;
    }

    fprintf(stderr, "Total FindPV: %d\n", findpv_cnt);
    // fprintf(stderr, "Total simulates: %lld\n", total_simulates);
    // fprintf(stderr, "Total nodes: %d\n", total_nodes);
    
    return nodes[maxchild].move;
}

int FindPV(int ptr, min_board &pvb) {
    int maxchild, i, ctmp;
    double maxV, tmp;
    
    while(nodes[ptr].Nchild > 0)
    {
        maxchild = child(ptr, 0);
        maxV = UCB(maxchild);
        for (i = 1; i < nodes[ptr].Nchild; i++) {
            ctmp = child(ptr, i);
            tmp = UCB(ctmp);
            if (maxV < tmp) {
                maxV = tmp;
                maxchild = ctmp;
            }
        }
        ptr = maxchild;
        pvb.do_move(nodes[maxchild].move);
    }
    
    return ptr;
}

void expand(int &id, min_board &game) {
    int i;
    nodes[id].Nchild = game.move_gen_all(func_expand_moves);
    // fprintf(stderr,"Expand num: %d\n", nodes[id].Nchild);

    for (i = 0; i < nodes[id].Nchild; i++)
    {
        nodes[total_nodes].move = func_expand_moves[i];
        nodes[total_nodes].p_id = id;
        nodes[total_nodes].depth = nodes[id].depth + 1;
        nodes[total_nodes].Nchild = 0;
        nodes[total_nodes].Ntotal = SIMULATION_PER_BRANCH;
        nodes[total_nodes].sum1 = 0;
        nodes[id].c_id[i] = total_nodes;
        total_nodes += 1;
        // int cube = func_expand_moves[i] >> 4;
        // int direction = func_expand_moves[i] & 0xf;
        // fprintf(stderr, "> Expand move: %2d, %2d\n", cube, direction);
    }
}

void simulate(int &ptr, min_board &pvb)
{
    int i, id, undo_move, cnt;
    simulate_deltaN = nodes[ptr].Nchild * SIMULATION_PER_BRANCH;
    simulate_deltaW = 0;
    for (i = 0; i < nodes[ptr].Nchild; i++) {
        id = nodes[ptr].c_id[i];
        
        undo_move = pvb.do_move_simulate(nodes[id].move);
        
        simulate_01.copy(pvb);
        
        nodes[id].next = simulate_01.next;

        cnt = SIMULATION_PER_BRANCH;
        
        while(cnt--) {
            // fprintf(stderr, "> Random_walk.\n");
            nodes[id].sum1 += (self_color == random_walk(simulate_01)) ? 1 : 0;
            simulate_01.copy(pvb);
        }

        // nodes[id].CsqrtlogN = 1.18 * sqrt(log((double)nodes[id].Ntotal));
        nodes[id].sqrtN = sqrt((double)nodes[id].Ntotal);
        nodes[id].Average = (double)nodes[id].sum1 / (double)nodes[id].Ntotal;
        
        pvb.undo(undo_move);
        simulate_deltaW += nodes[id].sum1;
    }
}

void simulate_is_over(int &id, int &rtover) {
    nodes[id].Ntotal += 100;
    nodes[id].sum1 += (self_color == rtover) ? 100 : 0;

    // nodes[id].CsqrtlogN = 1.18 * sqrt(log((double)nodes[id].Ntotal));
    nodes[id].sqrtN = sqrt((double)nodes[id].Ntotal);
    nodes[id].Average = (double)nodes[id].sum1 / (double)nodes[id].Ntotal;
}

void backpropagation(int ptr) {
    while (ptr != root) {
        update_nodes(ptr);
        ptr = parent(ptr);
    }
    update_nodes(root);
}

void backpropagation_single(int ptr) {
    simulate_deltaN = nodes[ptr].Ntotal;
    simulate_deltaW = nodes[ptr].sum1;
    while (ptr != root) {
        update_nodes(ptr);
        ptr = parent(ptr);
    }
    update_nodes(ptr);
}

void update_nodes(int &id) {
    nodes[id].Ntotal += simulate_deltaN;
    nodes[id].CsqrtlogN = 1.18 * sqrt(log((double)nodes[id].Ntotal));
    nodes[id].sqrtN = sqrt((double)nodes[id].Ntotal);
    nodes[id].sum1 += simulate_deltaW;
    nodes[id].Average = (double)nodes[id].sum1 / (double)nodes[id].Ntotal;
}

inline double UCB(int &id) {
    return ((nodes[id].depth%2) ? nodes[id].Average : (1.0-nodes[id].Average)) + nodes[parent(id)].CsqrtlogN / nodes[id].sqrtN;
}

int get_random_move(min_board &game) {
    int num_moves = game.move_gen_all(func_random_moves);
    return func_random_moves[arc4random_uniform(num_moves)];
}

int random_walk(min_board &game) {
    int num_moves, win_flag;
    while(true) {
        // fprintf(stderr, "----next board----\n");
        // game.print_board();
        win_flag = game.is_over_rtresult();
        if (win_flag)
            return win_flag;

        num_moves = game.move_gen_all(func_random_moves);
        game.do_move(func_random_moves[arc4random_uniform(num_moves)]);
    }
    return -1;
}
