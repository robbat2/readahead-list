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
#include <errno.h>

static char* program_name = "readhead-list";
static char* program_header = "$Header: /code/convert/cvsroot/infrastructure/readahead-list/Attic/readahead-list.c,v 1.3 2005/03/21 02:32:43 robbat2 Exp $";
static char* program_id = "$Id: readahead-list.c,v 1.3 2005/03/21 02:32:43 robbat2 Exp $";

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
static char* short_options = "vdhV"; // no short opt for version

void process_file(char *filename) {
#define __FUNCTION__ "process_file"
	int fd;
	struct stat buf;
	
	if (!filename)
		return;
	
	if(flag_debug) {
		fprintf(stderr,"%s:%s:%d:Attempting to readhead file: %s\n",__FILE__,__FUNCTION__,__LINE__,filename);
	}
		
	fd = open(filename,O_RDONLY);
	if (fd<0) {
		if(flag_debug) {
			fprintf(stderr,"%s:%s:%d:failed to open file: %s\n",__FILE__,__FUNCTION__,__LINE__,filename);
		}
		return;
	}
	
	if (fstat(fd, &buf)<0) {
		if(flag_debug) {
			fprintf(stderr,"%s:%s:%d:failed to fstat file: %s\n",__FILE__,__FUNCTION__,__LINE__,filename);
		}
		return;
	}
	
	readahead(fd, (loff_t)0, (size_t)buf.st_size);
	int readahead_errno = errno;
	switch(readahead_errno) {
		case 0: 
			if(flag_debug) 
				fprintf(stderr,"%s:%s:%d:Loaded %s\n",__FILE__,__FUNCTION__,__LINE__,filename);
			if(flag_verbose) 
				fprintf(stdout,"Loaded file:%s\n",filename);
			break;
		case EBADF:
			if(flag_debug)
				fprintf(stderr,"%s:%s:%d:Bad file: %s\n",__FILE__,__FUNCTION__,__LINE__,filename);
		case EINVAL:
			if(flag_debug)
				fprintf(stderr,"%s:%s:%d:Invalid filetype for readhead: %s\n"__FILE__,__FUNCTION__,__LINE__,filename);
			break;
	}

	close(fd);
	/* be nice to other processes now */
	sched_yield();
#undef __FUNCTION__
}

#define MAXPATH 2048
void process_files(char* filename) {
#define __FUNCTION__ "process_files"
	int fd;
	char* file = NULL;
	struct stat statbuf;
	char buffer[MAXPATH+1];
	char* iter = NULL;

	if (!filename)
		return;
	
	if(flag_debug) {
		fprintf(stderr,"%s:%s:%d:Attempting to load list: %s\n",__FILE__,__FUNCTION__,__LINE__,filename);
	}

	fd = open(filename,O_RDONLY);
	if (fd<0) {
		if(flag_debug) {
			fprintf(stderr,"%s:%s:%d:failed to open list: %s\n",__FILE__,__FUNCTION__,__LINE__,filename);
		}
		return;
	}
	
	if (fstat(fd, &statbuf)<0) {
		if(flag_debug) {
			fprintf(stderr,"%s:%s:%d:failed to fstat list: %s\n",__FILE__,__FUNCTION__,__LINE__,filename);
		}
		return;
	}

	/* map the whole file */
	file = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (!file || file == MAP_FAILED) {
		if(flag_debug) {
			fprintf(stderr,"%s:%s:%d:failed to mmap list: %s\n",__FILE__,__FUNCTION__,__LINE__,filename);
		}
		return;
	}
	if(flag_verbose) 
		fprintf(stdout,"Loaded list:%s\n",filename);

	iter = file;
	while (iter) {
		/* find next newline */
		char* next = memchr(iter,'\n',file + statbuf.st_size - iter);
		if (next) {
			// if the length is positive, and shorter than MAXPATH
			// then we process it
			if((next - iter) >= MAXPATH) {
				fprintf(stderr,"%s:%s:%d:item in list too long!\n",__FILE__,__FUNCTION__,__LINE__);
			} else if (next-iter > 1) {
				memcpy(buffer, iter, next-iter);
				// replace newline with string terminator
				buffer[next-iter]='\0';
				process_file(buffer);
			}
			iter = next + 1;
		} else {
			iter = NULL;
		}
	}
#undef __FUNCTION__
}

void skel_command_msg_exit(FILE * f, char* msg, unsigned char retval) {
	fprintf(f,msg);
	exit(retval);
}

void command_error() {
#define LEN 1024
	char s[LEN];
	snprintf(s,LEN,"Try `%s --help' for more information.\n",program_name);
#undef LEN
	skel_command_msg_exit(stderr,s,1);
}

void command_version() {
#define LEN 1024
	char s[LEN];
	snprintf(s,LEN,"%s: %s\n",program_name,program_id);
#undef LEN
	fprintf(stdout,s);
}

void command_help() {
#define LEN 8192
	char s[LEN];
	snprintf(s,LEN,
			"Usage: %s [OPTION]... [FILE]...\n"\
			"Loads lists from FILE and performs readahead(2) on each entry.\n"\
			"\n"\
			"Options:\n"\
			"  -v --verbose   Print the name of each file that is successfully loaded.\n"\
			"  -d --debug     Print out status messages while processing.\n"\
			"  -h --help      Stop looking at me!\n"\
			"  -V --version   As the name says.\n"\
			,program_name);
#undef LEN
	fprintf(stdout,s);
}

int main(int argc, char **argv) {
#define __FUNCTION__ "main"
	int i;
	program_name = argv[0];
	while(1) {
		int long_index,c;
		long_index = -1;
		c = getopt_long(argc,argv,short_options,long_options,&long_index);
		if (c == -1)
			break;
		switch(c) {
			// is this a long option?
			case 0:
			case '-': 
				switch(long_index) {
					// handled by getopt directly
					case 0: // verbose
					case 1: // debug
					case 2: // version
					case 3: // help
						break;
					default:
						command_error();
				}
				break;
			// nope, short option
			case 'v':
				flag_verbose = 1;
				break;
			case 'd':
				flag_version = 1;
				break;
			case 'h':
				flag_help = 1;
				break;
			case 'V':
				flag_version = 1;
				break;
			default:
				command_error();
		}
	}
	if(flag_help) {
		command_help();
	}
	if(flag_version) {
		command_version();
	}
	if(flag_version || flag_help) {
		exit(0);
	}
	// now do the work
	for (i=optind; i<argc; i++) {
		process_files(argv[i]);
	}
	return 0;
}

// vim: ts=4 sw=4:
