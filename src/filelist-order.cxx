// Copyright (C) 2005 Robin Hugh Johnson <robbat2@orbis-terrarum.net>
//  
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <vector>

#include <string.h>
#include <getopt.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <fcntl.h>
#include <linux/fs.h>
#include <../config.h>

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 2048
#endif

#define _DEBUG_MSG(fmt,msg...) { fprintf(stderr,fmt, ##msg); fflush(stderr); };
#define WARNING(fmt,msg...) _DEBUG_MSG(fmt,##msg);
#ifdef INSANE_DEBUG_MODE
int __STACK_DEBUG__ = 0;
#define DEBUG(fmt,msg...) { if(flag_debug) { _DEBUG_MSG("%d:%s:%d:",__STACK_DEBUG__,__FILE__,__LINE__); _DEBUG_MSG(fmt, ##msg ); } }
#define DEBUG_STACK_UP { if(flag_debug) { __STACK_DEBUG__++; } }
#define DEBUG_STACK_DOWN { if(flag_debug) { __STACK_DEBUG__--; } }
#else
#define DEBUG(fmt,msg...)
#define DEBUG_STACK_UP
#define DEBUG_STACK_DOWN
#endif //INSANE_DEBUG_MODE

using namespace std;

static char* program_name = "readhead-list";
static char* program_header = "$Header: /code/convert/cvsroot/infrastructure/readahead-list/src/filelist-order.cxx,v 1.3 2005/03/28 06:33:15 robbat2 Exp $";
static char* program_id = "$Id: filelist-order.cxx,v 1.3 2005/03/28 06:33:15 robbat2 Exp $";

static int flag_input_file = 0;
static int flag_input_stdin = 0;
static int flag_debug = 0;
static int flag_verbose = 0;
static int flag_version = 0;
static int flag_help = 0;
static int flag_fields = 0;
static int flag_rootneeded = 0;

static struct option long_options[] = {
	{"verbose", 0, &flag_verbose, 1},
	{"debug", 0, &flag_debug, 1},
	{"version", 0, &flag_version, 1},
	{"help", 0, &flag_help, 1},
	//{"fields", 1, &flag_fields, 1},
	{"fields", 1, &flag_fields, 1},
	{0, 0, 0, 0}
};
static char* short_options = "f:vdhV";

// 64-bit integers are needed for some fields
#define BASE_DATATYPE long long int

struct mapkey {
	struct stat *sb;
	BASE_DATATYPE first_block;
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

inline const int numcmp(const BASE_DATATYPE a, const BASE_DATATYPE b) {
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

inline const BASE_DATATYPE func_ST_DEV(mapkey a) 		{ return ((a).sb->st_dev); }
inline const BASE_DATATYPE func_ST_INO(mapkey a) 		{ return ((a).sb->st_ino); }
inline const BASE_DATATYPE func_ST_MODE(mapkey a) 		{ return ((a).sb->st_mode); }
inline const BASE_DATATYPE func_ST_NLINK(mapkey a) 		{ return ((a).sb->st_nlink); }
inline const BASE_DATATYPE func_ST_UID(mapkey a) 		{ return ((a).sb->st_uid); }
inline const BASE_DATATYPE func_ST_GID(mapkey a) 		{ return ((a).sb->st_gid); }
inline const BASE_DATATYPE func_ST_RDEV(mapkey a) 		{ return ((a).sb->st_rdev); }
inline const BASE_DATATYPE func_ST_SIZE(mapkey a) 		{ return ((a).sb->st_size); }
inline const BASE_DATATYPE func_ST_BLKSIZE(mapkey a) 	{ return ((a).sb->st_blksize); }
inline const BASE_DATATYPE func_ST_BLOCKS(mapkey a) 		{ return ((a).sb->st_blocks); }
inline const BASE_DATATYPE func_ST_ATIME(mapkey a) 		{ return ((a).sb->st_atime); }
inline const BASE_DATATYPE func_ST_MTIME(mapkey a) 		{ return ((a).sb->st_mtime); }
inline const BASE_DATATYPE func_ST_CTIME(mapkey a) 		{ return ((a).sb->st_ctime); }
inline const BASE_DATATYPE func_IOCTL_FIBMAP(mapkey a)	{ return ((a).first_block); }
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
// note that this one is backwards
inline const int cmp_FILENAME(mapkey a, mapkey b) 	{ return  strcmp(func_FILENAME(b),func_FILENAME(a)); }

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
				DEBUG("ioctl failed(%d): %s\n",ioctl_ret,filename);
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
					case ST_DEV: case_entry("%lld",ST_DEV);
					case ST_INO: case_entry("%lld",ST_INO);
					case ST_MODE: case_entry("%lld",ST_MODE);
					case ST_NLINK: case_entry("%lld",ST_NLINK);
					case ST_UID: case_entry("%lld",ST_UID);
					case ST_GID: case_entry("%lld",ST_GID);
					case ST_RDEV: case_entry("%lld",ST_RDEV);
					case ST_SIZE: case_entry("%lld",ST_SIZE);
					case ST_BLKSIZE: case_entry("%lld",ST_BLKSIZE);
					case ST_BLOCKS: case_entry("%lld",ST_BLOCKS);
					case ST_ATIME: case_entry("%lld",ST_ATIME);
					case ST_MTIME: case_entry("%lld",ST_MTIME);
					case ST_CTIME: case_entry("%lld",ST_CTIME);
					case IOCTL_FIBMAP: case_entry("%lld",IOCTL_FIBMAP);
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

void process_input(istream *is, MULTIMAP_COMPLETE_TYPE &m) {
	char buffer[BUFFER_SIZE];
	while(is->good()) {
		is->getline(buffer,BUFFER_SIZE);
		int filename_len = is->gcount()-1; //faster than strlen
		if(filename_len > 0 && buffer[0] != '#') {
			char* filename = new char[filename_len+1];
			strncpy(filename,buffer,filename_len+1);
			DEBUG("%s:(%s)\n","main-got-filename",filename);
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

void error_exit(char* msg) {
	skel_command_msg_exit(stderr,msg,1);
}

void command_error() {
#define LEN 1024
	char s[LEN];
	snprintf(s,LEN,"Try `%s --help' for more information.\n",program_name);
#undef LEN
	error_exit(s);
}

void command_version() {
#define LEN 1024
	char s[LEN];
	snprintf(s,LEN,"%s: %s\n",program_name,PACKAGE_VERSION);
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
			"  [-f|--fields] F1,F2,...  Specify custom sorting fields.\n"\
			"  [-v|--verbose]           Print all calculation fields in result.\n"\
			"%s"\
			"  [-h|--help]              Stop looking at me!\n"\
			"  [-V|--version]           As the name says.\n"\
			"\n"\
			"Fields for -f (default position in brackets):\n"\
			"  stat.st_dev     - device\n"\
			"  stat.st_ino     - inode\n"\
			"  stat.st_mode    - protection\n"\
			"  stat.st_nlink   - number of hard links\n"\
			"  stat.st_uid     - user ID of owner\n"\
			"  stat.st_gid     - group ID of owner\n"\
			"  stat.st_rdev    - device type (if inode device)\n"\
			"  stat.st_size    - total size, in bytes\n"\
			"  stat.st_blksize - blocksize for filesystem I/O\n"\
			"  stat.st_blocks  - number of blocks allocated\n"\
			"  stat.st_atime   - time of last access\n"\
			"  stat.st_mtime   - time of last modification\n"\
			"  stat.st_ctime   - time of last status change\n"\
			"  ioctl.fibmap    - position of first block of file (root only)\n"\
			"  raw.filename    - the filename, including full path\n"\
			"Prefix a field with '-' to invert it's order.\n"\
			"Default order is stat.st_dev,ioctl.fibmap,stat.st_ino\n"\
			,program_name,
#ifdef INSANE_DEBUG_MODE
			"  [-d|--debug]             Print internal debugging.\n"
#else
			""
#endif //INSANE_DEBUG_MODE
			);
	fprintf(stdout,s);
#undef LEN
}

void process_fieldorder(char* orderString) {
	assert(orderString != NULL);
	bool field_filename_done = false;
	DEBUG("param(hex): %x\n",orderString);
	DEBUG("param(str): %s\n",orderString);
	DEBUG("param length: %d\n",strlen(orderString));
	char **inputp = &orderString;
	const char* delim = " ,:;";
	DEBUG("process_fieldorder-initial-strsep(delim='%s',inputp=%x,*inputp=%x)\n",delim,inputp,*inputp);
	char* tok = strsep (inputp,delim);
	DEBUG("process_fieldorder-strsep-result: %x\n",tok);
	while(tok != NULL) {
		DEBUG("process_fieldorder-got-token: %s\n",tok);
		bool reverse = false;
		OrderField_Type oft;
		// check for leading -
		if(tok[0] == '-') {
			reverse = true;
			// force it null to compare easier
			tok[0] = 0; 
			// and advance
			tok++;
		}
		// time for some fun
#define CMP(str,res)	if(0 == strcmp(tok,str)) { oft = res; }
#define ECMP(str,res)	else CMP(str,res)
		CMP("stat.st_dev",ST_DEV)
			ECMP("stat.st_ino",ST_INO)
			ECMP("stat.st_mode",ST_MODE)
			ECMP("stat.st_nlink",ST_NLINK)
			ECMP("stat.st_uid",ST_UID)
			ECMP("stat.st_gid",ST_GID)
			ECMP("stat.st_rdev",ST_RDEV)
			ECMP("stat.st_size",ST_SIZE)
			ECMP("stat.st_blksize",ST_BLKSIZE)
			ECMP("stat.st_blocks",ST_BLOCKS)
			ECMP("stat.st_atime",ST_ATIME)
			ECMP("stat.st_mtime",ST_MTIME)
			ECMP("stat.st_ctime",ST_CTIME)
			ECMP("ioctl.fibmap",IOCTL_FIBMAP)
			ECMP("raw.filename",FILENAME)
		else {
			DEBUG("process_fieldorder-got-bad-token: %s\n",tok);
			command_error();
		}
		// add to list
		myOrder->push_back(new OrderField(oft,reverse));
		// for forcing uniqueness
		if(oft == FILENAME) {
			field_filename_done = true;
		}
		// FIBMAP needs root
		if(oft == IOCTL_FIBMAP) {
			flag_rootneeded = 1;
		}
		// next run
		tok = strsep(inputp,delim);
	}
	DEBUG("process_fieldorder-done-parse-loop\n");
	// force in filename if it isn't there
	if(!field_filename_done) {
		DEBUG("process_fieldorder-forcing-filename\n");
		myOrder->push_back(new OrderField(FILENAME,false));
	}
}

void process_opts(int argc, char** argv) {
	myOrder = new vector<OrderField*>;
	char *param_fields = NULL;
	program_name = argv[0];
	DEBUG("before-loop: optind:%d opterr:%d optopt:%d optarg:%x\n",optind,opterr,optopt,optarg);
	while(1) {
		int long_index,c;
		long_index = -1;
		optarg = NULL;
		c = getopt_long(argc,argv,short_options,long_options,&long_index);
		DEBUG("inside-loop: optind:%d opterr:%d optopt:%d long_index:%d optarg:%x\n",optind,opterr,optopt,long_index,optarg);
		if(optarg != NULL) {
			DEBUG("inside-loop: optarg-str:%s\n",optarg);
		}
		if (c == -1)
			break;
		switch(c) {
			// is this a long option?
			case 0:
			case '-': 
				switch(long_index) {
					// handled by getopt directly
					case 0: // verbose
						flag_verbose = 1;
						break;
					case 1: // debug
						flag_debug = 1;
						break;
					case 2: // version
						flag_version = 1;
						break;
					case 3: // help
						flag_help = 1;
						break;
					case 4: // fields
						flag_fields = 1;
						int len = strlen(optarg);
						param_fields = (new char[len]);
						strncpy(param_fields,optarg,len);
						break;
					default:
						command_error();
						break;
				}
				break;
				// nope, short option
			case 'v':
				flag_verbose = 1;
				break;
			case 'd':
				flag_debug = 1;
				break;
			case 'h':
				flag_help = 1;
				break;
			case 'V':
				flag_version = 1;
				break;
			case 'f':
				flag_fields = 1;
				int len = strlen(optarg);
				param_fields = (new char[len]);
				strncpy(param_fields,optarg,len);
				break;
			default:
				command_error();
		}
	}
#ifndef INSANE_DEBUG_MODE
	if(flag_debug) {
		command_error();
	}
#endif //INSANE_DEBUG_MODE
	if(flag_help) {
		command_help();
	}
	if(flag_version) {
		command_version();
	}
	if(flag_version || flag_help) {
		exit(0);
	}
	// handle the default
	if(param_fields == NULL) {
		flag_fields = 1;
		param_fields = "stat.st_dev,ioctl.fibmap,stat.st_ino,raw.filename";
	}
	process_fieldorder(param_fields);
}


int main(int argc, char** argv) {
	DEBUG_STACK_UP;
	MULTIMAP_COMPLETE_TYPE  m;
	set<const char*> hs;

	process_opts(argc,argv);
	// safety
	if(optind == 0) {
		optind = argc;
	}
	if(optind > 0 && flag_rootneeded && getuid() != 0) {
		WARNING("ioctl(FIBMAP) is limited to root only! Results may be less than optimal.\n");
	}
	for(int i = optind; i<argc;i++) {
		istream *is;
		if(strcmp(argv[i],"-") == 0) {
			flag_input_stdin = 1;
			is = &cin;
		} else {
			flag_input_file = 1;
			is = new ifstream(argv[i]);
		}
		process_input(is,m);
	}
	if(flag_input_file == 0 && flag_input_stdin == 0) {
		process_input(&cin,m);
	}
	process_output(m);
	DEBUG_STACK_DOWN;
}

// vim: ts=4 sw=4:
