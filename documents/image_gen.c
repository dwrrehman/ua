

static void generate_lifetime_images(
	const char* z_list_filename, 
	const char* destination_dir, 
	byte* graph, byte start, 
	nat begin_timestep, nat end_timestep,
	nat begin_cell, nat end_cell,
	nat maximum
) {
	nat list_count = 0;
	byte** list = read_z_list_from_file(z_list_filename, &list_count);
	if (not list) return;

	printf("generate: generating %llu lifetimes .ppm's...\n", list_count);

	const nat n = array_size - 1;
	nat* array = calloc(array_size, sizeof(nat));
	nat pointer = 0;

	for (nat z = 0; z < list_count; z++) {

		memcpy(graph, list[z], 32);
		memset(array, 0, array_size * sizeof(nat));
		pointer = 0;

		byte i = start;

		nat timestep = 0;
		
		printf("\r [ ( %llu / %llu ) ] generating lifetime:  %s            ", 
				z, list_count, hex_string(graph));

		fflush(stdout);

		
		char path[4096] = {0}, filename[4096] = {0};
		sprintf(filename, "z_%s.ppm", hex_string(graph));
		strcpy(path, destination_dir);
		strcat(path, "/");
		strcat(path, filename);

		FILE* file = fopen(path, "wb");

		if (not file) {
			fprintf(stderr, "error: %s: could not open file for writing: %s\n", 
				path, strerror(errno));
			return;
		}

		fprintf(file, "P6\n%llu %llu\n255\n", end_cell - begin_cell, end_timestep - begin_timestep);

		do {
			
			if (i == 5) {
				if (timestep >= begin_timestep and 
				    timestep < end_timestep) {
					for (nat j = begin_cell; j < end_cell; j++) {
						double x = (double) array[j] / (double) maximum;  

						// nat x = array[j];
				   //              unsigned char 
							// r = x ? 255 : 0, 
							// g = x ? 255 : 0, 
							// b = x ? 255 : 0;

						unsigned char 
							r = (unsigned char)(x * 255.0), 
							g = (unsigned char)(x * 255.0), 
							b = (unsigned char)(x * 255.0);
				                fwrite(&r, 1, 1, file);
				                fwrite(&g, 1, 1, file);
				                fwrite(&b, 1, 1, file);
					}
				}
				timestep++;
			}


			if (i == 0xE) {
				i = graph[i * 2 + (array[n] < array[pointer])];
			} else if (i == 0xC) {
				i = graph[i * 2 + (array[n] != array[pointer])];
			} else if (i == 0xF) {
				i = graph[i * 2 + (array[n] > array[pointer])];
			}

			else if (i == 1) { pointer++; 			i = graph[i * 2];  }
			else if (i == 2) { array[n]++; 			i = graph[i * 2];  }
			else if (i == 3) { array[pointer]++; 		i = graph[i * 2];  }
			else if (i == 5) { pointer = 0; 		i = graph[i * 2];  }
			else if (i == 6) { array[n] = 0; 		i = graph[i * 2];  }
			
		} while (timestep < end_timestep);

		fclose(file);


	}
	printf("generate: generated all images.\n");
	free(array);
}


