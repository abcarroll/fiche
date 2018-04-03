#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

#define USAGE do{fprintf(stderr, "usage: cleaner	[-hvD] [-s sleep] [-t time] [-d directory]\n");\
	return (1); }while(0);
#define VERBOSE(...) printf("info: "__VA_ARGS__ ); printf("\n");

#define DEFAULT_SLEEP 10 //seconds

// bools
static uint8_t verbose = 0;
static uint8_t dry_run = 0;

/**
 * @brief Cleaning function
 * @args path to dir and file age to check
 */
static void clean(const char *path, const uint16_t age) {
	DIR *dfd;
	struct dirent *dent;

	{
		if ((dfd = opendir(path)) == NULL) {
			perror("error");
			exit (1);
		}
	};

	
	if (verbose) {
		VERBOSE("reading directory content.")
	}
	while ((dent = readdir(dfd)) != NULL) {
		// avoid actual and previous dir
		if ((strcmp(".", dent->d_name) == 0) ||
			(strcmp("..", dent->d_name) == 0))
			continue;
		// avoid other than dirs
		if (dent->d_type != DT_DIR)
			continue;

		char *fname = dent->d_name;
		time_t c_age;
		{
			struct stat info;
			if (stat(fname, &info) == -1) {
				perror("error");
				continue;
			}
			c_age = info.st_mtime;
		};

		// get the minutes since last mod
		c_age = (time(NULL) - c_age)/60;

		// found one?
		if (c_age >= age) {
			if (verbose) {
				VERBOSE("deleting file %s.", fname)
			}
			if (!dry_run) {
				{
					// clean index.txt
					char index_path[NAME_MAX];
					strncpy(index_path, fname, (NAME_MAX-11));
					strcat(index_path, "/index.txt");
					if (unlink(index_path) == -1) {
						perror("error");
						continue;
					} else {
						if (rmdir(fname) == -1) {
							perror("error");
							continue;
						}
					}
				};
			}	
		}
	}

	// there was an error?
	if (errno != 0)
		perror("error");

	(void)closedir(dfd);
}

/**
 * @brief main function
 */
int main(int argc, char *argv[])
{
	char *dir = NULL;
	uint16_t time = 0, sleep_time = DEFAULT_SLEEP;

	if ((argc-1) == 0)
		USAGE
	int c;
	while ((c = getopt(argc, argv, "hDvt:d:s:")) != -1) {
		switch (c) {
			case 'D':
				dry_run++;
				break;
			case 'v':
				verbose++;
				break;
			case 't':
				time = atoi(optarg);
				break;
			case 's':
				sleep_time = atoi(optarg);
				if (sleep_time == 0)
					sleep_time = DEFAULT_SLEEP;
				break;
			case 'd':
				dir = optarg;
				break;
			case 'h':
			default:
				USAGE
				break;
		}
	}

	if (dir == NULL) {
		fprintf(stderr, "error: directory not defined.\n");
		return (1);
	}
	if (time == 0) {
		fprintf(stderr, "error: age must be older than 0 minutes.\n");
		return (1);
	}
	if (dry_run) {
		VERBOSE("dry running.")
	}

	// change to fiche output dir
	{
		if (verbose) {
			VERBOSE("changing dir.")
		}
		int o = chdir (dir);
		if (o != 0) {
			perror("error");
			return (1);
		}
	};

	// main loop
	while (1)
	{
		if (verbose) {
			VERBOSE("entering main loop")
		}
		clean(dir, time);
		sleep(sleep_time);
	}

	return (0);
}

