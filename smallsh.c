//Sample file used as base//
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define INPUT_LENGTH 2048
#define MAX_ARGS 512


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
	return curr_command;
}

int main() {
    struct command_line *curr_command;

    while (true) {
        // Get the user input and parse it into a command structure
        curr_command = parse_input(); 	

        if (curr_command->argc > 0 && strcmp(curr_command->argv[0], "ls") == 0) {
            DIR *currDir = opendir(".");
            if (currDir == NULL) {
                perror("Unable to open directory");
                exit(EXIT_FAILURE);
            }

            struct dirent *entry;
            while ((entry = readdir(currDir)) != NULL) {
                printf("%s\n", entry->d_name);
            }
            closedir(currDir);
        }
	
	}

    return EXIT_SUCCESS;
}