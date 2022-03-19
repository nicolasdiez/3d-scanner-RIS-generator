#ifndef __RISTAGS_H
#define __RISTAGS_H

/* Range Image Standard (RIS)
 *
 * Created 22/07/94
 */

#include "risfile.h"
#include "tagnums.h"

#define MAXROOTDIRSIZE 100 /* max tags in root directory (arbitrary) */

/*
 * internal format for tag data
 */
typedef struct s_value {
   risdir_t	v_item;
   char		__v_buffer[8]; /* if the item's small, its here */
} value_t;

/*
 * various macros to peek into v_item
 */
#define v_type  v_item.rd_type
#define v_label v_item.rd_label
#define v_count v_item.rd_count

#define as_pointer v_item.u.rd_void_p
#define as_ascii v_item.u.rd_ascii_p
#define as_byte  v_item.u.rd_byte_p
#define as_short v_item.u.rd_short_p
#define as_float v_item.u.rd_float_p
#define as_long  v_item.u.rd_long_p
#define as_dir   v_item.u.rd_directory_p

/* large tags are not automatically loaded as a file is read */

#define LARGETAG(x) ((x).v_type != RT_ASCII && \
		     RisSizeOf(&(x).v_item) >= sizeof((x).__v_buffer))

/*
 * tags.inc defines a macro that names each tag
 * and provides a method to iterate over them.
 */

#include "tags.inc"

typedef struct s_ris {
   RISFILE	*r_rfp;
   risdir_t	r_root;

#define Y(name, type, tag) value_t name;
FORALLTHETAGS
#undef Y

} ris_t;

typedef ris_t RIS; /* for ease of typing */

/*
 * high-level things you can do with a RIS
 */

RIS *RisOpen(char *filename, int oflag);
RIS *RisCreate(char *filename, int oflag, int pmode);
int RisClose(RIS *rp);

int RisLoadSmallTags(RIS *rp);
int RisSaveAllTags(RIS *rp);

long RisReadTag(RIS *rp, rislabel_t label, risdir_t *dir,
				vaddr_t buffer, long size);

long RisRead2DData(RIS *rp, rislabel_t label, int scanline,
				   unsigned long linelength, risdir_t *dir,
				   vaddr_t buffer, long size);

#endif /* __RISTAGS_H */
