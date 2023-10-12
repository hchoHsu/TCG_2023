#ifndef EWN_H
#define EWN_H

#include <functional>

#define MAX_ROW 9
#define MAX_COL 9
#define MAX_PIECES 6
#define MAX_PERIOD 18
#define MAX_PLIES 100
#define MAX_MOVES 16

extern int ROW;
extern int COL;
extern int PERIOD;

class EWN{
private:
    int row, col;
    int board[MAX_ROW * MAX_COL];
    int dice_seq[MAX_PERIOD];
    int period;
    int goal_piece;

    int history[MAX_PLIES];

    void sort_move(int *moves, int n_move);

public:
    int pos[MAX_PIECES + 2];  // pos[0] and pos[MAX_PIECES + 1] are not used
    int n_plies;
    int step_need;

    EWN();
    void scan_board();
    void print_board();
    bool is_goal();
    int print_history();

    int move_gen_all(int *moves);
    void do_move(int move);
    void undo();

    int heuristic();
    int heuristic2();

    int calc_step_need();
};

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
        seed ^= hasher(cur.n_plies) + 0x9e3779b9 + (seed<<6) + (seed >> 2);
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
               lhs.pos[6] == rhs.pos[6] &&
               lhs.n_plies == rhs.n_plies;
    }
};

#endif
