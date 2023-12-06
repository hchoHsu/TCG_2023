#ifndef MCTS_H
#define MCTS_H

#define ROW 6
#define COL 7
#define MAX_CUBES 6
#define RED 0
#define BLUE 1

#define PERIOD 21      // the period of the given dice sequence
#define MAX_PLIES 150  // the average ply of a game is far smaller than this number
#define MAX_MOVES 8

#define SIMULATION_PER_BRANCH 20
#define SIMULATION_PER_OVER_BRANCH 30

extern int self_color;
extern int enmy_color;

class EWN {
public:
    int pos[MAX_CUBES * 2];  // red cubes: 0~5, blue cubes: 6~11
    int num_cubes[2];
    int next;  // next player
    int self_color;

    int history[MAX_PLIES];
    int n_plies;

    int board[ROW * COL];

    void init_board();
    bool is_over();
    int move_gen_all(int *move_arr);
    void do_move(int move);
    void undo();
    void print_board();
};

class min_board {
public:
    int pos[MAX_CUBES * 2];  // red cubes: 0~5, blue cubes: 6~11
    int num_cubes[2];
    int next;  // next player
    int board[ROW * COL];
    int n_plies;

    bool is_over();
    int  is_over_rtresult();
    int  move_gen_all(int *move_arr);
    void do_move(int move);
    int  do_move_simulate(int move);
    void undo(int move);
    void copy(const min_board &game);
    void copy(const EWN &game);
    void print_board();
};

class node {
public:
    int move=0;  // move from parent

    int p_id=0;
    int c_id[MAX_MOVES];
    int depth=0;
    int Nchild=0;
    
    long long Ntotal=0;
    float CsqrtlogN=0;
    float sqrtN=0;
    long long sum1=0;
    float Average=0;
    float Variance=0;
};

int MCTS(EWN &game);

int FindPV(int ptr, min_board &pvb);
void expand(int &id, min_board &game);

void simulate(int &id, min_board &pvb);
void simulate_is_over(int &rtover);

void backpropagation(int ptr);
// void backpropagation_single(int ptr);

void update_nodes(int &id);
// void update_nodes(int &ptr, int &id);

inline float UCB(int &id);

int random_walk(min_board &game);
// int get_random_move(min_board &game);

int move_gen(int *move_arr, int cube, int location);
// int get_random_move(EWN &);

#endif
