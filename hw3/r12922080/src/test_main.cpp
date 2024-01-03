#include <stdio.h>
#include <string.h>
#include "Agent.h"

static const int dir_val[2][4] = {{1, COL, COL+1}, {-1, -COL, -COL-1}};

int main()
{
    EWN agent;
    agent.num_cubes[RED] = 6;
    agent.pos[0] = 2;
    agent.pos[1] = 6;
    agent.pos[2] = 1;
    agent.pos[3] = 0;
    agent.pos[4] = 10;
    agent.pos[5] = 5;

    agent.num_cubes[BLUE] = 6;
    agent.pos[6] = 23;
    agent.pos[7] = 19;
    agent.pos[8] = 18;
    agent.pos[9] = 22;
    agent.pos[10] = 24;
    agent.pos[11] = 14;

    agent.next = BLUE;
    agent.init_board();

    int dice_seq[50] = {5, 1, 2, 4, 5, 0, 0, 4, 5, 2, 
                        5, 3, 2, 5, 3, 4, 1, 4, 2, 3, 
                        3, 1, 0, 0, 2, 0, 4, 5, 2, 5,
                        3, 4, 1, 0, 0, 4, 2, 3, 1, 5,
                        2, 2, 4, 4, 5, 1, 3, 0, 1, 0};
    // printf("input dice:\n");
    // scanf("%d", &dice);
    for (int i = 0; i < 50; i++)
    {
        int dice = dice_seq[i];
        // fprintf(stderr, "Dice %d\n", dice);
        int move = search_and_get_move_new(agent, dice);
        int cube = (move) >> 4;
        int direction = move & 0xf;
        int src = agent.pos[cube] + dir_val[agent.next][direction];
        // fprintf(stderr, "Final: %d to %d\n", cube, src);
        printf("Step %2d Dice %2d: Final: %d to %d\n", i, dice, cube, src);
        fprintf(stderr, "Step %2d Dice %2d: Final: %d to %d\n", i, dice, cube, src);
        agent.do_move(move);
        if (agent.is_over())
            break;
    }

    return 0;
}