

/* Range Image Standard (RIS)
 *
 * Created 22/07/94
 */


#include <errno.h>  /* ... */
#include <fcntl.h>  /* O_RDWR etc */
#include <malloc.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <io.h>    /* lseek etc */

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif
