#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

int bgstat = 0;
char* infile;
char* outfile;
char* cmds[512] = { NULL };

void BgCommands() {
    pid_t pid;
    int status;

    while ((pid = waitpid(0, &status, WUNTRACED | WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            printf("Child process %jd done. Exit status %d.\n", (intmax_t)pid, WEXITSTATUS(status));
        }
        if (WIFSTOPPED(status)) {
            kill(pid, SIGCONT);
            printf("Child process %jd stopped. Continuing.\n", (intmax_t)pid);
        }
        if (WIFSIGNALED(status)) {
            printf("Child process %jd done. Signaled %d.\n", (intmax_t)pid, WTERMSIG(status));
        }
    }
}

void Parse(int num, char** cmd) {
    int j = 0, i = 0;
    while (num > 0) {
        if (strcmp(cmd[num - 1], "#") == 0) {
            j = num - 2;
            cmd[num - 1] = NULL;
            i--;
        }
        num--;
        i++;
    }

    if (j > 0 || i > 0) {
        if (j <= 0) {
            j = i - 1;
        }
        if (strcmp(cmd[j], "&") == 0) {
            cmd[j] = NULL;
            bgstat = 1;
            j--;
        }
        if (j > 2) {
            if (strcmp(cmd[j - 1], "<") == 0) {
                infile = cmd[j];
                cmd[j] = NULL;
                cmd[j - 1] = NULL;
                if (j > 4) {
                    if (strcmp(cmd[j - 3], ">") == 0) {
                        outfile = cmd[j - 2];
                        cmd[j - 2] = NULL;
                        cmd[j - 3] = NULL;
                    }
                }
            }
            else if (strcmp(cmd[j - 1], ">") == 0) {
                outfile = cmd[j];
                cmd[j] = NULL;
                cmd[j - 1] = NULL;
                if (j > 4) {
                    if (strcmp(cmd[j - 3], "<") == 0) {
                        infile = cmd[j - 2];
                        cmd[j - 2] = NULL;
                        cmd[j - 3] = NULL;
                    }
                }
            }
        }
    }
}

char* str_gsub(char* restrict* restrict haystack, char const* restrict needle, char const* restrict sub) //Re : String search and replace tutorial video
{
    char* str = *haystack;
    size_t haystack_len = strlen(str);
    size_t const needle_len = strlen(needle),
        sub_len = strlen(sub);

    for (; (str = strstr(str, needle));) {
        ptrdiff_t off = str - *haystack;
        if (sub_len > needle_len) {
            str = realloc(*haystack, sizeof * *haystack * (haystack_len + sub_len - needle_len + 1));
            if (!str) goto exit;
            *haystack = str;
            str = *haystack + off;
        }
        memmove(str + sub_len, str + needle_len, haystack_len + 1 - off - needle_len);
        memcpy(str, sub, sub_len);
        haystack_len = haystack_len + sub_len - needle_len;
        str += sub_len;
    }
    str = *haystack;
    if (sub_len < needle_len) {
        str = realloc(*haystack, sizeof * *haystack * (haystack_len + 1));
        if (!str) goto exit;
        *haystack = str;
    }

exit:
    return str;
}

int main(void) {

    char* line = NULL;
    size_t n = 0;
    int childStatus = 0;
    char* background = "";
    char* foreground = "";
    int Sfile;
    int Tfile;

    for (;;) {
    start:

        bgstat = 0;
        infile = NULL;
        outfile = NULL;

        for (int n = 0; n < 512; n++) {
            cmds[n] = NULL;
        }

        BgCommands();

        char* PS1;
        if ((PS1 = getenv("PS1")) == NULL) {
            PS1 = "";
        }
        fprintf(stderr, "%s", PS1);

        ssize_t line_length = getline(&line, &n, stdin);
        if (feof(stdin) != 0) {
            fprintf(stderr, "\nexit\n");
            exit((int)*foreground);
        }

        if (getenv("IFS") == NULL) {
            setenv("IFS", " \t\n", 1);
        }
        char* IFS = (getenv("IFS"));

        int i = 0;
        line = strtok(line, IFS); //https://www.man7.org/linux/man-pages/man1/bash.1.html
        while (line != NULL) {
            cmds[i] = strdup(line);
            i++;
            line = strtok(NULL, IFS);
        }

        int cmdnum = 0;
        char* PID = malloc(sizeof(int) * 8);
        sprintf(PID, "%d", getpid());
        while (i > 0) {
            if (cmds[cmdnum] != NULL) {
                if (strncmp(cmds[cmdnum], "~/", 2) == 0) {
                    str_gsub(&cmds[cmdnum], "~", getenv("HOME"));
                }
                str_gsub(&cmds[cmdnum], "$$", PID);
                str_gsub(&cmds[cmdnum], "$?", foreground);
                str_gsub(&cmds[cmdnum], "$!", background);
            }
            i--;
            cmdnum++;
        }

        Parse(cmdnum, cmds);

        if (cmds[0] == NULL) {
            goto start;
        }
        else if (cmds[0] != NULL) {
            if (strcmp(cmds[0], "exit") == 0) {
                if (cmds[1] == NULL) {
                    fprintf(stderr, "\nexit\n");
                    exit((int)*foreground);
                }
                fprintf(stderr, "\nexit\n");
                exit(atoi(cmds[1]));
            }
            else if (strcmp(cmds[0], "cd") == 0) {
                if (cmds[1] == NULL) {
                    cmds[1] = getenv("HOME");
                    strcat(cmds[1], "/");
                }
                chdir(cmds[1]);
                goto start;
            }
        }

        pid_t spawnPid = fork();

        switch (spawnPid) {
        case -1:
            break;
        case 0:
            if (infile != NULL) {
                Sfile = open(infile, O_RDONLY);
                dup2(Sfile, 0);
            }
            if (outfile != NULL) {
                Tfile = open(outfile, O_WRONLY | O_TRUNC | O_CREAT, 0777);
                dup2(Tfile, 1);
            }
            execvp(cmds[0], cmds);
            if (infile != NULL) {
                close(Sfile);
            }
            if (outfile != NULL) {
                close(Tfile);
            }
            exit(0);
            break;
        default:
            if (bgstat == 0) {
                spawnPid = waitpid(spawnPid, &childStatus, 0);
                foreground = malloc(10 * sizeof(int));
                sprintf(foreground, "%d", WEXITSTATUS(childStatus));
            }
            else {
                background = malloc(10 * sizeof(int));
                sprintf(background, "%d", spawnPid);
            }
            goto start;
        }
    }
    return 0;
}
