import os
import sys
from resource import setrlimit, RLIMIT_AS

TIMEOUT = -2
WA = -1

if __name__ == "__main__":

    if len(sys.argv) != 2:
        print("usage: python3 grading.py your_program", file=sys.stderr)
        sys.exit(1)
    program = sys.argv[1]

    # set the limit of memory usage
    memory_limit = 1 << 30  # 1G
    setrlimit(RLIMIT_AS, (memory_limit, memory_limit))

    # grading
    score = [0, 0, 0, 0]
    testcases_num = [0, 7, 7, 6]
    for level in [1, 2, 3]:
        for i in range(testcases_num[level]):
            pipe_fd = os.pipe()
            pid = os.fork()
            if pid == 0:
                os.dup2(pipe_fd[1], 1)
                os.close(pipe_fd[0])
                os.close(pipe_fd[1])
                testcase = f"testcases/{level}{i+1}.in"
                os.execl("./testing", "./testing", program, testcase)
                print("exec failed", file=sys.stderr)
                sys.exit(1)
            else:
                os.close(pipe_fd[1])
                with open(f"testcases/{level}{i+1}.out", "r") as fo: 
                    best_answer = int(fo.readline())
                answer = int(os.read(pipe_fd[0], 3).decode())
                os.close(pipe_fd[0])
                os.wait()

                if answer == TIMEOUT:
                    print(f"{level}{i+1}.in: timeout")
                elif answer == WA:
                    print(f"{level}{i+1}.in: wrong answer")
                else:
                    print(f"{level}{i+1}.in: {best_answer}/{answer}")
                    score[level] += best_answer / answer

    for i in [1, 2, 3]:
        score[i] *= 4
        print("level {}, score: {:.2f}".format(i, score[i]))
    print("total score: {:.2f}".format(sum(score)))
