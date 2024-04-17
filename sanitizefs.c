#define _GNU_SOURCE
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <stdbool.h>
#include <getopt.h>

#ifndef DT_DIR
#define DT_DIR 4
#endif

#ifndef DT_REG
#define DT_REG 8
#endif

#define MAX_PATH 4096

bool dry_run = false;

typedef struct {
	char path[MAX_PATH];
	int depth;
} Entry;

Entry *entries = NULL;
int num_entries = 0;
int max_depth = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int processed_entries = 0;
int num_threads = 0;

static inline void replace_chars(char *str, int preserve_ext) {
	char *src = str;
	char *dst = str;
	char *ext = preserve_ext ? strrchr(str, '.') : NULL;

	while (*src && src != ext) {
		char c = *src;
		if (c >= 'A' && c <= 'Z') {
			*dst++ = c + ('a' - 'A');
		} else if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
			*dst++ = c;
		} else if (c == '_' || c == ' ' || c == '-' || c == '.') {
			if (dst == str || *(dst-1) != '_') {
				*dst++ = '_';
			}
		}
		src++;
	}

	if (dst > str && *(dst-1) == '_') {
		dst--;
	}

	while (ext && *ext) {
		*dst++ = *ext++;
	}

	*dst = '\0';

	if (str[0] == '_') {
		size_t len = dst - str;
		memmove(str, str + 1, len);
		str[len - 1] = '\0';
	}
}

static inline void process_entry(const char *fpath, int depth) {
	pthread_mutex_lock(&mutex);
	entries = realloc(entries, (num_entries + 1) * sizeof(Entry));
	strlcpy(entries[num_entries].path, fpath, MAX_PATH);
	entries[num_entries].depth = depth;
	num_entries++;
	if (depth > max_depth)
		max_depth = depth;
	pthread_mutex_unlock(&mutex);
}

static inline void dfs(const char *path, int depth) {
	DIR *dir = opendir(path);

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_name[0] == '.')
			continue;
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		char subpath[MAX_PATH];
		snprintf(subpath, sizeof(subpath), "%s/%s", path, entry->d_name);

		if (entry->d_type == DT_DIR) {
			process_entry(subpath, depth);
			dfs(subpath, depth + 1);
		} else if (entry->d_type == DT_REG) {
			process_entry(subpath, depth);
		}
	}

	closedir(dir);
}

typedef struct {
	int thread_id;
	pthread_t thread;
} ThreadInfo;

void *thread_func(void *arg) {
	ThreadInfo *thread_info = (ThreadInfo *)arg;
	int thread_id = thread_info->thread_id;
	int entries_per_thread = num_entries / num_threads;
	int start_index = thread_id * entries_per_thread;
	int end_index = (thread_id == num_threads - 1) ? num_entries : (thread_id + 1) * entries_per_thread;

	for (int depth = max_depth; depth >= 0; depth--) {
		for (int i = start_index; i < end_index; i++) {
			if (entries[i].depth == depth) {
				char path[MAX_PATH];
				strlcpy(path, entries[i].path, MAX_PATH);

				char dir[MAX_PATH];
				strlcpy(dir, path, MAX_PATH);
				dirname(dir);

				char *name = basename(path);

				struct stat path_stat;
				int dirfd = openat(AT_FDCWD, dir, O_PATH | O_CLOEXEC);
				if (dirfd == -1)
					continue;

				if (fstatat(dirfd, name, &path_stat, AT_SYMLINK_NOFOLLOW) != 0) {
					close(dirfd);
					continue;
				}

				char new_name[MAX_PATH];
				strlcpy(new_name, name, MAX_PATH);
				replace_chars(new_name, S_ISREG(path_stat.st_mode));

				if (!dry_run) {
					renameat2(dirfd, name, dirfd, new_name, RENAME_NOREPLACE);
				} else {
					if (strcmp(name, new_name) != 0) {
						printf("\"%s\" --> \"%s\"\n\n", path, new_name);
					}
				}

				close(dirfd);
			}
		}

		pthread_mutex_lock(&mutex);
		processed_entries++;

		if (processed_entries == num_threads) {
			processed_entries = 0;
			pthread_cond_broadcast(&cond);
		} else {
			pthread_cond_wait(&cond, &mutex);
		}
		pthread_mutex_unlock(&mutex);
	}

	return NULL;
}

int main(int argc, char *argv[]) {
	if (getuid() == 0) {
		fprintf(stderr, "No root usage.\n");
		return 1;
	}

	int opt;
	const char* short_opts = "dh";
	const struct option long_opts[] = {
		{"dry-run", no_argument, NULL, 'd'},
		{"help", no_argument, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};

	while (argc == 1 || (opt = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
		switch (opt) {
			case 'd':
		        	dry_run = true;
		        	break;
			case 'h':
				printf("USAGE:\n"
		               "        %s [-dh] [path1] [path2] ...\n\n"
		               "DESCRIPTION:\n"
		               "        Sanitize file and directory names recursively; according to UNIX and URL standards.\n"
		               "        Either give it a PATH, or a RELATIVE PATH or an INDIVIDUAL FILE.\n"
		               "        Use dry running feature to test to see how the names would change.\n"
		               "        SYSTEM files, and DOTFILES are protected. No worries.\n\n"
		               "EXAMPLES:\n"
		               "        %s EXAMPLE_DIR\n"
		               "        %s --dry-run \"/home/username/EXAMPLE DIR\"\n"
		               "        %s \"/home\"\n"
			       "	%s \"VIDEO FILE.mkv\" \"PICTURE.jpg\" \"DIRECTORY\" \n\n"
		               "OPTIONS:\n"
		               "        -d, --dry-run	Perform a 'dry run' and exit. Do not rename anything.\n"
		               "        -h, --help	Show this message and exit.\n",
		               argv[0], argv[0], argv[0], argv[0], argv[0]);
		        return 0;
		}
	}

	num_threads = sysconf(_SC_NPROCESSORS_ONLN);
	ThreadInfo thread_pool[num_threads];

	for (int i = 1; i < argc; i++) {
		struct stat sb;
		if (lstat(argv[i], &sb) == 0) {
			if (S_ISDIR(sb.st_mode)) {
				dfs(argv[i], 0);
			}
		}
	}

	for (int i = 0; i < num_threads; i++) {
		thread_pool[i].thread_id = i;
		pthread_create(&thread_pool[i].thread, NULL, thread_func, &thread_pool[i]);
	}

	for (int i = 0; i < num_threads; i++) {
		pthread_join(thread_pool[i].thread, NULL);
	}

	free(entries);

	for (int i = 1; i < argc; i++) {
		char absolute_path[PATH_MAX];
		if (realpath(argv[i], absolute_path) != NULL) {
			struct stat sb;
			if (lstat(absolute_path, &sb) == 0) {
				char *dir = dirname(strdup(absolute_path));
				char *name = basename(absolute_path);

				char new_name[MAX_PATH];
				strcpy(new_name, name);
				replace_chars(new_name, S_ISREG(sb.st_mode));

				if (strcmp(name, new_name) != 0) {
					if (!dry_run)
						renameat2(AT_FDCWD, absolute_path, AT_FDCWD, new_name, RENAME_NOREPLACE);
					else
						printf("\"%s\" --> \"%s\"\n\n", absolute_path, new_name);
				}

				free(dir);
			}
		}
	}

	return 0;
}

