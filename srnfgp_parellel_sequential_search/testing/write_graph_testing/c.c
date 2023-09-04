#include <stdio.h>
#include <stdlib.h>   
#include <string.h>   
#include <unistd.h>   // a unit test for the write_graph portion of the srnfgp algorithm.
#include <iso646.h> 
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h> 
#include <termios.h>

typedef int8_t byte;
typedef uint64_t nat;

static const nat max_buffer_count = 100;

struct item {
	char z[64];
	char dt[32];
};

static byte* graph = NULL;

static byte graph_count = 20;

static nat buffer_count = 0;

static struct item buffer[max_buffer_count] = {0};

static char directory[4096] = "./";
static char filename[4096] = {0};


static void get_datetime(char datetime[32]) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* tm_info = localtime(&tv.tv_sec);
	strftime(datetime, 32, "1%Y%m%d%u.%H%M%S", tm_info);
}

static void print_graph(void) { for (byte i = 0; i < graph_count; i++) printf("%hhu", graph[i]); putchar(10); }

static void get_graphs_z_value(char string[64]) { 
	for (byte i = 0; i < graph_count; i++) string[i] = graph[i] + '0';
	string[graph_count] = 0;
}



static void print_buffer(void) {
	printf("%llu / %llu :: {\n", buffer_count, max_buffer_count);
	for (nat i = 0; i < buffer_count; i++) {
		printf("\t%llu : \n", i);
		printf("\t");
		puts(buffer[i].z);
		printf("\t");
		puts(buffer[i].dt);
		printf("\t");
		puts("");
	}
	puts("}");
}





/*

	directory = "."
	filename = ""

append_to_file();


	directory = "."
	filename = "202309037.173812_3_3000_z.txt"


append_to_file();

	*/




static void append_to_file(nat zindex, nat zcount) {
	
	char newfilename[4096] = {0};

	strlcpy(newfilename, filename, sizeof newfilename);

	const int dir = open(directory, O_RDONLY | O_DIRECTORY, 0);
	if (dir < 0) { 
		perror("write open directory"); 
		printf("directory=%s ", directory); 
		return; 
	}
	int flags = O_WRONLY | O_APPEND;  mode_t m = 0;
try_open:;
	const int file = openat(dir, newfilename, flags, m);
	if (file < 0) {
		if (m) {
			perror("create openat file");
			printf("filename=%s ", newfilename);
			close(dir); return;
		}
		perror("write openat file");
		printf("filename=%s\n", newfilename);
		flags = O_CREAT | O_WRONLY | O_APPEND | O_EXCL;
		m     = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

		char dt[32] = {0};
		get_datetime(dt);
		snprintf(newfilename, sizeof newfilename, "%s_%llu_%llu_z.txt", dt, zindex, zcount);
		strlcpy(filename, newfilename, sizeof filename);

		goto try_open;
	}

	for (nat i = 0; i < buffer_count; i++) {
		write(file, buffer[i].z, strlen(buffer[i].z));
		write(file, " ", 1);
		write(file, buffer[i].dt, strlen(buffer[i].dt));
		write(file, "\n", 1);
	}

	close(file); 
	
	if (m) {
		printf("write: created %llu z values to ", buffer_count);
		printf("%s : %s\n", directory, newfilename);
		close(dir);  
		return;
	}

	char dt[32] = {0};
	get_datetime(dt);
	snprintf(newfilename, sizeof newfilename, "%s_%llu_%llu_z.txt", dt, zindex, zcount);

	if (renameat(dir, filename, dir, newfilename) < 0) {
		perror("rename");
		printf("filename=%s newfilename=%s", filename, newfilename);
		close(dir); return;
	}
	printf("[\"%s\" renamed to  -->  \"%s\"]\n", filename, newfilename);
	strlcpy(filename, newfilename, sizeof filename);

	close(dir);

	printf("\033[1mwrite: saved %llu z values to ", buffer_count);
	printf("%s : %s \033[0m\n", directory, newfilename);
}


static void write_graph(nat zindex, nat zcount) {

	get_datetime(buffer[buffer_count].dt);
	get_graphs_z_value(buffer[buffer_count].z);
	buffer_count++;

	print_buffer();

	if (buffer_count == max_buffer_count) {
		append_to_file(zindex, zcount);
		buffer_count = 0;
	}

	sleep(1);
	usleep(10000);
}


static const byte _ = 0;

static const byte _63R[20] = {
	0,  1, 4, _,      //        3
	1,  0, _, _,      //     6  7 
	2,  0, _, _,      //    10 11
	3,  _, _, _,      // 13 14 15
	4,  2, 0, _,      //       19
};


static const nat begin = 2;
static const nat end = 100;
static const nat range = end - begin;



int main(void) {

	graph = calloc(graph_count, 1);
	memcpy(graph, _63R, 20);
	nat counter = 0; 
	
	write_graph(++counter, range); 
	write_graph(++counter, range); 
	write_graph(++counter, range); 
	write_graph(++counter, range); 

	write_graph(++counter, range); 
	write_graph(++counter, range); 
	write_graph(++counter, range); 
	write_graph(++counter, range); 

	write_graph(++counter, range); 
	write_graph(++counter, range); 
	write_graph(++counter, range);

	write_graph(++counter, range); 
	write_graph(++counter, range); 
	write_graph(++counter, range);

	append_to_file(counter, range);
}






