#ifndef __RIS_HPP
#define __RIS_HPP

/*
 * C++ wrapper for RIS handling
 */

/* Range Image Standard (RIS)
 *
 * Created 22/07/94
 */

extern "C" {
#include "ristags.h"
};

#include "risfile.hpp"

class Ris : public RisFile {
	
public:
	
	// Ris tags are polymorphic; either a large number of function prototypes
	// must be created for each tag (such as SetAscii, or GetLong - the
	// operator= stuff really isn't up to it) or we must allow direct access
	// to the tags. Hence tags is public, much against my better judgement.
	
	RIS *tags;

	Ris() { tags = 0; }
	~Ris() {}
	
	virtual int Valid() { return (tags ? 1 : 0); }
	
	virtual int Open(char *filename, int oflag = O_RDWR|O_BINARY) {
	   if ((tags = RisOpen(filename, oflag)) != 0)
		  rfp = tags->r_rfp; // for raw accesses
		return Valid();
	}
	virtual int Create(char *filename, int oflag = O_RDWR|O_BINARY, int pmode = 0666, s_risparams* = 0L )
	{
	   if ((tags = RisCreate(filename, oflag, pmode)) != 0)
		  rfp = tags->r_rfp;
		return Valid();
	}
	
	Ris(char *filename, int create = 0) {
		tags = 0;
		if (create)
			(void)Create(filename);
		else
			(void)Open(filename);
	}
	
	virtual int Close() { int t = RisClose(tags); tags = 0; rfp = 0; return t; }
	int LoadSmallTags() { return RisLoadSmallTags(tags); }
	int SaveAllTags() { return RisSaveAllTags(tags); }
	long ReadTag(rislabel_t label, risdir_t *dir, vaddr_t buffer, long size) {
		return RisReadTag(tags, label, dir, buffer, size);
	}
	long Read2DData(rislabel_t label, int scanline, unsigned long linelength,
				   risdir_t *dir, vaddr_t buffer, long size) {
		return RisRead2DData(tags, label, scanline, linelength,
							 dir, buffer, size);
	}
};

#endif /* __RIS_HPP */
