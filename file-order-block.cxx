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

#define func_ST_DEV(a) ((a).sb->st_dev)
#define func_ST_INO(a) ((a).sb->st_ino)
#define func_ST_MODE(a) ((a).sb->st_mode)
#define func_ST_NLINK(a) ((a).sb->st_nlink)
#define func_ST_UID(a) ((a).sb->st_uid)
#define func_ST_GID(a) ((a).sb->st_gid)
#define func_ST_RDEV(a) ((a).sb->st_rdev)
#define func_ST_SIZE(a) ((a).sb->st_size)
#define func_ST_BLKSIZE(a) ((a).sb->st_blksize)
#define func_ST_BLOCKS(a) ((a).sb->st_blocks)
#define func_ST_ATIME(a) ((a).sb->st_atime)
#define func_ST_MTIME(a) ((a).sb->st_mtime)
#define func_ST_CTIME(a) ((a).sb->st_ctime)
#define func_IOCTL_FIBMAP(a) ((a).first_block)

using namespace std;

bool flag_debug = true;

struct mapkey {
	struct stat *sb;
	int first_block;
};

typedef enum OrderField_Type { ST_DEV,ST_INO,ST_MODE,ST_NLINK,ST_UID,ST_GID,ST_RDEV,ST_SIZE,ST_BLKSIZE,ST_BLOCKS,ST_ATIME,ST_MTIME,ST_CTIME,IOCTL_FIBMAP };
struct OrderField {
	OrderField_Type type;
	bool reverse;
};

inline int numcmp(const int a, const int b) {
	//fprintf(stderr,"%s:%d:%s\n",__FILE__,__LINE__,"numcmp-init");
	if(a > b) {
		return -1;
	} else if(a < b) {
		return 1;
	} else {
		return 0;
	}
	//fprintf(stderr,"%s:%d:%s\n",__FILE__,__LINE__,"numcmp-done");
}


// a < b
int mapcmp(const mapkey *a, const mapkey *b, vector<OrderField*> ofa) {
#define order_func(func,varname) val_##varname = func_##func(mk_##varname);
#define order_func_both(func) order_func(func,a) order_func(func,b)
#define case_entry(func) order_func_both(func); break;
#define CMP_NE_RET if(cmp != 0) { return cmp; }
	//fprintf(stderr,"%s:%d:%s\n",__FILE__,__LINE__,"mapcmp-init");
	// must check for null first
	bool a_null = (a == NULL);
	bool b_null = (b == NULL);
	if(a_null) {
		return (b_null ? 0 : 1);
	} else if(b_null) {
		return -1;
	}
	// now we can check for all others
	int cmp = 0;
	//ofa = {ST_DEV,IOCTL_FIBMAP,ST_INO};
	mapkey mk_a = *a;
	mapkey mk_b = *b;
	//fprintf(stderr,"%s:%d:%s\n",__FILE__,__LINE__,"mapcmp-outside-loop");
	if(ofa.size() > 0) {
		for(vector<OrderField*>::iterator ofp = ofa.begin(); ofp != ofa.end(); ++ofp ) {
			//fprintf(stderr,"%s:%d:%s\n",__FILE__,__LINE__,"mapcmp-inside-loop#1");
			int val_a, val_b;
			bool rev = false;
			OrderField of = *(*ofp);
			//fprintf(stderr,"%s:%d:%s:(%d)\n",__FILE__,__LINE__,"mapcmp-inside-loop#2",(int)of.type);
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
			cmp = numcmp(val_a,val_b) * (of.reverse ? -1 : 1);
			CMP_NE_RET;
		}
	}
	//fprintf(stderr,"%s:%d:%s\n",__FILE__,__LINE__,"mapcmp-done-loop");
	return cmp;
#undef order_func
#undef order_func_both
#undef case_entry
#undef CMP_NE_RET
}

mapkey* build_mapkey(char *filename) {
	fprintf(stderr,"%s:%d:%s\n",__FILE__,__LINE__,"build_mapkey-init");
	int fd;

	mapkey *mk = new mapkey;
	mk->sb = new (struct stat);

	// if you ever see this in the debug output, something is wrong.
	mk->first_block = -2; 


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
	fprintf(stderr,"%s:%d:%s\n",__FILE__,__LINE__,"build_mapkey-done");
	return mk;
}

vector <OrderField*> myOrder;

struct maplt {
	bool operator()(const mapkey *a, const mapkey *b) const {
		//cerr << "maplt" << endl;
		int cmp = mapcmp(a,b,myOrder);
		return cmp > 0;
	}
};

#define MULTIMAP_KEY mapkey*
#define MULTIMAP_DATA const char *
#define MULTIMAP_LT maplt
#define MULTIMAP_COMPLETE_TYPE multimap<MULTIMAP_KEY,MULTIMAP_DATA,MULTIMAP_LT>
#define PAIR_COMPLETE_TYPE pair<MULTIMAP_KEY,MULTIMAP_DATA>

void printItems(PAIR_COMPLETE_TYPE p) {
	//fprintf(stderr,"%s:%d:%s\n",__FILE__,__LINE__,"printItems-init");
	mapkey mk = *(p.first);
	const char* filename = p.second;
	fprintf(stderr,"%s:%d:%s:(%x,%s)\n",__FILE__,__LINE__,"printItems-print",mk,filename);
	if(flag_debug) {
		//fprintf(stderr,"%d %d %d %s\n",func_ST_DEV(mk), func_IOCTL_FIBMAP(mk) , func_ST_INO(mk), filename);
		//fprintf(stderr,"%d %d %d %s\n",func_ST_DEV(mk), func_IOCTL_FIBMAP(mk) , func_ST_INO(mk), "badfilename");
		fprintf(stderr,"%d ",func_ST_DEV(mk));
		fprintf(stderr,"%d ",func_IOCTL_FIBMAP(mk));
		fprintf(stderr,"%d ",func_ST_INO(mk));
		fprintf(stderr,"%s\n",filename);
	} else {
		printf("%s\n",filename);
	}
	//fprintf(stderr,"%s:%d:%s\n",__FILE__,__LINE__,"printItems-done");
}

int main(int argc, char** argv) {
	if(getuid() != 0) {
		fprintf(stderr,"ioctl(FIBMAP) is limited to root only! Results may be less than optimal.\n");
	}

	//myOrder.push_back(new OrderField(IOCTL_FIBMAP,true));

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
				fprintf(stderr,"%s:%d:%s\n",__FILE__,__LINE__,"main-create-pair");
				PAIR_COMPLETE_TYPE p = PAIR_COMPLETE_TYPE(mk,filename);
				printItems(p);
				fprintf(stderr,"%s:%d:%s\n",__FILE__,__LINE__,"main-insert-pair");
				m.insert(p);
			} else {
				fprintf(stderr,"%s:%d:%s\n",__FILE__,__LINE__,"main-key-failed");
				//cerr << "key failed: " << filename << endl;
			}
		}
	}
	fprintf(stderr,"Results:\n");
  for (MULTIMAP_COMPLETE_TYPE::iterator it = m.begin(); it != m.end(); ++it) {
	  printItems(*it);
  }
}

// vim: ts=4 sw=4:
