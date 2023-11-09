#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include "ewn.h"

#define TIMEOUT -2
#define WA -1

int pid;

void Redir_Stdin(const char *testcase) {
    int fd;
    if ((fd = open(testcase, O_RDONLY)) == -1) {
        fprintf(stderr, "open failed: %s: %s\n", strerror(errno), testcase);
        exit(1);
    }
    if (dup2(fd, 0) == -1) {
        perror("dup2 failed");
        exit(1);
    }
    close(fd);
}

int Exec_EWN_Program(const char *program) {
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("pipe failed");
        exit(1);
    }

    int pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(1);
    } 
    else if (pid == 0) {
        lseek(0, 0, SEEK_SET);
        if (dup2(pipe_fd[1], 1) == -1) {
            perror("dup2 failed");
            exit(1);
        }
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        execl(program, program, NULL);
        fprintf(stderr, "exec failed: %s: %s\n", strerror(errno), program);
        exit(1);
    }
    else {
        if (dup2(pipe_fd[0], 0) == -1) {
            perror("dup2 failed");
            exit(1);
        }
        close(pipe_fd[0]);
        close(pipe_fd[1]);
    }

    return pid;
}

int Testing(EWN &game) {
    int moves[MAX_MOVES];
    int n_move;
    int n_ply;

    scanf(" %d", &n_ply);
    for (int i = 0; i < n_ply; i++) {
        int piece, direction;
        scanf(" %d %d", &piece, &direction);
        int move = piece << 4 | direction;
        bool legal = false;

        n_move = game.move_gen_all(moves);
        for (int j = 0; j < n_move; j++) {
            if (moves[j] == move) {
                legal = true;
                break;
            }
        }
        if (!legal) return WA;
        game.do_move(move);
    }
    if (!game.is_goal()) return WA;

    return n_ply;
}

void alarm_handler(int signum) {
    if (signum != SIGALRM) fprintf(stderr, "???\n");

    // kill the child process
    if (kill(pid, SIGKILL) == -1) {
        fprintf(stderr, "failed to kill the process with pid %d: %s\n", pid, strerror(errno));
    }
    printf("%d", TIMEOUT);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 3) exit(1);

    EWN game;
    Redir_Stdin(argv[2]);
    game.scan_board();

    if (signal(SIGALRM, alarm_handler) == SIG_ERR) {
        perror("signal failed");
        exit(1);
    }
    alarm(5);
    pid = Exec_EWN_Program(argv[1]);
    wait(nullptr);
    alarm(0);

    printf("%d", Testing(game));
    return 0;
}
