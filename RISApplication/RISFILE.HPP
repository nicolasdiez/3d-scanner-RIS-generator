#ifndef __RISFILE_HPP
#define __RISFILE_HPP

/*
 * C++ wrapper for RISFILE handling
 */

/* Range Image Standard (RIS)
 *
 * Created 22/07/94
 */

extern "C" {
#include "risfile.h"
};

class RisFile {
	
protected:
	RISFILE *rfp;
	
public:
	RisFile() { rfp = 0; }
	virtual ~RisFile() {}
	
	virtual int Valid() { return (rfp ? 1 : 0); }
	
	virtual int Create(char *filename, int oflag = O_RDWR|O_BINARY, int pmode = 0666,
			   risparams_t *rpp = 0) {
		risparams_t dummy;
		if (! rpp) {
			dummy.rp_endian = RisNativeEndian();
			rpp = &dummy;
		}
		rfp = RisFileCreate(filename, oflag, pmode, rpp);
		return Valid();
	}
	virtual int Open(char *filename, int oflag = O_RDWR|O_BINARY) {
		rfp = RisFileOpen(filename, oflag);
		return Valid();
	}
	
	RisFile(char *filename, int create = 0) {
		if (create)
			Create(filename);
		else
			Open(filename);
	}
	
	int Attach(int fid, int mode = 0) {
		rfp = RisFileAttach(fid, mode);
		return Valid();
	}
	int Stat(risparams_t *rpp) { return RisFileStat(rfp, rpp); }
	int Flush() { return RisFileFlush(rfp); }
	virtual int Close() { int t = RisFileClose(rfp); rfp = 0; return t; }
	
	int GetRootDir(risdir_t *rdp) { return RisFileGetRootDir(rfp, rdp); }
	int ReadDirEntry(risdir_t *dir, risdir_t *rdp, int which) {
		return RisFileReadDirEntry(rfp, dir, rdp, which);
	}
	int WriteDirEntry(risdir_t *dir, risdir_t *rdp, int which) {
		return RisFileWriteDirEntry(rfp, dir, rdp, which);
	}
	int SearchDir(risdir_t *dir, risdir_t *ret, rislabel_t tag) {
		return RisFileSearchDir(rfp, dir, ret, tag);
	}
	long Read(risdir_t *rdp, vaddr_t buffer, long size) {
		return RisFileRead(rfp, rdp, buffer, size);
	}
	long Write(risdir_t *rdp, vaddr_t buf) {
		return RisFileWrite(rfp, rdp, buf);
	}
};

#endif /* __RISFILE_HPP */
