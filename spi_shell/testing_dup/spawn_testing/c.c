// an example using posix spawn that i found online, 
// for doing shell stuff basically. without fork.

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <spawn.h>

int main(int argc, char* argv[], char* env[]) {

    posix_spawn_file_actions_t actions;

    int ret = posix_spawn_file_actions_init(&actions);
    if (ret) { 
	perror("posix_spawn");
	exit(1);
    }

    ret = posix_spawn_file_actions_addopen(&actions, 1, "./foo-log", O_WRONLY | O_CREAT | O_EXCL | O_TRUNC, 0644);
    if (ret) {
        perror("posix_spawn");
	exit(1);
    }
    ret = posix_spawn_file_actions_adddup2(&actions, 1, 2);
    if (ret) {
        perror("posix_spawn");
	exit(1);
    }

    pid_t pid;
    ret = posix_spawn(&pid, "/bin/date", &actions, NULL, argv, env);
    if (ret) {
        puts(strerror(ret));
	exit(1);
    }
}



