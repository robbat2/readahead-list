#include <map>
#include <iostream>

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
#define func_ST_DEV(a) ((a).sb.st_dev)
#define func_ST_INO(a) ((a).sb.st_ino)
#define get_IOCTL_FIBMAP(a) ((a).first_block)

using namespace std;

bool flag_debug = true;

struct mapkey {
	struct stat sb;
	int first_block;
};

enum OrderField_Type = { ST_DEV,ST_INO,ST_MODE,ST_NLINK,ST_UID,ST_GID,ST_RDEV,ST_SIZE,ST_BLKSIZE,ST_BLOCKS,ST_ATIME,ST_MTIME,ST_CTIME,IOCTL_FIBMAP };
struct OrderField {
	OrderField_Type type;
	boolean reverse;
}

inline int numcmp(const int a, const int b) {
	if(a > b) {
		return -1;
	} else if(a < b) {
		return 1;
	} else {
		return 0;
	}
}


// a < b
int mapcmp(const mapkey *a, const mapkey *b, OrderField ofa[], int of_size) {
#define CMP_NE_RET if(cmp != 0) { return cmp; }
	// must check for null first
	bool a_null = (a == NULL);
	bool b_null = (b == NULL);
	if(a_null) {
		return (b_null ? 0 : 1);
	} else if(b_null) {
		return -1;
	}
	// now we can check for all others
	int cmp;
	ofa = {ST_DEV,IOCTL_FIBMAP,ST_INO};
#define order_func(func,val) func_#func(val);
#define order_func_both(func) order_func(func,val_a) order_func(func,val_b)
#define case_entry(func) order_func_both(func); break;
	for(int i = 0; i < of_size; i++) {
		int val_a, val_b;
		bool rev = false;
		OrderField of = ofa[i];
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
		}
		cmp = numcmp(a,b) * (of.reverse ? -1 : 1);
		CMP_NE_RET;
	}
	return cmp;
#undef CMP_NE_RET
}

mapkey* build_mapkey(char *filename) {
	int fd;

	mapkey *mk = new mapkey;

	// if you ever see this in the debug output, something is wrong.
	mk->first_block = -2; 


	fd = open(filename, O_RDONLY);
	if(fd >= 0) {
		if( fstat( fd, &(mk->sb) ) == -1 || !S_ISREG((mk->sb).st_mode)) {
			delete mk;
			mk = NULL;
		} else {
			int block = 0; // temp space
			int ioctl_ret = ioctl( fd, FIBMAP, &block );
			if( ioctl_ret == -1 ) {
				if(flag_debug) {
					fprintf(stderr,"ioctl failed(%d): %s\n",ioctl_ret,filename);
				}
				// not available on some filesystems, like reiserfs
				// goto failure; but this is NOT a permanant failure
				block = -3;
			}
			mk->first_block = block;
		}
	} 
	close(fd);
	return mk;
}

struct maplt {
	bool operator()(const mapkey *a, const mapkey *b) const {
		int cmp = mapcmp(a,b);
		return cmp > 0;
	}
};

#define MULTIMAP_KEY mapkey*
#define MULTIMAP_DATA const char *
#define MULTIMAP_LT maplt
#define MULTIMAP_COMPLETE_TYPE multimap<MULTIMAP_KEY,MULTIMAP_DATA,MULTIMAP_LT>
#define PAIR_COMPLETE_TYPE pair<MULTIMAP_KEY,MULTIMAP_DATA>

void printItems(PAIR_COMPLETE_TYPE p) {
	if(flag_debug) {
		cerr << func_ST_DEV(*(p.first)) << "\t" << get_IOCTL_FIBMAP(*(p.first)) << "\t" << func_ST_INO(*(p.first)) << "\t" << p.second << endl;
	} else {
		cout << p.second << endl;
	}
}

int main(int argc, char** argv) {
	if(getuid() != 0) {
		fprintf(stderr,"ioctl(FIBMAP) is limited to root only! Results may be less than optimal.\n");
	}
	MULTIMAP_COMPLETE_TYPE  m;
	// note that reiserfs doesn't implement FIBMAP!
	char buffer[BUFFER_SIZE];
	while(cin.good()) {
		cin.getline(buffer,BUFFER_SIZE);
		//printf("%d %d %s\n",strlen(buffer),cin.gcount(),buffer);
		int filename_len = cin.gcount()-1; //faster than strlen
		if(filename_len > 0) {
			char* filename = new char[filename_len+1];
			strncpy(filename,buffer,BUFFER_SIZE);
			mapkey* mk = build_mapkey(filename);
			if(mk != NULL) { 
				m.insert(PAIR_COMPLETE_TYPE(mk,filename));
			} else {
				//cerr << "key failed: " << filename << endl;
			}
		}
	}
  for (MULTIMAP_COMPLETE_TYPE::iterator it = m.begin(); it != m.end(); ++it) {
	  printItems(*it);
  }
}

// vim: ts=4 sw=4:
