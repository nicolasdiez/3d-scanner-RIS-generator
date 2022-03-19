#ifndef __RISFILE_H
#define __RISFILE_H

/* Range Image Standard (RIS)
 *
 * Created 22/07/94
 */

/*
 * RIS access library
 *
 * The RIS file format is (loosely) based based on the TIFF standard.
 * The major difference is in the handling of directories which are
 * just another data type.
 *
 * There is one master (root) directory which contains directory
 * entries sorted by tag (or 'label') order.  The library provides
 * routines to access this directory.  Any of the directory entries
 * can itself point at a directory, as in any sensible file system.
 */

#include<fcntl.h>
	
typedef int buflen_t; /* the 'count' argument to read, memcpy etc. */

typedef char *vaddr_t;

/* fundamental contants */

#define RIS_SIG		"RIS"
#define MAJORVERSION	0
#define MINORVERSION	9
#define LITTLEENDIAN	('I' * 0x101)
#define BIGENDIAN		('M' * 0x101)

/* directory entry types */

#define RT_EMPTY	0
#define RT_ASCII	1
#define RT_BYTE		2
#define RT_SHORT	3
#define RT_LONG		4
#define RT_FLOAT	5
#define RT_DOUBLE	6
#define RT_VOID		7
#define RT_DIRECTORY	8
#define RT_DIRARRAY	9
/* directories are sorted; dirarrays are not */

/*
 * magic tag - instead of scanning the root directory for
 * this tag, return the root directory itself.
 */

#define ROOT_DOT_TAG	0

/* file creation / stat parameters */

typedef struct s_risparams {
	int	rp_endian;
} risparams_t;

/* fundamental types */

typedef long risoffset_t;
typedef unsigned short ristype_t;
typedef unsigned short rislabel_t;

/*
 * file header
 */
typedef struct s_risheader {
	char			rh_signature[4];  /* RIS_SIG  */
	char			rh_magic[2];      /* II or MM */
	struct risversion {
		char major;
		char minor;
	}			rh_version;
	unsigned long	rh_dircount;	/* root directory count */
	risoffset_t		rh_diroff;		/* root directory seek */
} risheader_t;


/*
 * a directory entry
 */
typedef struct s_risdir {
	rislabel_t		rd_label;	// tag number 
	ristype_t		rd_type;	// data type: RT_XXX
	unsigned long	rd_count;	// number of items
	union {
		void 		 *rd_void_p;	// char 
		char		 *rd_byte_p;
		short		 *rd_short_p;
		long		 *rd_long_p;
		float		 *rd_float_p;
		double		 *rd_double_p;
		struct s_risdir	 *rd_directory_p;
		risoffset_t	rd_offset_l;	/* seek offset for data */
	} u;
} risdir_t;

#define rd_ascii_p rd_byte_p
#define rd_empty_p rd_byte_p

/* risfile accesses use only this value */

#define rd_offset u.rd_offset_l

/*
 * external view of a RIS file is a pointer to struct s_risfile
 */
typedef struct s_risfile {
	int		rf_fid;
	int		rf_flags;
	risheader_t	rf_header;	/* in machine-native order */
} risfile_t;

typedef risfile_t RISFILE;	/* just for obviousness */

/* values for rf_flags */

#define RF_NEEDSWAP 1

/*
 * things you can do with a RISFILE
 */
int RisNativeEndian(void);
unsigned long RisSizeOf(risdir_t *rdp);

RISFILE *RisFileCreate(char *filename, int oflag, int pmode, risparams_t *rpp);
RISFILE *RisFileOpen(char *filename, int oflag);
RISFILE *RisFileAttach(int fid, int mode);
int RisFileStat(RISFILE *rfp, risparams_t *rpp);
int RisFileFlush(RISFILE *rfp);
int RisFileClose(RISFILE *rfp);

int RisFileGetRootDir(RISFILE *rfp, risdir_t *rdp);
int RisFileReadDirEntry(RISFILE *rfp, risdir_t *dir, risdir_t *rdp, int which);
int RisFileWriteDirEntry(RISFILE *rfp, risdir_t *dir, risdir_t *rdp, int which);
int RisFileSearchDir(RISFILE *rfp, risdir_t *dir, risdir_t *ret, rislabel_t tag);

long RisFileRead(RISFILE *rfp, risdir_t *rdp, vaddr_t buffer, long size);
long RisFileWrite(RISFILE *rfp, risdir_t *rdp, vaddr_t buffer);

#endif /* __RISFILE_H */
