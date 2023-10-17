#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include "ewn.h"

#define max(i,j) i>j?i:j

// these variables are available after calling EWN::scan_board()
int ROW;
int COL;
int PERIOD;
static int dir_value[8];

EWN::EWN() {
    row = 0;
    col = 0;
    pos[0] = 999;
    pos[MAX_PIECES + 1] = 999;
    for (int i = 1; i <= MAX_PIECES; i++) {
        pos[i] = -1;
    }
    period = 0;
    goal_piece = 0;
    
    n_plies = 0;
    step_need = calc_step_need();
}

void set_dir_value() {
    dir_value[0] = -COL - 1;
    dir_value[1] = -COL;
    dir_value[2] = -COL + 1;
    dir_value[3] = -1;
    dir_value[4] = 1;
    dir_value[5] = COL - 1;
    dir_value[6] = COL;
    dir_value[7] = COL + 1;
}

void EWN::scan_board() {
    scanf(" %d %d", &row, &col);
    for (int i = 0; i < row * col; i++) {
        scanf(" %d", &board[i]);
        if (board[i] > 0)
            pos[board[i]] = i;
    }
    scanf(" %d", &period);
    for (int i = 0; i < period; i++) {
        scanf(" %d", &dice_seq[i]);
    }
    scanf(" %d", &goal_piece);

    // initialize global variables
    ROW = row;
    COL = col;
    PERIOD = period;
    set_dir_value();
    // printf("hhh");
    calc_step_need();
}

void EWN::print_board() {
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            printf("%4d", board[i * col + j]);
        }
        printf("\n");
    }
    printf("%d,%d\n", n_plies, step_need);
}

int EWN::print_history() {
    printf("%d\n", n_plies);
    int piece, dir;
    for (int i = 0; i < n_plies; i++) {
        piece = (history[i] & 255) >> 4;
        dir = history[i] & 15;
        printf("%d %d\n", piece, dir);
    }
    return 0;
}

bool EWN::is_goal() {
    if (goal_piece == 0) {
        if (board[row * col - 1] > 0) return true;
    }
    else {
        if (board[row * col - 1] == goal_piece) return true;
    }
    return false;
}

/*
move: an integer using only 12 bits
3~0: store the direction
7~4: store the piece number
11~8: store the eaten piece (used only in history)
*/

int move_gen2(int *moves, int piece, int location) {
    int count = 0;
    int row = location / COL;
    int col = location % COL;

    bool left_ok = col != 0;
    bool right_ok = col != COL - 1;
    bool up_ok = row != 0;
    bool down_ok = row != ROW - 1;

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
        move |= board[dst] << 8;
    }
    board[pos[piece]] = 0;
    board[dst] = piece;
    pos[piece] = dst;
    history[n_plies++] = move;

    calc_step_need(); // TODO
}

void EWN::undo() {
    if (n_plies == 0) {
        fprintf(stderr, "no history\n");
        exit(1);
    }

    int move = history[--n_plies];
    int eaten_piece = move >> 8;
    int piece = (move & 255) >> 4;
    int direction = move & 15;
    int dst = pos[piece] - dir_value[direction];

    if (eaten_piece > 0) {
        board[pos[piece]] = eaten_piece;
        pos[eaten_piece] = pos[piece];
    }
    else board[pos[piece]] = 0;
    board[dst] = piece;
    pos[piece] = dst;

    calc_step_need(); // TODO
}

int EWN::calc_step_need() {
    int tmp = 0;
    step_need = MAX_MOVES + 1;
    // target shortest distance
    if (goal_piece != 0) {
        if (pos[goal_piece] < 0) {
            return step_need;
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
    // TODO: target valid moves

    int target_valid_moves = 0;
    for (int i = 0; i < ; i++) {
        if (pos[dice_seq[i]] == -1) {

        }
        else {
            
        }
    }

    return step_need;
}

size_t EWN::calc_hash() {
    std::hash<int> hasher;
    size_t seed = 0;
    seed ^= hasher(pos[1]) + 0x9e3779b9 + (seed<<6) + (seed >> 2);
    seed ^= hasher(pos[2]) + 0x9e3779b9 + (seed<<6) + (seed >> 2);
    seed ^= hasher(pos[3]) + 0x9e3779b9 + (seed<<6) + (seed >> 2);
    seed ^= hasher(pos[4]) + 0x9e3779b9 + (seed<<6) + (seed >> 2);
    seed ^= hasher(pos[5]) + 0x9e3779b9 + (seed<<6) + (seed >> 2);
    seed ^= hasher(pos[6]) + 0x9e3779b9 + (seed<<6) + (seed >> 2);
    // TODO: remove n_plies from hash
    seed ^= hasher(n_plies) + 0x9e3779b9 + (seed<<6) + (seed >> 2);
    return seed;
}
