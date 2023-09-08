/*
 * Copyright (c) 2023 Thomas Frohwein
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define STR_MAX			1024
#define MAX_PROMISE_LENGTH	16	/* max string length of promises */
#define MAX_PROMISES		64

#define OK			0
#define FAIL			-1

/* don't include error promise here because it is pledged by default */
char *promise_all[] = {
	"stdio", "rpath", "wpath", "cpath",
	"dpath", "tmppath", "inet", "mcast",
	"fattr", "chown", "flock", "unix",
	"dns", "getpw", "sendfd", "recvfd",
	"tape", "tty", "proc", "exec",
	"prot_exec", "settime", "ps", "vminfo",
	"id", "pf", "route", "wroute",
	"audio", "video", "bpf", "unveil"
};

const char *promise_error = "error";
char execpromises[STR_MAX] = "\0";
int npromises = 0;
char **promises;
int use_error = 0;
int verbosity = 0;

static const char optstr[]	= "+elp:v";
static struct option longopts[]	= {
	{ "error",	no_argument,		NULL,	'e'	},
	{ "list",	no_argument,		NULL,	'l'	},
	{ "promise",	required_argument,	NULL,	'p'	},
	{ "verbose",	no_argument,		NULL,	'v'	},
	{ NULL,		0,			NULL,	0	}
};

void usage(char *self) {
	printf("%s [-lev] [-p promise[,promise,...]] binary [flags ...]\n", self);
	exit(0);
}

void list_promises() {
	size_t npromise_all = sizeof(promise_all) / sizeof(promise_all[0]);
	for (int i = 0; i < npromise_all; i++) {
		printf("%s\n", promise_all[i]);
	}
	exit(0);
}

int is_string_in_array(char *str, size_t max_str, char **arr, size_t arr_len) {
	for (int i = 0; arr[i] != NULL && i < arr_len; i++) {
		if (strncmp(str, arr[i], max_str) == 0)
			return OK;
	}
	return FAIL;
}

void run(int argc, char **argv) {
	const char *executable = *argv;
	char *promise;
	size_t npromise_all = sizeof(promise_all) / sizeof(promise_all[0]);

	for (int i = 0; i < npromises; i++) {
		if (is_string_in_array(promises[i], MAX_PROMISE_LENGTH, promise_all,
		                       npromise_all) == FAIL) {
			errx(-1, "invalid vice: %s", promises[i]);
		}
	}

	for (int i = 0; i < npromise_all; i++) {
		promise = promise_all[i];
		if (is_string_in_array(promise, sizeof(promise), promises, MAX_PROMISES) == FAIL) {
			if (*execpromises == '\0') {
				if (strlcpy(execpromises, promise, sizeof(execpromises)) > sizeof(execpromises))
					errx(-1, NULL);
			}
			else if ((strnlen(execpromises, sizeof(execpromises)) >=
			          strnlen(promise, sizeof(promise))) &&
			         (strstr(execpromises, promise) == NULL)) {
				if (snprintf(execpromises, sizeof(execpromises), "%s %s", execpromises, promise) < 0)
					errx(-1, NULL);
			}
		}
	}
	if (use_error) {
		if (snprintf(execpromises, sizeof(execpromises), "%s %s", execpromises, promise_error) < 0)
			errx(-1, NULL);
	}

	if (verbosity) {
		printf("executing:\t`%s", executable);
		if (argc > 1) {
			for (int i = 1; i < argc; i++)
				printf(" %s", argv[i]);
		}
		printf("'\n");
		printf("execpromises:\t%s\n\n", execpromises);
	}

	if (pledge(NULL, execpromises) == -1)
		errx(-1, "unable to pledge: %s(%d)", strerror(errno), errno);
	if (execvp(executable, argv) == -1)
		errx(-1, "unable to execute `%s': %s(%d)", executable, strerror(errno), errno);
}

int main(int argc, char** argv) {
	int ch;
	char *self = argv[0];
	char *p;

	if ((promises = calloc(MAX_PROMISES, MAX_PROMISE_LENGTH)) == NULL)
	    errx(-1, NULL);

	while ((ch = getopt_long(argc, argv, optstr, longopts, NULL)) != -1) {
		switch (ch) {
		case 'e':
			use_error = 1;
			break;
		case 'l':
			list_promises();
			break;
		case 'p':
			while ((p = strsep(&optarg, ",")) != NULL) {
				if (*p != '\0')
					promises[npromises++] = p;
			}
			break;
		case 'v':
			verbosity = 1;
			break;
		default:
			usage(self);
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 1)
		usage(self);

	run(argc, argv);
}
