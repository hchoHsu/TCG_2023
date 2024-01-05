#ifndef EWN_H
#define EWN_H

#define ROW 5
#define COL 5
#define MAX_CUBES 6
#define RED 0
#define BLUE 1

#define PERIOD 21      // the period of the given dice sequence
#define MAX_PLIES 150  // the average ply of a game is far smaller than this number
#define MAX_MOVES 8

#define DEPTH_LIMIT 8
#define TT_SIZE 100000

#define EXACT 0
#define UPPER 1
#define LOWER -1

#define DIS_WEIGHT 2
#define DET_WEIGHT 1

void Print_BOARD();

class EWN {
public:
    int pos[MAX_CUBES * 2];  // red cubes: 0~5, blue cubes: 6~11
    int num_cubes[2];
    int next;  // next player

    int dice_seq[DEPTH_LIMIT + 1];
    int history[60];
    int n_plies;

    void init_board();
    bool is_over();
    int is_over_rtresult();
    int move_gen_all(int *move_arr, const int dice);
    void do_move(int move);
    void undo();

    int greedy();
    // int heuristic();

    inline int pos_shortest_distance(int idx);
    double get_state_value();

    friend int search(EWN &, int alpha, int beta, int depth);
};

int search_and_get_move(EWN &, int depth);
int search_and_get_move_new(EWN &, int dice);
int get_random_move(EWN &);

int NegaScout_Start(EWN &agent, int dice, double alpha, double beta, int depth);
double NegaScout(EWN &agent, int dice, double alpha, double beta, int depth);
double star1(EWN &agent, double alpha, double beta, int depth);

size_t return_hash_value(const int *pos, const int *moves, const int move_count);
void store_result_into_hash(size_t hash_value, int depth, int bound, int status);

#endif
