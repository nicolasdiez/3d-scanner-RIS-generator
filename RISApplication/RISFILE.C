/*
 * risfile.c: raw RISFILE access library
 */

/* Range Image Standard (RIS)
 *
 * Created 22/07/94
 */

#include "risfile.h"
#include "compat.h"

/*
 * one entry per fundamental type
 * RT_EMPTY .. RT_DIRARRAY
 */
static int SizeOfType[] = {
	0, 1, 1, 2, 4, 4, 8, 0, 12, 12
	};


/*
 * return the total size of the data associated with a directory entry
 */
unsigned long RisSizeOf(risdir_t *rdp)
{
	if (! rdp || rdp->rd_type >= sizeof(SizeOfType) / sizeof(SizeOfType[0]))
		return 0;
	
	return rdp->rd_count * SizeOfType[rdp->rd_type];
}


/*
 * return 'I' or 'M' depending on whether this is an Intel (little) or
 * Motorola (big) endian machine.  All bets are off for PDP11s ...
 */
int RisNativeEndian()
{
	/* the kind of code you're supposed not to write :-) */
	return (*(short *)"IM") & 255;
}


/*
 * internal byte-swapping routine for endianness conversions
 */
static int __swap(RISFILE *rfp, vaddr_t value, long size, long count)
{
	vaddr_t p, q;
	char t;
	
	if ((rfp->rf_flags & RF_NEEDSWAP) == 0 || size < 2)
		return 0;
	
	while (count--) {
		p = value;
		q = value + size - 1;
		
		while (p < q) {
			t = *p;
			*p++ = *q;
			*q-- = t;
		}
		value += size;
	}
	
	return 1;
}


/*
 * swap the fields of an array of directory entries
 */
static int swapdir(RISFILE *rfp, vaddr_t value, long count)
{
	if ((rfp->rf_flags & RF_NEEDSWAP) == 0)
		return 0;
	
	while (count--) {
		__swap(rfp, value, 2, 2L);
		__swap(rfp, value + 4, 4, 2L);
		value += 12;
	}
	
	return 1;
}


/*
 * swap the 'count' objects of type 'type' at address 'value'
 */
static int swap(RISFILE *rfp, ristype_t type, vaddr_t value, long count)
{
	if ((rfp->rf_flags & RF_NEEDSWAP) == 0)
		return 0;
	
	switch (type) {
		
	case RT_SHORT:
		return __swap(rfp, value, 2, count);
		
	case RT_LONG:
	case RT_FLOAT:
		return __swap(rfp, value, 4, count);
		
	case RT_DOUBLE:
		return __swap(rfp, value, 8, count);
		
	case RT_DIRECTORY:
	case RT_DIRARRAY:
		return swapdir(rfp, value, count);
	}
	
	return 0; /* no swapping required */
}


/*
 * comparison function for directory sorting routine;
 * zero tags are moved to the top; others are sorted.
 */
static int __compare(risdir_t *a, risdir_t *b)
{
	/* true if the tags are out of order */
	return (a->rd_label != b->rd_label &&
			(! a->rd_label || a->rd_label > b->rd_label));
}


/*
 * Shell sort; courtesy 'Algorithms' by Robert Sedgewick
 * (just when you were expecting a bubble sort ... :-)
 */
static int SortDir(risdir_t *base, int count)
{
	int i, j, h;
	risdir_t v;
	
	for (h = 1; h <= count/9; h = 3*h+1)
		;
	
	for ( ; h > 0; h /= 3) {
		for (i = h; i < count; i += h) {
			v = base[i]; j = i;
			while (j >= h && __compare(&base[j - h], &v) > 0) {
				base[j] = base[j - h];
				j -= h;
			}
			base[j] = v;
		}
	}
	
	return 0;
}


/*
 * seek to a directory entry
 */
static int SeekDir(RISFILE *rfp, risdir_t *dir, int which)
{
	long dirseek;
	
	if (which < 0 || which >= (int)dir->rd_count) {
		errno = ENOENT;
		return -1;
	}
	
	dirseek = dir->rd_offset + which * sizeof(risdir_t);
	
	if (lseek(rfp->rf_fid, dirseek, SEEK_SET) != dirseek)
		return -1;
	
	return which;
}


/*
 * fabricate a root directory entry.  The ROOT_DOT_TAG label is special;
 * it indicates that when this directory is written, the RIS header
 * should be modified.
 */
int RisFileGetRootDir(RISFILE *rfp, risdir_t *rdp)
{
	if (! rfp || ! rdp) {
		errno = EINVAL;
		return -1;
	}
	rdp->rd_label = ROOT_DOT_TAG;
	rdp->rd_type = RT_DIRECTORY;
	rdp->rd_count = rfp->rf_header.rh_dircount;
	rdp->rd_offset = rfp->rf_header.rh_diroff;
	
	return (int)rdp->rd_count;
}


/*
 * read a single entry from a directory
 */
int RisFileReadDirEntry(RISFILE *rfp, risdir_t *dir, risdir_t *rdp, int which)
{
	risdir_t buffer;
	
	if (! rfp || ! dir || ! rdp) {
		errno = EINVAL;
		return -1;
	}
	if (dir->rd_type != RT_DIRECTORY && dir->rd_type != RT_DIRARRAY) {
		errno = ENOTDIR;
		return -1;
	}
	if (SeekDir(rfp, dir, which) == which &&
		read(rfp->rf_fid, &buffer, sizeof(risdir_t)) == sizeof(risdir_t)) {
		swapdir(rfp, (vaddr_t)&buffer, 1);
	}
	else return -1;
	
	memcpy(rdp, &buffer, sizeof(risdir_t));
	return which;
}


/*
 * search a directory / dirarray for an entry with the specified tag.
 */
int RisFileSearchDir(RISFILE *rfp, risdir_t *dir, risdir_t *rdp, rislabel_t tag)
{
	int lower, slot, upper;
	
	if (! rfp || ! dir || ! rdp) {
		errno = EINVAL;
		return -1;
	}
	if (dir->rd_type != RT_DIRECTORY && dir->rd_type != RT_DIRARRAY) {
		errno = ENOTDIR;
		return -1;
	}
	
	if (dir->rd_type == RT_DIRARRAY) {
		for (slot = 0; slot < (int)dir->rd_count; slot++) {
			if (RisFileReadDirEntry(rfp, dir, rdp, slot) != slot)
				return -1;
			if (rdp->rd_label == tag)
				return slot;
		}
		errno = ENOENT;
		return -1;
	}
	
	/*
	 * the tags are ordered; a binary chop is indicated.  If
	 * several entries have the same tag, return the first
	 * one.  Courtesy Programming Pearls by Jon Bentley
	 */
	
	lower = -1;
	slot = upper = (int)dir->rd_count;
	
	while (lower + 1 != upper) {
		slot = (lower + upper) / 2;
		if (RisFileReadDirEntry(rfp, dir, rdp, slot) != slot)
			return -1;
		if (rdp->rd_label && rdp->rd_label < tag)
			lower = slot;
		else
			upper = slot;
	}
	
	if (slot != upper) /* n.b. upper may still be beyond end */
		if (RisFileReadDirEntry(rfp, dir, rdp, upper) != upper)
			return -1;
	
	if (rdp->rd_label != tag) {
		errno = ENOENT;
		return -1;
	}
	
	return upper;
}


/*
 * write a single directory entry, no questions asked.  NOTE: it is
 * possible to break the 'directories are sorted' assumption by using
 * this routine carelessly.  We could gratuitously turn directories
 * into dirarrays if this function is called, but that is probably
 * better left to our caller.
 */
int RisFileWriteDirEntry(RISFILE *rfp, risdir_t *dir, risdir_t *rdp, int which)
{
	risdir_t buffer;
	
	if (! rfp || ! dir || ! rdp) {
		errno = EINVAL;
		return -1;
	}
	if (dir->rd_type != RT_DIRECTORY && dir->rd_type != RT_DIRARRAY) {
		errno = ENOTDIR;
		return -1;
	}
	
	memcpy(&buffer, rdp, sizeof(risdir_t));
	swapdir(rfp, (vaddr_t)&buffer, 1);
	
	if (SeekDir(rfp, dir, which) == which &&
		write(rfp->rf_fid, &buffer, sizeof(risdir_t)) == sizeof(risdir_t))
		return which;
	
	return -1;
}


/*
 * read the data associated with the given directory entry into a buffer;
 * transfer at most 'size' bytes.
 */
long RisFileRead(RISFILE *rfp, risdir_t *rdp, vaddr_t buffer, long size)
{
	long count;
	
	if (! rfp || ! rdp || ! buffer) {
		errno = EINVAL;
		return -1L;
	}
	
	count = RisSizeOf(rdp);
	
	if (count > size)
		count = size;
	else
		size = count;
	
	if (count == 0)
		return 0;
	
	if (count <= 4)
		memcpy(buffer, &rdp->rd_offset, (int)count);
	
	else {
		if (lseek(rfp->rf_fid, rdp->rd_offset, SEEK_SET) != rdp->rd_offset)
			count = -1;
		else {
/*			if (sizeof(buflen_t) == sizeof(long)) {
				if (read(rfp->rf_fid, buffer, (buflen_t)count) != count)
					count = -1;
			}       */
				{
				long _count = count;
				while (_count > 0 && count > 0) {
					buflen_t bite = (_count > 0x4000) ? 0x4000 : _count;
					if (read(rfp->rf_fid, buffer, bite) != bite)
						count = -1;
					_count -= bite;
					buffer += bite;
				}
			}
		}
	}
	
	if (count > 0)
		swap(rfp, rdp->rd_label, buffer, (rdp->rd_count * count) / size);
	
	return count;
}


/*
 * Write the data associated with a directory entry into the output file;
 * get the data from 'buffer'.  If the directory entry specifies an
 * offset, write the data at that offset, otherwise append it to the end
 * of the file.
 */
long RisFileWrite(RISFILE *rfp, risdir_t *rdp, vaddr_t buffer)
{
	long count;
	
	if (! rfp || ! rdp || ! buffer) {
		errno = EINVAL;
		return -1L;
	}
	
	count = RisSizeOf(rdp);
	
	if (count == 0)
		return 0;
	
	swap(rfp, rdp->rd_label, buffer, rdp->rd_count);
	
	if (count <= 4) /* store small amounts in the offset */
		memcpy(&rdp->rd_offset, buffer, (buflen_t)count);
	
	else {
		if (rdp->rd_offset == 0)
			rdp->rd_offset = lseek(rfp->rf_fid, 0L, SEEK_END);
		
		if (rdp->rd_type == RT_DIRECTORY || rdp->rd_type == RT_DIRARRAY) {
			if (rdp->rd_label == ROOT_DOT_TAG) {
				rfp->rf_header.rh_diroff = rdp->rd_offset;
				rfp->rf_header.rh_dircount = rdp->rd_count;
			}
			if (rdp->rd_type == RT_DIRECTORY)
				SortDir((risdir_t *)buffer, (int)rdp->rd_count);
		}
		
		if (lseek(rfp->rf_fid, rdp->rd_offset, SEEK_SET) != rdp->rd_offset)
			count = -1;
		else {
/*			if (sizeof(buflen_t) == sizeof(long)) {
				if (write(rfp->rf_fid, buffer, (buflen_t)count) != count)
					count = -1;
			}*/
			/*else*/ {
				long _count = count;
				while (_count > 0 && count > 0) {
					buflen_t bite = (_count > 0x4000) ? 0x4000 : _count;
					if (write(rfp->rf_fid, buffer, bite) != bite)
						count = -1;
					_count -= bite;
					buffer += bite;
				}
			}
		}
	}
	
	/*
	 * reswap caller's data to prevent surprises; this is an efficiency
	 * hit but the alternative is to allocate/free a temporary buffer
	 * with all the potential nonsense that entails.  We must do this
	 * even if the above write fails.
	 */
	swap(rfp, rdp->rd_label, buffer, rdp->rd_count);
	return count;
}


/*
 * attach an open file descriptor to a RIS structure
 */
RISFILE *RisFileAttach(int fid, int flags)
{
	RISFILE *rfp = (RISFILE *)malloc(sizeof(RISFILE));
	
	if (! rfp) {
		errno = ENOMEM;
		return (RISFILE *)0;
	}
	rfp->rf_fid = fid;
	rfp->rf_flags = flags & ~RF_NEEDSWAP;
	
	if (read(rfp->rf_fid, &rfp->rf_header, sizeof(risheader_t)) !=
		sizeof(risheader_t))
		goto error;
	
	if (*(long *)rfp->rf_header.rh_signature != *(long *)RIS_SIG ||
		(*(short *)rfp->rf_header.rh_magic != LITTLEENDIAN &&
		 *(short *)rfp->rf_header.rh_magic != BIGENDIAN)) {
		errno = EINVAL;
		goto error;
	}
	
	/* is the prevailing endianess the same as that of the file ? */
	
	if (rfp->rf_header.rh_magic[0] != RisNativeEndian())
		rfp->rf_flags |= RF_NEEDSWAP;
	
	__swap(rfp, (vaddr_t)&rfp->rf_header.rh_dircount, 4, 2L);
	
	if (rfp->rf_header.rh_version.major != MAJORVERSION) { /* but not minor */
		errno = EACCES;
		goto error;
	}
	
	return rfp;
	
 error:
	free((char *)rfp);
	return (RISFILE *)0;
}


/*
 * open the specified file in the given mode, and attach it
 * to a RIS structure.
 */
RISFILE *RisFileOpen(char *filename, int oflag)
{
	int fid;
	int flags;
	RISFILE *retval;
	
	if ((fid = open(filename, oflag)) < 0)
		return (RISFILE *)0;
	
	flags = 0;
	if ((retval = RisFileAttach(fid, flags)) == (RISFILE *)0)
		close(fid);
	
	return retval;
}


/*
 * flush a RIS file by writing out its header block, if necessary
 */
int RisFileFlush(RISFILE *rfp)
{
	risheader_t buffer;
	
	memcpy(&buffer, &rfp->rf_header, sizeof(risheader_t));
	__swap(rfp, (vaddr_t)&buffer.rh_dircount, 4, 2L);
	
	if (lseek(rfp->rf_fid, 0L, SEEK_SET) != 0L ||
		write(rfp->rf_fid, &buffer, sizeof(risheader_t)) !=
		sizeof(risheader_t))
		return -1;
	
	return 0;
}


/*
 * close a RIS file
 */
int RisFileClose(RISFILE *rfp)
{
	int retval;
	
	if (! rfp) {
		errno = EINVAL;
		return -1;
	}
	
	(void)RisFileFlush(rfp);
	
	retval = close(rfp->rf_fid);
	free((char *)rfp);
	
	return retval;
}


/*
 * Create a new RIS file from scratch
 */
RISFILE *RisFileCreate(char *filename, int oflag, int pmode, risparams_t *rpp)
{
	RISFILE *rfp;
	
	if (! filename || ! rpp ||
		(rpp->rp_endian != 'I' && rpp->rp_endian != 'M')) {
		errno = EINVAL;
		return (RISFILE *)0;
	}

	rfp = (RISFILE *)malloc(sizeof(RISFILE));
	
	if (! rfp) {
		errno = ENOMEM;
		return (RISFILE *)0;
	}
	
	if ((rfp->rf_fid = open(filename, oflag|O_CREAT|O_TRUNC, pmode)) < 0)
		return (RISFILE *)0;
	
	rfp->rf_flags = 0;
	// ROGER
	//*(long *)rfp->rf_header.rh_signature = *(long *)RIS_SIG;
	// MIKE MODIFICATION
	strcpy(rfp->rf_header.rh_signature, RIS_SIG);
	rfp->rf_header.rh_magic[0] = rfp->rf_header.rh_magic[1] = rpp->rp_endian;
	rfp->rf_header.rh_version.major = MAJORVERSION;
	rfp->rf_header.rh_version.minor = MINORVERSION;
	rfp->rf_header.rh_dircount = 0;
	rfp->rf_header.rh_diroff = 0;
	
	if (RisFileFlush(rfp) < 0) {
		close(rfp->rf_fid);
		free((char *)rfp);
		return (RISFILE *)0;
	}
	
	return rfp;
}


/*
 * return the parameters used to create the file
 */
int RisFileStat(RISFILE *rfp, risparams_t *rpp)
{
	if (! rfp || ! rpp) {
		errno = EINVAL;
		return -1;
	}
	
	rpp->rp_endian = rfp->rf_header.rh_magic[0];
	return 0;
}

/* end */
