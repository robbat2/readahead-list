#include <map>
#include <iostream>
#include <vector>

#include <string.h>
#include <getopt.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <fcntl.h>
#include <linux/fs.h>

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 2048
#endif

#define _DEBUG_MSG(fmt,msg...) { fprintf(stderr,fmt, ##msg); fflush(stderr); };
#define WARNING(fmt,msg...) _DEBUG_MSG(fmt,##msg);
int __STACK_DEBUG__ = 0;
bool __DEBUG__ = false;
#define DEBUG(fmt,msg...) { if(__DEBUG__ && flag_debug) { _DEBUG_MSG("%d:%s:%d:",__STACK_DEBUG__,__FILE__,__LINE__); _DEBUG_MSG(fmt, ##msg ); } }
#define DEBUG_STACK_UP { if(__DEBUG__ && flag_debug) { __STACK_DEBUG__++; } }
#define DEBUG_STACK_DOWN { if(__DEBUG__ && flag_debug) { __STACK_DEBUG__--; } }
#define DEBUG_ON { __DEBUG__ = true; }
#define DEBUG_OFF { __DEBUG__ = false; }

using namespace std;

static char* program_name = "readhead-list";
static char* program_header = "$Header: /code/convert/cvsroot/infrastructure/readahead-list/Attic/filelist-order.cxx,v 1.1 2005/03/22 08:31:08 robbat2 Exp $";
static char* program_id = "$Id: filelist-order.cxx,v 1.1 2005/03/22 08:31:08 robbat2 Exp $";

static int flag_debug = 0;
static int flag_verbose = 0;
static int flag_version = 0;
static int flag_help = 0;
static int flag_fields = 0;
static char* param_fields = NULL;
static struct option long_options[] = {
	{"verbose", 0, &flag_verbose, 1},
	{"debug", 0, &flag_debug, 1},
	{"version", 0, &flag_version, 1},
	{"help", 0, &flag_help, 1},
	{"fields", 1, &flag_fields, 1},
	{0, 0, 0, 0}
};
static char* short_options = "fvdhV";

struct mapkey {
	struct stat *sb;
	long first_block;
	char* filename;
};

typedef enum OrderField_Type { ST_DEV,ST_INO,ST_MODE,ST_NLINK,ST_UID,ST_GID,ST_RDEV,ST_SIZE,ST_BLKSIZE,ST_BLOCKS,ST_ATIME,ST_MTIME,ST_CTIME,IOCTL_FIBMAP,FILENAME };
struct OrderField {
		OrderField_Type type;
		bool reverse;
		OrderField(OrderField_Type t, bool r) : 
			type(t), reverse(r) {
		}
};

inline const int numcmp(const long a, const long b) {
	DEBUG_STACK_UP;
	DEBUG("%s\n","numcmp-init");
	int ret = 0;
	if(a > b) {
		ret = -1;
	} else if(a < b) {
		ret = 1;
	} 
	DEBUG("%s\n","numcmp-done");
	DEBUG_STACK_DOWN;
	return ret;
}

inline const long func_ST_DEV(mapkey a) 		{ return ((a).sb->st_dev); }
inline const long func_ST_INO(mapkey a) 		{ return ((a).sb->st_ino); }
inline const long func_ST_MODE(mapkey a) 		{ return ((a).sb->st_mode); }
inline const long func_ST_NLINK(mapkey a) 		{ return ((a).sb->st_nlink); }
inline const long func_ST_UID(mapkey a) 		{ return ((a).sb->st_uid); }
inline const long func_ST_GID(mapkey a) 		{ return ((a).sb->st_gid); }
inline const long func_ST_RDEV(mapkey a) 		{ return ((a).sb->st_rdev); }
inline const long func_ST_SIZE(mapkey a) 		{ return ((a).sb->st_size); }
inline const long func_ST_BLKSIZE(mapkey a) 	{ return ((a).sb->st_blksize); }
inline const long func_ST_BLOCKS(mapkey a) 		{ return ((a).sb->st_blocks); }
inline const long func_ST_ATIME(mapkey a) 		{ return ((a).sb->st_atime); }
inline const long func_ST_MTIME(mapkey a) 		{ return ((a).sb->st_mtime); }
inline const long func_ST_CTIME(mapkey a) 		{ return ((a).sb->st_ctime); }
inline const long func_IOCTL_FIBMAP(mapkey a)	{ return ((a).first_block); }
inline const char* func_FILENAME(mapkey a) 		{ return ((a).filename); }

inline const int cmp_ST_DEV(mapkey a, mapkey b) 	{ return  numcmp(func_ST_DEV(a),func_ST_DEV(b)); }
inline const int cmp_ST_INO(mapkey a, mapkey b) 	{ return  numcmp(func_ST_INO(a),func_ST_INO(b)); }
inline const int cmp_ST_MODE(mapkey a, mapkey b) 	{ return  numcmp(func_ST_MODE(a),func_ST_MODE(b)); }
inline const int cmp_ST_NLINK(mapkey a, mapkey b) 	{ return  numcmp(func_ST_NLINK(a),func_ST_NLINK(b)); }
inline const int cmp_ST_UID(mapkey a, mapkey b) 	{ return  numcmp(func_ST_UID(a),func_ST_UID(b)); }
inline const int cmp_ST_GID(mapkey a, mapkey b) 	{ return  numcmp(func_ST_GID(a),func_ST_GID(b)); }
inline const int cmp_ST_RDEV(mapkey a, mapkey b)	{ return  numcmp(func_ST_RDEV(a),func_ST_RDEV(b)); }
inline const int cmp_ST_SIZE(mapkey a, mapkey b)	{ return  numcmp(func_ST_SIZE(a),func_ST_SIZE(b)); }
inline const int cmp_ST_BLKSIZE(mapkey a, mapkey b) { return  numcmp(func_ST_BLKSIZE(a),func_ST_BLKSIZE(b)); }
inline const int cmp_ST_BLOCKS(mapkey a, mapkey b) 	{ return  numcmp(func_ST_BLOCKS(a),func_ST_BLOCKS(b)); }
inline const int cmp_ST_ATIME(mapkey a, mapkey b) 	{ return  numcmp(func_ST_ATIME(a),func_ST_ATIME(b)); }
inline const int cmp_ST_MTIME(mapkey a, mapkey b) 	{ return  numcmp(func_ST_MTIME(a),func_ST_MTIME(b)); }
inline const int cmp_ST_CTIME(mapkey a, mapkey b) 	{ return  numcmp(func_ST_CTIME(a),func_ST_CTIME(b)); }
inline const int cmp_IOCTL_FIBMAP(mapkey a, mapkey b) { return  numcmp(func_IOCTL_FIBMAP(a),func_IOCTL_FIBMAP(b)); }
inline const int cmp_FILENAME(mapkey a, mapkey b) 	{ return  strcmp(func_FILENAME(a),func_FILENAME(b)); }

#define CMP_NE_RET if(cmp != 0) { DEBUG("%s:(%d)\n","mapcmp-ne-shortcircuit",cmp); DEBUG_STACK_DOWN; return cmp; }
int mapcmp(const mapkey *a, const mapkey *b, vector<OrderField*> *ofa) {
	int cmp = 0;

	DEBUG_STACK_UP;
	DEBUG("%s:(%x,%x)\n","mapcmp-init",a,b);
	// must check for null first
	bool a_null = (a == NULL);
	bool b_null = (b == NULL);
	if(a_null) {
		cmp = (b_null ? 0 : 1);
	} else if(b_null) {
		cmp = -1;
	}
	DEBUG("%s:(%d)\n","mapcmp-null-check",cmp);
	CMP_NE_RET;

	
	// now we can check for all others
	mapkey mk_a = *a;
	mapkey mk_b = *b;
	DEBUG("%s\n","mapcmp-outside-loop");

	if(ofa->size() > 0) {
		for(vector<OrderField*>::iterator it = ofa->begin(); it != ofa->end(); ++it) {
			DEBUG("%s\n","mapcmp-inside-loop#init");
			OrderField *ofp = *it;
			OrderField of = *ofp;
			DEBUG("%s:(%d)\n","mapcmp-inside-loop#get-type",(int)of.type);
#define case_entry(func) cmp = cmp_##func(mk_a,mk_b); break;
			switch(of.type) {
				case ST_DEV: case_entry(ST_DEV);
				case ST_INO: case_entry(ST_INO);
				case ST_MODE: case_entry(ST_MODE);
				case ST_NLINK: case_entry(ST_NLINK);
				case ST_UID: case_entry(ST_UID);
				case ST_GID: case_entry(ST_GID);
				case ST_RDEV: case_entry(ST_RDEV);
				case ST_SIZE: case_entry(ST_SIZE);
				case ST_BLKSIZE: case_entry(ST_BLKSIZE);
				case ST_BLOCKS: case_entry(ST_BLOCKS);
				case ST_ATIME: case_entry(ST_ATIME);
				case ST_MTIME: case_entry(ST_MTIME);
				case ST_CTIME: case_entry(ST_CTIME);
				case IOCTL_FIBMAP: case_entry(IOCTL_FIBMAP);
				case FILENAME: case_entry(FILENAME);
			}
#undef case_entry
			cmp = cmp * (of.reverse ? -1 : 1);
			CMP_NE_RET;
		}
	}
	DEBUG("%s\n","mapcmp-done-loop");
	DEBUG_STACK_DOWN;
	return cmp;
}
#undef CMP_NE_RET

mapkey* build_mapkey(char *filename) {
	DEBUG_STACK_UP;
	DEBUG("%s\n","build_mapkey-init");
	int fd;
	mapkey *mk = NULL;

	fd = open(filename, O_RDONLY);
	if(fd >= 0) {
		struct stat *tmpsb = new (struct stat);
		if( fstat( fd, tmpsb ) == -1 || S_ISDIR(tmpsb->st_mode)) {
			// bad, do nothing
		} else {
			mk = new mapkey;
			mk->sb = tmpsb;
			// if you ever see this in the debug output, something is wrong.
			mk->first_block = -2; 
			mk->filename = filename;
			int block = 0; // temp space
			int ioctl_ret = ioctl( fd, FIBMAP, &block );
			if( ioctl_ret == -1 ) {
				if(flag_debug) {
					DEBUG("ioctl failed(%d): %s\n",ioctl_ret,filename);
				}
				// not available on some filesystems, like reiserfs
				// goto failure; but this is NOT a permanant failure
				block = -3;
			}
			mk->first_block = block;
		}
	}
	close(fd);
	DEBUG("build_mapkey-done\n");
	DEBUG_STACK_DOWN;
	return mk;
}

vector <OrderField*> *myOrder;

struct maplt {
	bool operator()(const mapkey *a, const mapkey *b) const {
		int cmp;
		DEBUG_STACK_UP;
		DEBUG("maplt-init\n");
		cmp = mapcmp(a,b,myOrder);
		DEBUG("maplt-done\n");
		DEBUG_STACK_DOWN;
		return cmp > 0;
	}
};

#define MULTIMAP_KEY mapkey*
#define MULTIMAP_DATA const char *
#define MULTIMAP_LT maplt
#define MULTIMAP_COMPLETE_TYPE multimap<MULTIMAP_KEY,MULTIMAP_DATA,MULTIMAP_LT>
#define PAIR_COMPLETE_TYPE pair<MULTIMAP_KEY,MULTIMAP_DATA>

void printItem(PAIR_COMPLETE_TYPE p,vector <OrderField*> *ofa) {
	DEBUG_STACK_UP;
	DEBUG("printItems-init:(%x,%x)\n",p.first,p.second);
	const char* filename = p.second;
	DEBUG("printItems-got-filename:(%s)\n",filename);
	mapkey mk = *(p.first);
	DEBUG("printItems-got-mapkey:(%x)\n",mk);
	bool first = true;
	if(flag_verbose) {
			int i = 0;
			for(vector<OrderField*>::iterator it = ofa->begin(); it != ofa->end(); ++it) {
				OrderField *ofp = NULL;
				ofp = *it;
				if(ofp == NULL) {
					DEBUG("printItem-null-ofp:(%x)\n",ofp);
					continue;
				}
				OrderField of = *ofp;
#define case_entry(fmt,func) printf("%s"fmt,(first ? "" : " "),func_##func(mk)); break;
				switch(of.type) {
					case ST_DEV: case_entry("%ld",ST_DEV);
					case ST_INO: case_entry("%ld",ST_INO);
					case ST_MODE: case_entry("%ld",ST_MODE);
					case ST_NLINK: case_entry("%ld",ST_NLINK);
					case ST_UID: case_entry("%ld",ST_UID);
					case ST_GID: case_entry("%ld",ST_GID);
					case ST_RDEV: case_entry("%ld",ST_RDEV);
					case ST_SIZE: case_entry("%ld",ST_SIZE);
					case ST_BLKSIZE: case_entry("%ld",ST_BLKSIZE);
					case ST_BLOCKS: case_entry("%ld",ST_BLOCKS);
					case ST_ATIME: case_entry("%ld",ST_ATIME);
					case ST_MTIME: case_entry("%ld",ST_MTIME);
					case ST_CTIME: case_entry("%ld",ST_CTIME);
					case IOCTL_FIBMAP: case_entry("%ld",IOCTL_FIBMAP);
					case FILENAME:	case_entry("%s",FILENAME);
				}
#undef case_entry
				first = false;
				i++;
			}
			printf("\n");
	}  else {
		printf("%s%s\n",(first ? "" : " "),filename);
	}
	DEBUG("%s\n","printItems-done");
	DEBUG_STACK_DOWN;
}

void process_input(MULTIMAP_COMPLETE_TYPE &m) {
	char buffer[BUFFER_SIZE];
	while(cin.good()) {
		cin.getline(buffer,BUFFER_SIZE);
		int filename_len = cin.gcount()-1; //faster than strlen
		if(filename_len > 0 && buffer[0] != '#') {
			char* filename = new char[filename_len+1];
			strncpy(filename,buffer,filename_len+1);
			DEBUG_ON;
			DEBUG("%s:(%s)\n","main-got-filename",filename);
			DEBUG_OFF;
			mapkey* mk = build_mapkey(filename);
			if(mk != NULL) { 
				DEBUG("%s\n","main-create-pair");
				PAIR_COMPLETE_TYPE p = PAIR_COMPLETE_TYPE(mk,filename);
				DEBUG("%s\n","main-insert-pair");
				m.insert(p);
				DEBUG("%s\n","main-insert-pair-done");
			} else {
				DEBUG("%s\n","main-key-failed");
			}
		}
	}
}

void process_output(MULTIMAP_COMPLETE_TYPE &m) {
	for (MULTIMAP_COMPLETE_TYPE::iterator it = m.begin(); it != m.end(); ++it) {
		printItem(*it,myOrder);
	}
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

void process_opts(int argc, char** argv) {
	myOrder = new vector<OrderField*>;
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
					case 4: // fields
						strcpy(param_fields,optarg);
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
			case 'f':
				flag_fields = 1;
				strcpy(param_fields,optarg);
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
	if(flag_fields) {
		// TODO: parse param_fields
		WARNING("TODO: parse param_fields\n");
		exit(-2);
	} else {
		// default fields
		myOrder->push_back(new OrderField(ST_DEV,false));
		myOrder->push_back(new OrderField(IOCTL_FIBMAP,false));
		myOrder->push_back(new OrderField(ST_INO,false));
		myOrder->push_back(new OrderField(FILENAME,false));
	}
}


int main(int argc, char** argv) {
	DEBUG_STACK_UP;
	if(getuid() != 0) {
		WARNING("ioctl(FIBMAP) is limited to root only! Results may be less than optimal.\n");
	}
	MULTIMAP_COMPLETE_TYPE  m;
	process_opts(argc,argv);
	process_input(m);
	process_output(m);
	DEBUG_STACK_DOWN;
}

// vim: ts=4 sw=4: