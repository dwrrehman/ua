#include <stdio.h>      //  this utility takes a file listing the timesteps (ins_count) 
#include <stdlib.h>     //  and reads them into a data structure, and then writes a select amount of that data to 
#include <string.h>     //  a csv file. 
#include <iso646.h>
#include <unistd.h>


typedef unsigned long long nat;

struct lifetime {
	nat count;
	nat* xws;
	nat* lms;
};

static void print_data(struct lifetime* data, nat data_count) {
	puts("printing all xw data for all z values...\n");
	for (nat i = 0; i < data_count; i++) {
		printf("z index: [%llu] { \n", i);
		for (nat j = data[i].count - 10; j < data[i].count; j++) {
			printf("\t (%llu, %llu, %llu)\n", j, data[i].xws[j], data[i].lms[j]);
		}
		puts("}");
		getchar();
	}
	puts("[done]");
}




// [ 200 123 56 45 34 54 12 67 23 12 67 23 1 34 23 5 4 2 1 0 0 0 0 0 0 ]








int main(int argc, const char** argv) {

	if (argc == 1) return puts("usage: ./run xp_data_file.txt");
	
	FILE* f = fopen(argv[1], "r");
	if (not f) { perror("open"); exit(1); }

	nat data_count = 0;
	struct lifetime* data = NULL;

	char buffer[4096] = {0};

	nat previous_e = (nat) -1;
	nat previous_xw = (nat) -1;
	nat previous_lm = (nat) -1;

	puts("reading in xp data...");
	while (fgets(buffer, sizeof buffer, f)) {

		nat e = 0;
		nat xw = 0;
		nat lm = 0;
		sscanf(buffer, "%llu %llu %llu\n", &e, &xw, &lm);

		// printf("--> read line: (%llu, %llu, %llu)\n", e, xw, lm);

		if (not e) {
			
			struct lifetime new = {.xws = malloc(sizeof(nat)), .lms = malloc(sizeof(nat)), .count = 1};
			new.xws[0] = xw;
			new.lms[0] = lm;

			data = realloc(data, sizeof(struct lifetime) * (data_count + 1));
			data[data_count++] = new;

		} else {
			for (nat i = previous_e; i < e - 1; i++) {
				data[data_count - 1].xws = realloc(data[data_count - 1].xws, sizeof(nat) * (data[data_count - 1].count + 1));
				data[data_count - 1].xws[data[data_count - 1].count] = previous_xw;

				data[data_count - 1].lms = realloc(data[data_count - 1].lms, sizeof(nat) * (data[data_count - 1].count + 1));
				data[data_count - 1].lms[data[data_count - 1].count++] = previous_lm;
			}

			data[data_count - 1].xws = realloc(data[data_count - 1].xws, sizeof(nat) * (data[data_count - 1].count + 1));
			data[data_count - 1].xws[data[data_count - 1].count] = xw;

			data[data_count - 1].lms = realloc(data[data_count - 1].lms, sizeof(nat) * (data[data_count - 1].count + 1));
			data[data_count - 1].lms[data[data_count - 1].count++] = lm;
		}

		previous_e = e;
		previous_xw = xw;
		previous_lm = lm;
	}
	fclose(f);

	// print_data(data, data_count);



	puts("writing xws.csv...");
	FILE* xws = fopen("xws.csv", "w");
	if (not xws) abort();
	fprintf(xws, "ts, ");
	for (nat i = 0 ; i < data_count; i++) {
		fprintf(xws, "z%llu", i);
		if (i < data_count - 1) fprintf(xws, ", "); else fprintf(xws, "\n"); 
	}
	for (nat j = 0; j < data->count; j++) {
		fprintf(xws,    "%llu, ", j);
		for (nat i = 0; i < data_count; i++) {
			fprintf(xws, "%llu", data[i].xws[j]);
			if (i < data_count - 1) fprintf(xws, ", "); else fprintf(xws, "\n"); 
		}
	}
	fclose(xws);


	puts("writing lms.csv...");
	FILE* lms = fopen("lms.csv", "w");
	if (not lms) abort();
	fprintf(lms, "ts, ");
	for (nat i = 0 ; i < data_count; i++) {
		fprintf(lms, "z%llu", i);
		if (i < data_count - 1) fprintf(lms, ", "); else fprintf(lms, "\n"); 
	}
	for (nat j = 0; j < data->count; j++) {
		fprintf(lms,    "%llu, ", j);
		for (nat i = 0; i < data_count; i++) {
			fprintf(lms, "%llu", data[i].lms[j]);
			if (i < data_count - 1) fprintf(lms, ", "); else fprintf(lms, "\n"); 
		}
	}
	fclose(lms);


	const nat bucket_width = 3;
	const nat bucket_count = 100;




	exit(1);





	for (nat at = 0; at < data->count; at++) {

		nat* buckets = calloc(bucket_count, sizeof(nat));

		for (nat j = 0; j < data_count; j++) {
			for (nat i = 0; i < bucket_count; i++) {
				if (data[j].lms[at] < (i + 1) * bucket_width) {
					buckets[i]++;
					break;
				}
			}
		}

		printf("\033[H\033[2J");
		for (nat i = 0; i < bucket_count; i++) {
			printf(". %5llu: ", i * bucket_width);
			for (nat j = 0; j < buckets[i]; j++) printf("#");
			puts("");
		}

		puts("");
		free(buckets);
		usleep(100);
	}

}










/*

if (j) fprintf(speeds, "%llu, %lf, %lf, %lf\n", j, 
			(double) average_data[j] / (double) j,
			(double) data[fastest_z_index].xws[j] / (double) j,
			(double) data[slowest_z_index].xws[j] / (double) j
		); else fprintf(speeds, "0, 0, 0, 0\n");


		// if (j) fprintf(speeds, "%llu, ", (double) data[i].xws[j] / (double) j);

*/




/*













	nat shortest_lifetime = (nat) ~0;

	for (nat i = 0; i < data_count; i++) {
		if (data[i].count < shortest_lifetime) shortest_lifetime = data[i].count;
	}


	printf("shortest_lifetime = %llu\n", shortest_lifetime);

	const nat at_ts = shortest_lifetime - 1;

	nat slowest_xw = (nat) ~0;
	nat slowest_z_index = 0;
	for (nat i = 0; i < data_count; i++) {
		if (data[i].xws[at_ts] < slowest_xw) {
			slowest_xw = data[i].xws[at_ts];
			slowest_z_index = i;
		}
	}


	nat fastest_xw = 0;
	nat fastest_z_index = 0;
	for (nat i = 0; i < data_count; i++) {
		if (data[i].xws[at_ts] > fastest_xw) {
			fastest_xw = data[i].xws[at_ts];
			fastest_z_index = i;
		}
	}
	
	printf("slowest z value = #%llu  -- had a speed of ts=%llu,xw=%llu\n", slowest_z_index, at_ts, slowest_xw);
	printf("fastest z value = #%llu  -- had a speed of ts=%llu,xw=%llu\n", fastest_z_index, at_ts, fastest_xw);

	nat* average_data = calloc(shortest_lifetime, sizeof(nat));
	for (nat j = 0; j < shortest_lifetime; j++) {
		nat sum = 0;
		for (nat i = 0; i < data_count; i++) sum += data[i].xws[j];
		average_data[j] = sum / data_count;
	}







*/







