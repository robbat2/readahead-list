#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sched.h>
#include <string.h>

static char* program_name = "readhead-list";
static char* program_version = "$Header: /code/convert/cvsroot/infrastructure/readahead-list/Attic/readahead-list.c,v 1.1 2005/03/20 03:32:10 robbat2 Exp $";

static int flag_debug = 0;
static int flag_verbose = 0;
static int flag_version = 0;
static int flag_help = 0;

static struct option long_options[] = {
	{"verbose", 0, &flag_verbose, 1},
	{"debug", 0, &flag_debug, 1},
	{"version", 0, &flag_version, 1},
	{"help", 0, &flag_help, 1},
	{0, 0, 0, 0}
};
static char* short_options = "vd"; // no short opt for version

void process_file(char *filename) {
	int fd;
	struct stat buf;
	
	if (!filename)
		return;
		
	fd = open(filename,O_RDONLY);
	if (fd<0)
		return;
	
	if (fstat(fd, &buf)<0)	 
		return;
	
	readahead(fd, (loff_t)0, (size_t)buf.st_size);
	close(fd);
	/* be nice to other processes now */
	sched_yield();
}

#define MAXPATH 1024
void process_files(char* filename) {
	int fd;
	char* file = NULL;
	struct stat statbuf;
	char buffer[MAXPATH+1];
	char* iter = NULL;

	if (!filename)
		return;

	fd = open(filename,O_RDONLY);
	if (fd<0)
		return;
	
	if (fstat(fd, &statbuf)<0)	 
		return;

	/* map the whole file */
	file = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (!file || file == MAP_FAILED)
		return;

	iter = file;
	while (iter) {
		/* find next newline */
		char* next = memchr(iter,'\n',file + statbuf.st_size - iter);
		if (next) {
			if (next - iter < MAXPATH && (next-iter > 1)) {
				memcpy(buffer, iter, next-iter);
				buffer[next-iter]='\0';
				process_file(buffer);
			}
			iter = next + 1;
		} else {
			iter = NULL;
		}
	}
}

void skel_command_msg_exit(FILE * f, char* msg, unsigned char retval) {
	fprintf(f,msg);
	exit(retval);
}

void command_version() {
#define LEN 1024
	char s[LEN];
	snprintf(s,LEN,"%s: %s\n",program_name,program_version);
#undef LEN
	skel_command_msg_exit(stdout,s,0);
}
void command_error() {
#define LEN 1024
	char s[LEN];
	snprintf(s,LEN,"Try `%s --help' for more information.\n",argv[0]);
#undef LEN
	skel_command_msg_exit(stderr,s,1);
}

int main(int argc, char **argv) {
	int i;
	printf("---BEFORE\n");
	for (i=1; i<argc; i++) {
		//process_files(argv[i]);
		printf("argv[%d]: '%s'\n",i,argv[i]);
	}
	printf("---DURING\n");
	while(1) {
		int long_index,c;
		long_index = -1;
		c = getopt_long(argc,argv,short_options,long_options,&long_index);
		//printf("optind: %d opterr: %d optopt: %d long_index: %d optarg: %s\n",optind,opterr,optopt,long_index,optarg);
		// done
		if (c == -1)
			break;
		switch(long_index) {
			// handled by getopt directly
			case 0: // verbose
			case 1: // debug
			case 2: // version
			case 3: // version
				break;
			// something else
			default:
				command_error();
		}

	}
	printf("---AFTER-processed\n");
	//if(flag_version) {
	//	command_version();
	//}
	printf("debug: %d\n",flag_debug);
	printf("verbose: %d\n",flag_verbose);
	printf("version: %d\n",flag_version);
	printf("help: %d\n",flag_help);
	printf("optind: %d opterr: %d optopt: %d optarg: %s\n",optind,opterr,optopt,optarg);
	printf("---AFTER-raw\n");
	for (i=optind; i<argc; i++) {
		//process_files(argv[i]);
		printf("argv[%d]: '%s'\n",i,argv[i]);
	}
	return 0;
}

// vim: ts=4 sw=4:
