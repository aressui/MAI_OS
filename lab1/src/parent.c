#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    char progpath[1024];
    {
        const char msg[] = "input file: ";
        write(STDOUT_FILENO, msg, sizeof(msg) - 1);

        ssize_t n = read(STDIN_FILENO, progpath, sizeof(progpath) - 1);
        if (n <= 0) {
            const char msg[] = "error: can't read the file\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            exit(EXIT_FAILURE);
        }
        progpath[n - 1] = '\0';
    }

    int child_to_parent[2];
    if (pipe(child_to_parent) == -1) {
        const char msg[] = "error: can't make the pipe\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    switch(pid) {
        case -1: {
            const char msg[] = "error: can't make a new process\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            exit(EXIT_FAILURE);
        } break;

        case 0: {
            close(child_to_parent[0]);

            int file = open(progpath, O_RDONLY);
            if (file == -1) {
                const char msg[] = "error: can't open file\n";
                write(STDERR_FILENO, msg, sizeof(msg) - 1);
                exit(EXIT_FAILURE);
            }

            dup2(file, STDIN_FILENO);
            close(file);

            dup2(child_to_parent[1], STDOUT_FILENO);
            close(child_to_parent[1]);

            execl("./child", "child", NULL);
            const char msg[] = "error: can't exec child\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            exit(EXIT_FAILURE);
        } break;

        default: {
            close(child_to_parent[1]);

            char buf[4096];
            ssize_t n;
            while ((n = read(child_to_parent[0], buf, sizeof(buf))) > 0) {
                write(STDOUT_FILENO, buf, n);
            }

            close(child_to_parent[0]);
            wait(NULL);
        } break;
    }

    return 0;
}
