// an example of pipes, and using dup and dup2 and fork and exec to look at commands output and use them for other things.

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <iso646.h>

int main(void) {
	int fd[2];
	pipe(fd);

	pid_t pid = fork();

	if (pid == 0) {

		dup2(fd[1], 1);
		close(fd[0]);
		close(fd[1]);
		execlp("ls", "ls", "-l", "../..", 0);
		fprintf(stderr, "Failed to execute \"%s\"\n", "ls");
		exit(1);

	} else {
		pid = fork();

		if (pid == 0) {

			dup2(fd[0], 0);
			close(fd[1]);
			close(fd[0]);
			execlp("wc", "wc", "-l", 0);
			fprintf(stderr, "Failed to execute \"%s\"\n", "wc");
			exit(1);
		} else {
			int status;
			close(fd[0]);
			close(fd[1]);
			waitpid(pid, &status, 0);
		}
	}
}

