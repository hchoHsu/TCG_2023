#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include "Agent.h"
using namespace std;

#define INF_DIST 99990
#define WIN 10000
#define LOSE -(WIN)

#define max(i,j) ((i) > (j) ? (i) : (j))
#define min(i,j) ((i) < (j) ? (i) : (j))

#define is_red_cube(x) ((x) >= 0 && (x) < (MAX_CUBES))
#define is_blue_cube(x) ((x) >= (MAX_CUBES) && (x) < (MAX_CUBES) << 1)
#define is_red_cube_fast(x) ((x) < (MAX_CUBES))
#define is_blue_cube_fast(x) ((x) >= (MAX_CUBES))
#define is_empty_cube(x) ((x) == 0xf)
#define change_player(x) ((x) ^= 1)

static const int dir_val[2][3] = {{1, COL, COL+1}, {-1, -COL, -COL-1}};
static const int init_pos[2][MAX_CUBES] = {
    {0, 1, 2, COL, COL+1, COL*2},
    {(ROW-2)*COL-1, (ROW-1)*COL-2, (ROW-1)*COL-1, ROW*COL-3, ROW*COL-2, ROW*COL-1}
};
static const int pos_determinancy[6] = {-1, -2, -3, -3, -2, -1};

int BOARD[ROW * COL];

int transition_table_count = 0;
int transition_table[TT_SIZE][3];   // [depth, value, bound type]
                                    // bound type: 1: upper, 0: exact, -1: lower
unordered_map<size_t, int> hash_table;

void Print_BOARD() {
    for (int i = 0; i < ROW; i++) {
        fprintf(stderr, "|");
        for (int j = 0; j < COL; j++) {
            if (BOARD[i * COL + j] < 0)
                fprintf(stderr, " _ ");
            else if (BOARD[i * COL + j] < MAX_CUBES)
                fprintf(stderr, "%2d ", BOARD[i * COL + j]);
            else
                fprintf(stderr, "%2c ", 'A' + (BOARD[i * COL + j] - 6));
        }
        fprintf(stderr, "|\n");
    }
    fprintf(stderr, "\n");
}

void EWN::init_board() {
    memset(BOARD, 0xff, sizeof(BOARD));
    memset(transition_table, 0x00, sizeof(transition_table));
    transition_table_count = 0;
    hash_table.clear();
    
    n_plies = 0;

    int offset = 0;
    for (int player = 0; player < 2; player++) {
        for (int i = 0; i < MAX_CUBES; i++) {
            BOARD[pos[i + offset]] = i + offset;
        }
        offset += MAX_CUBES;
    }

    fprintf(stderr, "\nThe current BOARD:\n");
    Print_BOARD();
}

bool EWN::is_over() {
    if (num_cubes[0] == 0 || num_cubes[1] == 0) return true;
    if (is_blue_cube(BOARD[0])) return true;
    if (is_red_cube(BOARD[ROW * COL - 1])) return true;
    return false;
}

int EWN::is_over_rtresult() {
    if (num_cubes[RED] == 0) return BLUE;
    if (num_cubes[BLUE] == 0) return RED;
    if (is_blue_cube(BOARD[0])) return BLUE;
    if (is_red_cube(BOARD[ROW * COL - 1])) return RED;
    return -1;
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
    // bool rev_v_ok;  // reverse, vertical

    if (is_red_cube_fast(cube)) {
        h_ok = col != COL - 1;
        v_ok = row != ROW - 1;
        // rev_v_ok = row != 0;
    }
    else {
        h_ok = col != 0;
        v_ok = row != 0;
        // rev_v_ok = row != ROW - 1;
    }
    if (h_ok && v_ok) move_arr[count++] = cube << 4 | 2;
    if (h_ok) move_arr[count++] = cube << 4;
    if (v_ok) move_arr[count++] = cube << 4 | 1;

    return count;
}

int EWN::move_gen_all(int *move_arr, const int dice) {
    int count = 0;
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
    // printf("(do move) cube %d to dst %d\n", cube, dst);

    if (n_plies == MAX_PLIES) {
        fprintf(stderr, "cannot do anymore moves\n");
        exit(1);
    }
    if (BOARD[dst] >= 0) {
        if (is_red_cube_fast(BOARD[dst])) num_cubes[RED]--;
        else num_cubes[BLUE]--;
        pos[BOARD[dst]] = -1;
        move |= BOARD[dst] << 8;
    }
    else move |= 0xf00;
    BOARD[pos[cube]] = -1;
    BOARD[dst] = cube;
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
        BOARD[pos[cube]] = eaten_cube;
        pos[eaten_cube] = pos[cube];
    }
    else BOARD[pos[cube]] = -1;
    BOARD[src] = cube;
    pos[cube] = src;
}

inline int EWN::pos_shortest_distance(int idx) {
    if (idx >= MAX_CUBES)
        return max((int)(pos[idx] / COL), (int)(pos[idx] % COL));
    else
        return max((int)(ROW - (pos[idx] / COL) - 1), (int)(COL - (pos[idx] % COL) - 1));
}

double EWN::get_state_value() {
    // printf("Get State Value\n");
    // Print_BOARD();
    int rtState = is_over_rtresult();
    // printf("rtState %d\n", rtState);

    if (rtState < 0)
    {
        // Determinancy
        /*
        int i, det = 0;
        if (next == RED) {
            for (i = 0; i < 6; i++) {
                if (pos[i] > 0)
                    det += pos_determinancy[i];
            }
        }
        else {
            for (i = 0; i < 6; i++) {
                if (pos[i+6] > 0)
                    det += pos_determinancy[i];
            }
        }
        */

        // Shortest Distance
        // /*
        int shortest[2] = {4, 4}; // {Red, Blue}

        int offset = 0;
        for (int player = 0; player < 2; player++) {
            for (int i = 0; i < MAX_CUBES; i++) {
                if (pos[i] < 0)
                    continue;
                if (pos_shortest_distance(i + offset) < shortest[player])
                    shortest[player] = pos_shortest_distance(i + offset);
            }
            offset += MAX_CUBES;
        }
        // printf("(get SV) shortest R/B %d / %d\n", shortest[RED], shortest[BLUE]);

        // bigger is better
        if (next == RED)
            return (double)((shortest[1] * 2 - shortest[0]) * 6);
        else
            return (double)((shortest[0] * 2 - shortest[1]) * 6);
        // if (next == RED)
        //     return (double)((shortest[1] - shortest[0] * 2) * 6);
        // else
        //     return (double)((shortest[0] - shortest[1] * 2) * 6);
        // if (next == RED)
        //     return (double)((shortest[1] * 2 - shortest[0]) * DIS_WEIGHT + det * DET_WEIGHT) * 3.0;
        // else
        //     return (double)((shortest[0] * 2 - shortest[1]) * DIS_WEIGHT + det * DET_WEIGHT) * 3.0;
        // if (next == RED)
        //     return (double)((10 - shortest[0])) * 6.0;
        // else
        //     return (double)((10 - shortest[1])) * 6.0;
        // */

        // Weight
        /*
        int rtval = 0;
        if (next == RED) {
            for (int i = 0; i < MAX_CUBES; i++) {
                if (pos[i] >= 0) {
                    int small = i - 1;
                    int large = i + 1;

                    while (small >= 0 && pos[small] == -1) small--;
                    while (large < MAX_CUBES && pos[large] == -1) large++;

                    rtval += (large - small - 1) * pos_shortest_distance(i);
                }
            }
        }
        else {
            for (int i = 0; i < MAX_CUBES; i++) {
                if (pos[i+6] >= 0) {
                    int small = i - 1;
                    int large = i + 1;

                    while (small >= 6 && pos[small] == -1) small--;
                    while (large < 12 && pos[large] == -1) large++;

                    rtval += (large - small - 1) * pos_shortest_distance(i + 6);
                }
            }
        }

        return (double)(20 - rtval) / 12.0;
        */
    }
    else if (rtState == next)
        return LOSE;
    else if (rtState != next)
        return WIN;

    return 0;
}

/* ------------------ NegaScout and star1 ------------------ */

long long depth0_cnt = 0;
long long hit_cnt = 0;

int search_and_get_move_new(EWN &agent, int dice)
{
    // printf( "Get Dice %d\n", dice);
    depth0_cnt = 0;
    hit_cnt = 0;
    int move = NegaScout_Start(agent, dice, -INF_DIST, INF_DIST, DEPTH_LIMIT);
    agent.do_move(move);
    fprintf(stderr, "Dep0_count %lld\n", depth0_cnt);
    fprintf(stderr, "hit_count %lld\n", hit_cnt);
    fprintf(stderr, "TT_count %d\n", transition_table_count);
    fprintf(stderr, "agent new move\n");
    Print_BOARD();
    agent.undo();
    return move;
}

int NegaScout_Start(EWN &agent, int dice, double alpha, double beta, int depth)
{
    int moves[MAX_MOVES];
    int count = agent.move_gen_all(moves, dice);
    // printf("(Neg Start) Count: %d\n", count);

    double m = -INF_DIST;
    double n = beta;
    int best_move = 0;

    for (int i = 0; i < count; i++)
    {
        agent.do_move(moves[i]);
        // Print_BOARD();

        double t = (-1) * star1(agent, -n, -max(alpha, m), depth-1);
        // double t = (-1) * NegaScout(agent, dice, -n, -max(alpha, m), depth-1);
        if (t > m)
        {
            best_move = i;

            if (n == beta || depth < 3 || t >= beta)
                m = t;
            else
                m = (-1) * star1(agent, -beta, -t, depth-1);
                // m = (-1) * NegaScout(agent, dice, -beta, -t, depth-1);
        }

        agent.undo();
        // Print_BOARD();

        if (m >= beta)
            return moves[best_move];

        n = max(alpha, m) + 1;

    }

    return moves[best_move];
}

double NegaScout(EWN &agent, int dice, double alpha, double beta, int depth)
{
    // fprintf(stderr, "Depth %d\n", depth);
    if (depth == 0) {
        depth0_cnt++;
        return agent.get_state_value();
    }
    if (agent.is_over()) {    // TODO: and if time is running up
        // printf("Is over\n");
        // Print_BOARD();
        if (depth % 2 == 0)
            return agent.get_state_value();
        else
            return (-1) * agent.get_state_value();
    }

    int moves[MAX_MOVES];
    int count = agent.move_gen_all(moves, dice);

    double m = -INF_DIST;
    double n = beta;

    // checkout transition table
    // bool is_find_in_hash_table = false;
    // // Print_BOARD();
    // size_t hash_value = return_hash_value(agent.pos, moves, count);
    // // printf("hash_value %zu\n", hash_value);
    // if (hash_table.find(hash_value) != hash_table.end())
    // {
    //     hit_cnt++;
    //     // printf("hit!\n\n");
    //     is_find_in_hash_table = true;
    //     if (transition_table[hash_table[hash_value]][0] >= depth)
    //     {
    //         switch (transition_table[hash_table[hash_value]][2])
    //         {
    //             case EXACT:
    //                 return transition_table[hash_table[hash_value]][1];
    //             break;
    //             case UPPER:
    //                 alpha = max(alpha, transition_table[hash_table[hash_value]][1]);
    //                 if (alpha >= beta)
    //                     return alpha;
    //             break;
    //             case LOWER:
    //                 beta = min(beta, transition_table[hash_table[hash_value]][1]);
    //                 if (beta <= alpha)
    //                     return beta;
    //         }
    //     }
    // }

    for (int i = 0; i < count; i++)
    {
        agent.do_move(moves[i]);
        // Print_BOARD();

        double t = (-1) * star1(agent, -n, -max(alpha, m), depth-1);
        // printf("(Neg Scout) Get t %lf\n", t);
        // double t = (-1) * NegaScout(agent, dice, -n, -max(alpha, m), depth-1);
        if (t > m)
        {
            if (n == beta || depth < 3 || t >= beta)
                m = t;
            else
                m = (-1) * star1(agent, -beta, -t, depth-1);
                // m = (-1) * NegaScout(agent, dice, -beta, -t, depth-1);
        }

        agent.undo();
        // Print_BOARD();

        if (m >= beta || m == INF_DIST) {
            // int tt_status;
            // if (m == INF_DIST)
            //     tt_status = EXACT;
            // else
            //     tt_status = UPPER;
            
            // if (is_find_in_hash_table) {
            //     transition_table[hash_table[hash_value]][0] = depth;
            //     transition_table[hash_table[hash_value]][1] = m;
            //     transition_table[hash_table[hash_value]][2] = tt_status;
            // }
            // else {
            //     store_result_into_hash(hash_value, depth, m, tt_status);
            // }
            return m;
        }

        n = max(alpha, m) + 1;
    }

    // int tt_status;
    // if (m > alpha)
    //     tt_status = EXACT;
    // else
    //     tt_status = LOWER;

    // if (is_find_in_hash_table) {
    //     transition_table[hash_table[hash_value]][0] = depth;
    //     transition_table[hash_table[hash_value]][1] = m;
    //     transition_table[hash_table[hash_value]][2] = tt_status;
    // }
    // else {
    //     store_result_into_hash(hash_value, depth, m, tt_status);
    // }
    return m;
}

double star1(EWN &agent, double alpha, double beta, int depth)
{
    double A_prev = 6 * (alpha - WIN) + WIN;
    double B_prev = 6 * (beta - LOSE) + LOSE;

    double m_prev = LOSE, M_prev = WIN;

    double vsum = 0;

    for (int dice = 0; dice < 6; dice++)
    {
        // printf("< Star1 depth %d Choose dice %d\n", depth, dice);
        double t = NegaScout(agent, dice, max(A_prev, LOSE), min(B_prev, WIN), depth);
        m_prev += (t - LOSE) / 6;
        M_prev += (t - WIN) / 6;
        
        if (t >= B_prev)
            return m_prev;
        if (t <= A_prev)
            return M_prev;

        vsum += t;
        A_prev += (WIN - t);
        B_prev += (LOSE - t);
        // printf("> Star1 vsum %lf\n", vsum);
    }
    return vsum / 6;
}

/* ------------------ Transition Table ------------------ */

size_t return_hash_value(const int *pos, const int *moves, const int move_count) 
{
    size_t hash = 0;
    int i;
    for (i = 0; i < 12; i++)
        hash = (hash + (324723947 + pos[i])) ^ 93485734985;
    for (i = 0; i < move_count; i++)
        hash = (hash + (324723947 + moves[i])) ^ 93485734985;
    return hash;
}

void store_result_into_hash(size_t hash_value, int depth, int bound, int status)
{
    hash_table[hash_value] = transition_table_count;
    transition_table[transition_table_count][0] = depth;
    transition_table[transition_table_count][1] = bound;
    transition_table[transition_table_count][2] = status;
    transition_table_count++;
    // transition_table_count %= TT_SIZE;
}
