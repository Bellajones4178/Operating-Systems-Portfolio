#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define INPUT_LENGTH 2048
#define MAX_ARGS 512
#define BG_PROC 100

int last_fg_status = 0;
pid_t bg_pids[BG_PROC];
int bg_num = 0;


struct command_line
{
	char *argv[MAX_ARGS + 1];
	int argc;
	char *input_file;
	char *output_file;
	bool is_bg;
};


struct command_line *parse_input()
{
	char input[INPUT_LENGTH];
	struct command_line *curr_command = (struct command_line *) calloc(1, sizeof(struct command_line));

	// Get input
	printf(": ");
	fflush(stdout);
	fgets(input, INPUT_LENGTH, stdin);

	// Tokenize the input
	char *token = strtok(input, " \n");
	while(token){
		if (token[0] == '#') {
			break;
		} else if (!strcmp(token,"<")){
			curr_command->input_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,">")){
			curr_command->output_file = strdup(strtok(NULL," \n"));
		} else if(!strcmp(token,"&")){
			curr_command->is_bg = true;
		} else{
			curr_command->argv[curr_command->argc++] = strdup(token);
		}
		token=strtok(NULL," \n");
	}
	curr_command->argv[curr_command->argc] = NULL;
	return curr_command;
}

int main() {
    struct command_line *curr_command;

    while (true) {
        // Run background proceses
        for (int i = 0; i < bg_num;) {
            int status;
            pid_t result = waitpid(bg_pids[i], &status, WNOHANG);
            if (result > 0) {
                if (WIFEXITED(status)) {
                    printf("background pid %d is done: exit value %d\n", result, WEXITSTATUS(status));
                } else if (WIFSIGNALED(status)) {
                    printf("background pid %d is done: terminated by signal %d\n", result, WTERMSIG(status));
                }
                fflush(stdout);

                // Remove from list
                for (int j = i; j < bg_num - 1; j++) {
                    bg_pids[j] = bg_pids[j + 1];
                }
                bg_num--;
            } else {
                i++;
            }
        }

        // Get user input and process command
        curr_command = parse_input();

        if (curr_command->argc == 0) {
            continue;
        }

        if (strcmp(curr_command->argv[0], "exit") == 0) {
            exit(0);
        } else if (strcmp(curr_command->argv[0], "cd") == 0) {
            char *target_dir = curr_command->argc == 1 ? getenv("HOME") : curr_command->argv[1];
            chdir(target_dir);
        } else if (strcmp(curr_command->argv[0], "status") == 0) {
            if (WIFEXITED(last_fg_status)) {
                printf("exit value %d\n", WEXITSTATUS(last_fg_status));
            } else if (WIFSIGNALED(last_fg_status)) {
                printf("terminated by signal %d\n", WTERMSIG(last_fg_status));
            }
        } else {
            pid_t spawnpid = fork();
            if (spawnpid == 0) {
                // Redirection of Input
                if (curr_command->input_file != NULL) {
                    int fd = open(curr_command->input_file, O_RDONLY);
                    if (fd == -1) {
                        perror("input redirection");
                        exit(1);
                    }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }

                // Redirection of Output
                if (curr_command->output_file != NULL) {
                    int fd = open(curr_command->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd == -1) {
                        perror("output redirection");
                        exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }

                execvp(curr_command->argv[0], curr_command->argv);
                perror("execvp");
                exit(1);
            } else {
                if (curr_command->is_bg) {
                    printf("background pid is %d\n", spawnpid);
                    fflush(stdout);
                    if (bg_num < BG_PROC) {
                        bg_pids[bg_num++] = spawnpid;
                    }
                } else {
                    waitpid(spawnpid, &last_fg_status, 0);
                }
            }
        }
    }

    return 0;
}
