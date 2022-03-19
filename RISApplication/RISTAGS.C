/*
 * ristags.c
 * 'cooked' RIS operations, that have knowledge of individual
 * tags and their types.
 */

/* Range Image Standard (RIS)
 *
 * Created 22/07/94
 */

#include "ristags.h"
#include "compat.h"

#include <string.h>
/*
 * simple utility, by way of an example really ...
 */
long RisReadTag(RIS *rp, rislabel_t label, risdir_t *dir,
				vaddr_t buffer, long size)
{
	if (RisFileSearchDir(rp->r_rfp, &rp->r_root, dir, label) < 0)
		return -1;
	
	return RisFileRead(rp->r_rfp, dir, buffer, size);
}


/*
 * read 2-d data.  Either the tag refers to data, in which case
 * 'linenumber' and 'linelength' are used to select some part of the
 * data, or the tag refers to a dirarray, in which case 'linenumber' is
 * used to select an entry in the dirarray (and the entire entry is
 * returned.)
 */
long RisRead2DData(RIS *rp, rislabel_t label, int linenumber,
				   unsigned long linelength, risdir_t *dir,
				   vaddr_t buffer, long size)
{
	if (! rp || ! buffer || ! dir || linenumber < 0 || size < 0)
		return -1;
	
	if (RisFileSearchDir(rp->r_rfp, &rp->r_root, dir, label) < 0)
		return -1;
	
	if (dir->rd_type == RT_DIRARRAY) {
		if (RisFileReadDirEntry(rp->r_rfp, dir, dir, linenumber) != linenumber)
			return -1;
	}
	else {
		risdir_t tempdir;
		tempdir.rd_type = dir->rd_type;
		tempdir.rd_count = linenumber * linelength;
		if (dir->rd_count < tempdir.rd_count)
			return -1; /* beyond end */
		dir->rd_count -= tempdir.rd_count;
		dir->rd_offset += RisSizeOf(&tempdir);
		if (dir->rd_count > linelength)
			dir->rd_count = linelength;
	}
	return RisFileRead(rp->r_rfp, dir, buffer, size);
}


static int __LoadOne(RIS *rp, value_t *where)
{
	risdir_t dir;
	char *buffer;
	
	where->v_type = RT_EMPTY;
	where->v_count = 0;
	
	if (RisFileSearchDir(rp->r_rfp, &rp->r_root, &dir, where->v_label) < 0)
		return 0;
	
	where->v_type = dir.rd_type;
	where->v_count = dir.rd_count;
	
	buffer = 0;
	switch (dir.rd_type) {
		/* n.b don't autoload directories */
		
	case RT_ASCII:
		
		/* autoload strings always */
		
		if (dir.rd_count + 1 > sizeof(where->__v_buffer))
		{
			//Original
			//where->as_ascii = malloc((buflen_t)dir.rd_count + 1);		
			
			//Modified by JORGE
			where->as_ascii = (char*)malloc((buflen_t)dir.rd_count + 1);
		}
		
		where->as_ascii[dir.rd_count] = 0;
		buffer = where->as_ascii;
		break;
		
	case RT_BYTE:
	case RT_SHORT:
	case RT_LONG:
	case RT_FLOAT:
	case RT_DOUBLE:
		
		/* otherwise only autoload items smaller than our buffer size ... */
		
		if (RisSizeOf(&dir) < sizeof(where->__v_buffer))
			buffer = where->__v_buffer;
		
		break;
	}
	
	if (! buffer)
		return 0;
	
	if (RisFileRead(rp->r_rfp, &dir, buffer, RisSizeOf(&dir)) < 0)
		return 0;
	
	return 1;
}


static int __SaveOne(RIS *rp, value_t *where, risdir_t **r)
{
	risdir_t *p = *r;
	
	if (! where->as_pointer)
		return -1;
	
	p->rd_type = where->v_type;
	p->rd_label = where->v_label;
	p->rd_offset = 0;  /* append to file */
	
	p->rd_count = (where->v_type == RT_ASCII && where->v_count == 0) ?
		strlen(( char* )(where->as_ascii)) : where->v_count;
	
	if (RisSizeOf(p) == 0)
		return 0;


#pragma warning( disable : 4018 )
	// Modified by JORGE
	if (RisFileWrite(rp->r_rfp, p, (vaddr_t)(where->as_pointer)) != RisSizeOf(p))
		return -1;
#pragma warning( default : 4018 )
	
	*r = p + 1;
	return 0;
}


/*
 * read all the tags declared in RIS structure.  Tag data that
 * will fit into __v_buffer[xx] is autoloaded, as are all ascii
 * strings.
 */
int RisLoadSmallTags(RIS *rp)
{
	int count = 0;
#define Y(name, type, tag) count += __LoadOne(rp, &rp->name);
	FORALLTHETAGS
#undef Y
		return count;
}


/*
 * save all the tags declared in RIS structure.
 */
int RisSaveAllTags(RIS *rp)
{
	risdir_t *rootdata, *slot;
	int retval;
	
	rootdata = (risdir_t *)malloc(MAXROOTDIRSIZE * sizeof(risdir_t));
	slot = rootdata;
	
#define Y(name, type, tag) __SaveOne(rp, &rp->name, &slot);
	FORALLTHETAGS
#undef Y
		
		rp->r_root.rd_count = slot - rootdata;
	retval = RisFileWrite(rp->r_rfp, &rp->r_root, (vaddr_t)rootdata);
	
	free((char *)rootdata);
	return retval;
}


/*
 * create a null RIS structure and set the default tag types.
 */
static RIS *MakeDefaultRis(void)
{
	RIS *rp = (RIS *)malloc(sizeof(RIS));
	
	if (! rp)
		return (RIS *)0;
	
	memset(rp, 0, sizeof(RIS));
	
#define Y(name, type, tag) rp->name.v_type = type; rp->name.v_label = tag; \
	rp->name.v_count = 0;
	FORALLTHETAGS
#undef Y
/* two separate lumps because of incompetent preprocessors */
#define Y(name, type, tag) rp->name.as_pointer = &rp->name.__v_buffer[0];
	FORALLTHETAGS
#undef Y		
		return rp;
}


/*
 * open a pre-existing RIS file and read all the tags declared in
 * the RIS structure.
 */
RIS *RisOpen(char *filename, int oflag)
{
	RIS *rp = MakeDefaultRis();
	
	if (! rp)
		return (RIS *)0;
	
	rp->r_rfp = RisFileOpen(filename, oflag);
	if (RisFileGetRootDir(rp->r_rfp, &rp->r_root) < 0) {
		RisFileClose(rp->r_rfp);
		free((char *)rp);
		return (RIS *)0;
	}
	RisLoadSmallTags(rp);
	
	return rp;
}


/*
 * create a new (null) RIS file
 */
RIS *RisCreate(char *filename, int oflag, int pmode)
{
	risparams_t p;
	RIS *rp = MakeDefaultRis();
	
	if (! rp)
		return (RIS *)0;
	
	p.rp_endian = RisNativeEndian();
	rp->r_rfp = RisFileCreate(filename, oflag, pmode, &p);
	
	if (RisFileGetRootDir(rp->r_rfp, &rp->r_root) < 0) {
		RisFileClose(rp->r_rfp);
		free((char *)rp);
		return (RIS *)0;
	}
	
	return rp;
}


/*
 * close an open RIS file.  NOTE: this does NOT automatically
 * call RisSaveAllTags !!!
 */
int RisClose(RIS *rp)
{
	int retval;
	
#define Y(name, type, tag) if (type == RT_ASCII && \
					   rp->name.v_count + 1 > sizeof(rp->name.__v_buffer)) \
					   free(rp->name.as_ascii);
	FORALLTHETAGS
#undef Y
		
	retval = RisFileClose(rp->r_rfp);
	free((char *)rp);
	return retval;
}


/* end */
