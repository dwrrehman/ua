#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <iso646.h>
#include <stdbool.h>

typedef uint64_t nat;
typedef uint8_t byte;

int main(const int argc, const char** argv) {

	const char* output_filename = "generated/partition_output_file__section_";

	if (argc != 2) return puts("usage: ./run <zlist_z.txt file>");


	nat text_length = 0;
	char* text = NULL;

	{ const char* filename = argv[1];
	int file = open(filename, O_RDONLY);
	if (file < 0) { perror("open: read: "); exit(1); }
	text_length = (nat) lseek(file, 0, SEEK_END);
	lseek(file, 0, SEEK_SET);
	text = calloc(text_length + 1, 1);
	read(file, text, text_length);
	close(file);
	}
	
	char** lines = NULL;
	nat line_count = 0;

	nat line_length = 0;
	nat line_start = 0;

	for (nat i = 0; i < text_length; i++) {
		const char c = text[i];	
		
		if (c == 10) {			
			char* line = strndup(text + line_start, line_length + 1);
			lines = realloc(lines, sizeof(char*) * (line_count + 1));
			lines[line_count++] = line;
			line_length = 0;
			line_start = i + 1;

		} else {
			line_length++;
		}
	}

	for (nat i = 0; i < 100; i++) {
		printf("#%llu: line = \"%s\"\n", i, lines[i]);
	}
	puts("");



	char** filenames = calloc(10, sizeof(char*));
	for (nat i = 0; i < 10; i++) filenames[i] = calloc(4096, 1);
	for (nat i = 0; i < 10; i++) snprintf(filenames[i], 4096, "%s%llu_z.txt", output_filename, i);
	
	char*** file_data = calloc(10, sizeof(char**));
	nat* file_data_count = calloc(10, sizeof(nat));

	{ const nat max_per_file = line_count / 10 + 1;

	printf("info: max_per_file = %llu\n", max_per_file);
	getchar();

	nat file_index = 0;
	for (nat i = 0; i < line_count; i++) {

		printf("\r %5llu: { %5llu, %5llu, %5llu, %5llu, %5llu, %5llu, %5llu, %5llu, %5llu, %5llu }", 
			i, file_data_count[0], 
			file_data_count[1], 
			file_data_count[2], 
			file_data_count[3], 
			file_data_count[4], 
			file_data_count[5], 
			file_data_count[6], 
			file_data_count[7], 
			file_data_count[8], 
			file_data_count[9]
			); fflush(stdout);
			//usleep(3000);

		file_data[file_index] = realloc(file_data[file_index], sizeof(char*) * (file_data_count[file_index] + 1)); 
		file_data[file_index][file_data_count[file_index]++] = lines[i];
		if (file_data_count[file_index] >= max_per_file) file_index++;
	} } 

	for (nat i = 0; i < 10; i++) {
		printf("file #%llu: .file_data_count = %llu\n", i, file_data_count[i]); 
		puts(".lines: {");
		for (nat l = 0; l < 100; l++) {
			printf("\tline[%llu] = \"%s\"\n", l, file_data[i][l]);
		}
		puts("}");
	}

	printf("writing data to 10 files...\n");
	for (nat file_index = 0; file_index < 10; file_index++) {
		int file = open(filenames[file_index], O_WRONLY | O_CREAT | O_EXCL | O_APPEND, 0666);
		if (file < 0) { perror("open: write: "); exit(1); } 
		for (nat i = 0; i < file_data_count[file_index]; i++) {
			write(file, file_data[file_index][i], strlen(file_data[file_index][i]));
		} 
		close(file);
	}
	puts("done.");
}




























