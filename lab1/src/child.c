#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

int main() {
    char buf[4096];
    ssize_t n;
    size_t pos = 0;

    while ((n = read(STDIN_FILENO, buf + pos, sizeof(buf) - pos - 1)) > 0) {
        pos += n;
        buf[pos] = '\0';

        char *cur_line = buf;
        char *tmp_line;

        while ((tmp_line = strchr(cur_line, '\n'))) {
            *tmp_line = '\0';

            int only_spaces = 1;
            for (char *p = cur_line; *p; p++) {
                if (!isspace((unsigned char)*p)) {
                    only_spaces = 0;
                    break;
                }
            }
            if (only_spaces) {
                cur_line = tmp_line + 1;
                continue;
            }

            int sum = 0;
            int invalid_input = 0;
            char *p = cur_line;

            while (*p) {
                while (*p && isspace((unsigned char)*p)) p++;
                if (!*p) break;

                int is_neg = 1;
                if (*p == '-') { is_neg = -1; p++; }

                if (!isdigit((unsigned char)*p)) {
                    invalid_input = 1;
                    break;
                }

                int num = 0;
                while (*p && isdigit((unsigned char)*p)) {
                    num = num * 10 + (*p - '0');
                    p++;
                }
                if (*p && !isspace((unsigned char)*p)) {
                    invalid_input = 1;
                    break;
                }
                sum += num * is_neg;

                while (*p && !isspace((unsigned char)*p)) p++;
            }

            if (invalid_input) {
                const char msg[] = "error: invalid input\n";
                write(STDERR_FILENO, msg, sizeof(msg) - 1);
            } else {
                char out[64];
                int len = snprintf(out, sizeof(out), "%d\n", sum);
                write(STDOUT_FILENO, out, len);
            }

            cur_line = tmp_line + 1;
        }

        if (*cur_line) {
            int only_spaces = 1;
            for (char *p = cur_line; *p; p++) {
                if (!isspace((unsigned char)*p)) {
                    only_spaces = 0;
                    break;
                }
            }
            if (!only_spaces) {
                int sum = 0;
                int invalid_input = 0;
                char *p = cur_line;

                while (*p) {
                    while (*p && isspace((unsigned char)*p)) p++;
                    if (!*p) break;

                    int is_neg = 1;
                    if (*p == '-') { is_neg = -1; p++; }

                    if (!isdigit((unsigned char)*p)) {
                        invalid_input = 1;
                        break;
                    }

                    int num = 0;
                    while (*p && isdigit((unsigned char)*p)) {
                        num = num * 10 + (*p - '0');
                        p++;
                    }

                    sum += num * is_neg;

                    while (*p && !isspace((unsigned char)*p)) p++;
                }

                if (invalid_input) {
                    const char msg[] = "error: invalid input\n";
                    write(STDERR_FILENO, msg, sizeof(msg) - 1);
                } else {
                    char out[64];
                    int len = snprintf(out, sizeof(out), "%d\n", sum);
                    write(STDOUT_FILENO, out, len);
                }
            }
        }

        pos = 0;
    }

    return 0;
}
