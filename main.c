//headers of the C standart
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

//"sys" module headers
#include "sys/wait.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/mount.h"

//"linux" module headers
#include "linux/limits.h"
#include "linux/fs.h"
#include "linux/kernel.h"
//#include "linux/init.h"
#include "linux/module.h"
//#include "linux/pagemap.h"

//other unix-system feature modules
#include "unistd.h"
#include "pwd.h"
#include "dirent.h"
#include "signal.h"

//"defcoms" is a header file that includes all built-in shell commands
#include "defcoms.h"

enum ERR {
	BUFF_MEM_ALLOC_ERROR,
	BUFF_ERROR
};

void AllocCheck(char* buffer) {
	if (!buffer) {
		fprintf(stderr, "BUFF_MEM_ALLOC_ERROR");
		exit(1);
	}
}

char* ReadLine(void) {
	char* line = nullptr;
	size_t buffsize = 0;
	getline(&line, &buffsize, stdin);	
	return line;
}

char** Tokenise(char* line) {
	size_t buffsize = TOKEN_BUFFSIZE;
	int pos = 0;
	char **tokens = malloc(buffsize * sizeof(char*));
	char* token;

	AllocCheck(token);

	const char* strippers = " \t\n\a\r";
	token = strtok(line, strippers);
	while (token != nullptr) {
		tokens[pos] = token;
		pos++;

		if (pos >= buffsize) {
			buffsize += TOKEN_BUFFSIZE;

			tokens = realloc(tokens, buffsize * sizeof(char*));
			AllocCheck(tokens[0]);
		}

		token = strtok(nullptr, strippers);
	}

	tokens[pos] = nullptr;
	return tokens;
}

int Execute(char** args) {
	/*printf("\n");
	for (int i = 0; args[i] != nullptr; i++) {
		printf(" |%s| ", args[i]);
	}
	printf("\n");
	*/

	if (args[0] == nullptr) return 1;

	for (int i = 0; i < DefNum(); i++) {
		if (strcmp(args[0], cmdList[i].cmdName) == 0) {
			return (*cmdList[i].cmdFunc)(args);
		}
	}

	return LinkProcess(args);
}

void HandleCmd() {
	char *line;
	char** args;
	int status = 1;

	char cwd[PATH_MAX];

	while (status) {
		printf("\033[1;36m%s\033[0m > ", getcwd(cwd, sizeof(cwd)));

		line = ReadLine();

		if (feof(stdin)) {
			printf("\n");
			return;
		}
		
		args = Tokenise(line);
		status = Execute(args);

		free(line); //вот из-за этого и не работает хистори
		free(args);
	}
}

void StarterArt(char* filename) {
	int width = 120, height  = 15;

	FILE* file = fopen(filename, "r");

	if (!file) {
		printf("\nSad, but there is no art file loaded :(\nWelcome by the way!\n\n");
		return;
	}

	char sym = fgetc(file);
	int i = 0;
	while (sym != EOF && i < width * height) {
		printf("%c", sym);
		sym = fgetc(file);
		i++;
	}

	printf("\n");
	fclose(file);
}

int main(int argc, char **argv) {
	printf("%s process id: \033[1;36m%d\033[0m\n", SHELL_NAME, getpid());	
	
	struct passwd* info = getpwuid(geteuid());
	printf("User id: %d\n", info->pw_uid);
		
	signal(SIGHUP, CatchSighup);
	signal(SIGINT, CatchSigint);

	StarterArt("result.txt");

	HandleCmd();
	
	umount(CRONFS_TARGET);
	return 0;
}
