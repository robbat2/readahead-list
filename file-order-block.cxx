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
#define get_device(a) ((a).sb.st_dev)
#define get_inode(a) ((a).sb.st_ino)
#define get_firstblock(a) ((a).first_block)

using namespace std;

bool flag_debug = true;

struct mapkey {
	struct stat sb;
	int first_block;
};

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
inline int mapcmp(const mapkey *a, const mapkey *b) {
#define CMP_NE_RET if(cmp != 0) { return cmp; }
	bool a_null = (a == NULL);
	bool b_null = (b == NULL);
	if(a_null) {
		if(b_null) {
			return 0;
		} else {
			return 1;
		}
	} else if(b_null) {
		return -1;
	}
	int cmp;
	cmp = numcmp(get_device(*a),get_device(*b));
	CMP_NE_RET;
	cmp = numcmp(get_firstblock(*a),get_firstblock(*b));
	CMP_NE_RET;
	cmp = numcmp(get_inode(*a),get_inode(*b));
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
		cerr << get_device(*(p.first)) << "\t" << get_firstblock(*(p.first)) << "\t" << get_inode(*(p.first)) << "\t" << p.second << endl;
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
