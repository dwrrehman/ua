// some common functions which are used in both server and client code.
// written on 1202604035.210728 by dwrr

static void print_binary(nat x) {
	for (nat i = 0; i < 64; i++) {
		if (not (i & 3)) putchar('_');
		printf("%llu", (x >> i) & 1);
	}
}

static void send_string(int con, char* string, nat length) {
	send(con, &length, sizeof(nat), 0);
	printf("SEND_STRING: info: send(8 bytes) call sent a string length of %llu\n", length);
	nat packet_count = (length / 512) + 1;	
	printf("SEND_STRING: info: sending %llu packets...\n", packet_count);
	nat pointer = 0;
	for (nat p = 0; p < packet_count; p++) {
		send(con, string + pointer, 512, 0);
		pointer += 512;
	}
	
}

static char* get_string(int con, nat* output_length) {
	recv(con, output_length, sizeof(nat), 0);
	printf("GET_STRING: info: recv(8 bytes) call received a string length of %llu\n", *output_length);
	nat packet_count = (*output_length / 512) + 1;
	printf("GET_STRING: info: receiving %llu packets...\n", packet_count);
	char* output = calloc(packet_count, 512);
	nat pointer = 0;
	for (nat p = 0; p < packet_count; p++) {
		recv(con, output + pointer, 512, 0);
		pointer += 512;
	}
	return output;
}

static void send_command(int this, char c) {
	char operation[16] = {0};
	operation[0] = c;
	send(this, operation, 16, 0);
}

static void print_string(const char* message, char* string, nat length) {
	printf("%s: (%llu bytes) <<<%.*s>>>\n", message, length, (int) length, string);
}

static void print_array(char* array, nat count) {
	printf("{\n\t");
	for (nat i = 0; i < count; i++) {
		if (not (i % 4)) puts("");
		printf("%02x(%c) ", array[i], array[i]); 
	}
	puts("\n}");
}

static char* load_file(const char* filename, nat* text_length) {
	int file = open(filename, O_RDONLY);
	if (file < 0) { 
		printf("utility: \033[31;1merror:\033[0m could not open '%s': %s\n", 
			filename, strerror(errno)
		); 
		exit(1);
	}
	*text_length = (nat) lseek(file, 0, SEEK_END);
	lseek(file, 0, SEEK_SET);
	char* text = calloc(*text_length + 1, 1);
	read(file, text, *text_length);
	close(file);
	return text;
}

static void get_datetime(char datetime[32]) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* tm_info = localtime(&tv.tv_sec);
	strftime(datetime, 32, "1%Y%m%d%u.%H%M%S", tm_info);
}

static void write_to_file(char* string, nat string_length) {
	char dt[32] = {0};
	get_datetime(dt);
	char filename[4096] = {0};
	snprintf(filename, sizeof filename, "%s_%08x%08x%08x%08x.txt", dt, rand(), rand(), rand(), rand());

	int flags = O_WRONLY | O_APPEND;
	mode_t permissions = 0;
	flags = O_CREAT | O_WRONLY | O_APPEND | O_EXCL;
	permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

	const int file = open(filename, flags, permissions);

	if (file < 0) {	
		perror("create open file");
		printf("print: [%s]: failed to create filename = \"%s\"\n", dt, filename);
		fflush(stdout);
		abort();
	}
	write(file, string, string_length);
	close(file);

	printf("wrote to file (%llu bytes): <<<%.*s>>>\n", string_length, (int) string_length, string);
	fflush(stdout);
}



























