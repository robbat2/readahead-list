#include <map>
#include <iostream>
#include <vector>

#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <fcntl.h>
#include <linux/fs.h>

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 4096
#endif

#define _DEBUG_MSG(fmt,msg...) { fprintf(stderr,fmt, ##msg); fflush(stderr); };
#define WARNING(fmt,msg...) _DEBUG_MSG(fmt,##msg);
#ifdef MESSY_DEBUGGING
int __STACK__ = 0;
bool __DEBUG__ = false;
#define DEBUG(fmt,msg...) { if(__DEBUG__) { _DEBUG_MSG("%d:%s:%d:",__STACK__,__FILE__,__LINE__); _DEBUG_MSG(fmt, ##msg ); } }
#define DEBUG_STACK_UP { __STACK__++; }
#define DEBUG_STACK_DOWN { __STACK__--; }
#define DEBUG_ON { __DEBUG__ = true; }
#define DEBUG_OFF { __DEBUG__ = false; }
#else
#define DEBUG_ON
#define DEBUG_OFF
#define DEBUG_STACK_DOWN
#define DEBUG_STACK_UP
#define DEBUG(fmt,msg...)
#endif // MESSY_DEBUGGING

using namespace std;


bool flag_debug = true;
bool flag_verbose = true;

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
			DEBUG("%s:(%d,%d)\n","mapcmp-inside-loop#cmp",val_a,val_b);
			cmp = cmp * (of.reverse ? 1 : -1);
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

	mapkey *mk = new mapkey;
	mk->sb = new (struct stat);

	// if you ever see this in the debug output, something is wrong.
	mk->first_block = -2; 
	mk->filename = filename;

	fd = open(filename, O_RDONLY);
	if(fd >= 0) {
		if( fstat( fd, mk->sb ) == -1 || !S_ISREG(mk->sb->st_mode)) {
			delete mk;
			mk = NULL;
		} else {
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

#define dumpMyOrder() { \
		DEBUG_ON; \
		int i; \
		i = 0; \
		for (vector<OrderField*>::iterator it = myOrder->begin(); it != myOrder->end(); ++it) { \
			DEBUG("main-check-myOrder#1:(%d,%x)\n",i,*it); \
			i++; \
		} \
		DEBUG_OFF; \
}

int main(int argc, char** argv) {
	DEBUG_STACK_UP;
	if(getuid() != 0) {
		WARNING("ioctl(FIBMAP) is limited to root only! Results may be less than optimal.\n");
	}

	myOrder = new vector<OrderField*>;

	myOrder->push_back(new OrderField(ST_DEV,false));
	myOrder->push_back(new OrderField(IOCTL_FIBMAP,false));
	myOrder->push_back(new OrderField(FILENAME,false));
	myOrder->push_back(new OrderField(ST_INO,false));

	//dumpMyOrder();

	MULTIMAP_COMPLETE_TYPE  m;
	// note that reiserfs doesn't implement FIBMAP!
	char buffer[BUFFER_SIZE];

	while(cin.good()) {
		//dumpMyOrder();
		cin.getline(buffer,BUFFER_SIZE);
		int filename_len = cin.gcount()-1; //faster than strlen
		if(filename_len > 0) {
			char* filename = new char[filename_len+1];
			dumpMyOrder(); // it's valid here
			strncpy(filename,buffer,filename_len+1);
			dumpMyOrder(); // but not here
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

	//dumpMyOrder();
	//return -1; 

	DEBUG("Results:\n");
	for (MULTIMAP_COMPLETE_TYPE::iterator it = m.begin(); it != m.end(); ++it) {
		printItem(*it,myOrder);
	}
	DEBUG_STACK_DOWN;
}

// vim: ts=4 sw=4:
