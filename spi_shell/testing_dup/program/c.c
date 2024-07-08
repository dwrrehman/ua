// a dummy program that simply uses stdin and stdout
// to talk to the user in some interactive way.
// used for testing my pipe system in the single wire shell,
//    which is used on the riscv machine. 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>

int main(void) {

	puts("list:");
	puts("  sort_list : sort a list");
	puts("  delete_all : sort a list");
	puts("  insert_one : sort a list");
	puts("");
	printf("give function to run: ");

	char buffer[128] = {0};
	fgets(buffer, sizeof buffer, stdin);

	if (not strcmp(buffer, "sort_list\n")) {
		puts("sorting the list of elements...");

	} else if (not strcmp(buffer, "delete_all\n")) {
		puts("deleting all elements in the list...");
		puts("warning: this may take a while.");

	} else if (not strcmp(buffer, "insert_one\n")) {
		puts("inserting one element into the list...");
		printf("give the element: ");
		fgets(buffer, sizeof buffer, stdin);
		printf("deleting char: %c...\n", buffer[0]);
		puts("done");

	} else {
		puts("error: unknown function to run.");
		exit(1);
	}
	exit(0);
}


