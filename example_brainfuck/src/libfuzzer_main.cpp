/*
 * Copyright 2016 Fabian Mastenbroek
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <stdint.h>

#ifdef BRAINFUCK_EDITLINE_LIB
	#include </usr/include/readline/readline.h>
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
	#define isatty _isatty
#endif

#include <brainfuck.h>

/**
 * Print the usage message of this program.
 *
 * @param name The name of this program (given by argv[0]).
 */
void print_usage(char *name) {
	fprintf(stderr, "usage: %s [-evh] [file...]\n", name);
	fprintf(stderr, "\t-e --eval\t\trun code directly\n");
	fprintf(stderr, "\t-v --version\t\tshow version information\n");
	fprintf(stderr, "\t-h --help\t\tshow a help message\n");
}

/**
 * Print the version information.
 */
void print_version() {
	fprintf(stderr, "brainfuck %d.%d.%d (%s, %s)\n", BRAINFUCK_VERSION_MAJOR,
		BRAINFUCK_VERSION_MINOR, BRAINFUCK_VERSION_PATCH, __DATE__, __TIME__);
	fprintf(stderr, "Copyright (c) 2016 Fabian Mastenbroek.\n");
	fprintf(stderr, "Distributed under the Apache License Version 2.0.\n");
}

#ifdef BRAINFUCK_EDITLINE_LIB
/**
 * Readline initialization calls, in this case disabling autocompletion and
 * enabling user input history.
 */
void initialize_readline() {
	rl_bind_key ('\t', rl_insert); /* disable tab autocompletion */
	stifle_history(READLINE_HIST_SIZE); /* set history size */
}
#endif

/**
 * Run the given brainfuck file.
 *
 * @param file The brainfuck file to run.
 * @return EXIT_SUCCESS if no errors are encountered, otherwise EXIT_FAILURE.
 */
int run_file(FILE *file) {
	BrainfuckState *state = brainfuck_state();
	BrainfuckExecutionContext *context = brainfuck_context(BRAINFUCK_TAPE_SIZE);
	if (file == NULL) {
		brainfuck_destroy_context(context);
		brainfuck_destroy_state(state);
		return EXIT_FAILURE;
	}
	brainfuck_add(state, brainfuck_parse_stream(file));
	brainfuck_execute(state->root, context);
	brainfuck_destroy_context(context);
	brainfuck_destroy_state(state);
	fclose(file);
	return EXIT_SUCCESS;
}

/**
 * Run the given brainfuck string.
 *
 * @param code The brainfuck string to run.
 * @return EXIT_SUCCESS if no errors are encountered, otherwise EXIT_FAILURE.
 */
int run_string(char *code) {
	BrainfuckState *state = brainfuck_state();
	BrainfuckExecutionContext *context = brainfuck_context(BRAINFUCK_TAPE_SIZE);
	BrainfuckInstruction *instruction = brainfuck_parse_string(code);
	brainfuck_add(state, instruction);
	//brainfuck_execute(state->root, context);
	brainfuck_destroy_context(context);
	brainfuck_destroy_state(state);
	return EXIT_SUCCESS;
}

/**
 * Run the brainfuck interpreter in interactive mode.
 */
void run_interactive_console() {
	printf("brainfuck %d.%d.%d (%s, %s)\n", BRAINFUCK_VERSION_MAJOR,
		BRAINFUCK_VERSION_MINOR, BRAINFUCK_VERSION_PATCH, __DATE__, __TIME__);
#ifdef BRAINFUCK_EXTENSION_DEBUG
	printf("Use # to inspect tape\n");
#endif
	BrainfuckState *state = brainfuck_state();
	BrainfuckExecutionContext *context = brainfuck_context(BRAINFUCK_TAPE_SIZE);
	BrainfuckInstruction *instruction;
#ifdef BRAINFUCK_EDITLINE_LIB
	char *line;

	initialize_readline();
	while(1) {
		line = readline(">> ");
		if (line) {
			/* Empty string crashes on some OSs/versions of editline */
			if (line[0] == '\0') {
				free(line);
				continue;
			}

			char* expansion;
			int result;

			result = history_expand(line, &expansion);
			if (result >= 0 && result != 2 ) { add_history(expansion); }
			free(expansion);
		} else {
			/* EOF */
			break;
		}
		instruction = brainfuck_parse_string(line);
		free(line);
		brainfuck_add(state, instruction);
		brainfuck_execute(instruction, context);
	}
#else
	printf(">> ");
	while(1) {
		fflush(stdout);
		instruction = brainfuck_parse_stream_until(stdin, '\n');
		if (feof(stdin)) { break; }
		fflush(stdin);
		brainfuck_add(state, instruction);
		brainfuck_execute(instruction, context);
		printf(">> ");
	}
#endif
}

/* Command line options */
static struct option long_options[] = {
	{"help", no_argument, 0, 'h'},
	{"eval", required_argument, 0, 'e'},
	{"version", no_argument, 0, 'v'},
	{0, 0, 0, 0}
};

/**
 * Main entry point of the program.
 *
 * @param argc The amount of arguments given.
 * @param argv The array with arguments.
 */

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	char * code;
  	
        if (data[size-1] != '\x00'){
            code = (char*)alloca(size+1);
            memset(code, 0, size+1);
        } else {
            code = (char*)alloca(size);
            memset(code, 0, size);
        }
	memcpy(code, data, size);

	if(size > 1){
		run_string(code);
		return 1;
	}

	return 0;
}
