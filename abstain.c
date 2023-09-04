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
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	// PATH_MAX
#include <unistd.h>

#define STR_MAX			1024
#define MAX_PROMISE_LENGTH	16	/* max string length of promises */
#define MAX_VICES		64

#define OK			0
#define FAIL			-1

extern char **environ;

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

char *promise_error = "error";
char *execpromises;
int nvices = 0;
char **vices;
char fullbin[PATH_MAX];

static const char optstr[]	= "+lV:v:";
static struct option longopts[]	= {
	{ "list",	no_argument,		NULL,	'l'	},
	{ "vice",	required_argument,	NULL,	'v'	},
	{ "vices",	required_argument,	NULL,	'V'	},
	{ NULL,		0,			NULL,	0	}
};

void list_promises() {
	size_t npromises = sizeof(promise_all) / sizeof(promise_all[0]);
	for (int i = 0; i < npromises; i++) {
		printf("%s\n", promise_all[i]);
	}
	exit(0);
}


void usage(char *self) {
	printf("%s [-v vice [-v vice ...] | -V vice,vice,...] [--] program [flags ...]\n", self);
	exit(0);
}

int is_string_in_array(char *str, size_t max_str, char **arr, size_t arr_len) {
	for (int i = 0; arr[i] != NULL && i < arr_len; i++) {
		if (strncmp(str, arr[i], max_str) == 0)
			return OK;
	}
	return FAIL;
}

char *path_lookup(char *bin, char *pathstring) {
	char *dir;

	while ((dir = strsep(&pathstring, ":")) != NULL) {
		if (*dir != '\0') {
			if (snprintf(fullbin, sizeof(fullbin), "%s/%s", dir, bin) < 0)
				errx(1, NULL);
			if (access(fullbin, X_OK) == 0)
				return fullbin;
		}
	}
	return NULL;
}

void run(int argc, char **argv) {
	char *promise;
	char *executable;
	char *path_executable;
	char *pathstring;

	for (int i = 0; i < nvices; i++) {
		if (is_string_in_array(vices[i], MAX_PROMISE_LENGTH, promise_all,
		                       sizeof(promise_all) / sizeof(promise_all[0])) == FAIL) {
			errx(1, "invalid vice: %s", vices[i]);
		}
	}

	if ((execpromises = malloc(STR_MAX)) == NULL)
		errx(1, NULL);
	if (strlcpy(execpromises, promise_error, sizeof(execpromises)) > sizeof(execpromises))
		errx(1, NULL);

	for (int i = 0; i < sizeof(promise_all) / sizeof(promise_all[0]); i++) {
		promise = promise_all[i];
		if (is_string_in_array(promise, sizeof(promise), vices, MAX_PROMISE_LENGTH) == FAIL) {
			if (snprintf(execpromises, STR_MAX, "%s %s", execpromises, promise) < 0)
				errx(1, NULL);
		}
	}
	if (execpromises[0] == ' ') {
		execpromises++;	/* remove initial whitespace */
	}

	executable = *argv;

	/* get absolute path of executable */
	if(access(executable, X_OK) == 0) {
		path_executable = executable;
	} else {
		if ((pathstring = getenv("PATH")) == NULL)
			errx(1, NULL);
		if ((path_executable = path_lookup(executable, pathstring)) == NULL)
		    errx(1, "no executable `%s' in PATH", executable);
	}

	printf("executing `%s' ", path_executable);
	if (argc > 1) {
		printf("with arguments: `");
		for (int i = 1; i < argc; i++)
			printf("%s ", argv[i]);
		printf("', ");
	}
	printf("with the following execpromises: \%s\n\n", execpromises);
	if (pledge(NULL, execpromises) == -1)
		errx(1, "unable to pledge: %s(%d)", strerror(errno), errno);
	if (execve(path_executable, argv, environ) == -1)
		errx(1, "unable to execute `%s': %s(%d)", path_executable, strerror(errno), errno);

	exit(0);
}

int main(int argc, char** argv) {
	int ch;
	char *self = argv[0];
	char *v;
	char *vices_string = "\0";

	if ((vices = calloc(MAX_VICES, MAX_PROMISE_LENGTH)) == NULL)
	    errx(1, NULL);

	while ((ch = getopt_long(argc, argv, optstr, longopts, NULL)) != -1) {
		switch (ch) {
		case 'l':
			list_promises();
			break;
		case 'V':
			vices_string = optarg;
			break;
		case 'v':
			vices[nvices++] = optarg;
			break;
		default:
			usage(self);
		}
	}
	argc -= optind;
	argv += optind;

	if (*vices != NULL && vices_string[0] != '\0')
		errx(1, "can't combine -v/--vice and -V/--vices flags");

	while ((v = strsep(&vices_string, ",")) != NULL) {
		if (*v != '\0')
			vices[nvices++] = v;
	}

	if (argc < 1)
		usage(self);

	run(argc, argv);
}
