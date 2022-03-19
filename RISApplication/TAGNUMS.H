#ifndef __TAGNUMS_H
#define __TAGNUMS_H

/* Range Image Standard (RIS)
 *
 * Created 22/07/94
 */

/*
 * RIS Tag Definitions.
 */

/*
 * Information Tags: general information on the measurement and the
 * data file.  Usually the company name, the measured part, the date
 * of digitising etc. are store here.
 */

#define TAG_IMAGEDESCRIPTION	270	/* ascii: description of image */
#define TAG_MAKE		271	/* ascii: scanner manufacturer */
#define TAG_MODEL		272	/* ascii: scanner model */
#define	TAG_PAGENUMBER		297	/* 2*short: page numbers herein */
#define TAG_SOFTWARE		305	/* ascii: creating package */
#define TAG_DATETIME		306	/* YYYY:MM:DD HH:MM:SS\0 */
#define TAG_ARTIST		315	/* ascii */
#define TAG_HOSTCOMPUTER	316	/* ascii */

/*
 * System Description Tags: describe the measurement system, the
 * computation of the measurement data and their representation.
 */

/*
 * The primary axis, if specified, is linearly interpolated.  If the primary
 * axis is not specified (or is zero), each data point contains at least 2
 * ordinate values.
 */

#define TAG_PRIMARYAXIS		2004	/* short: axis ^^l to scanlines */
#define     AX_SENSOR_X		(1<<0)
#define     AX_SENSOR_Y		(1<<1)
#define     AX_SENSOR_Z		(1<<2)
#define     AX_SENSOR_RANGE	(1<<3)
#define     AX_SENSOR_THETA	(1<<4)
#define     AX_PLATFORM_X	(1<<10)
#define     AX_PLATFORM_Y	(1<<11)
#define     AX_PLATFORM_Z	(1<<12)
#define     AX_PLATFORM_RANGE	(1<<13)
#define     AX_PLATFORM_THETA	(1<<14)

/*
 * If multiple ordinates are stored, the ordinate with the smallest
 * AX_XXXX value is stored at the lowest address in the file.
 */

/*
 * The secondary axis data is for information only; each line's start and
 * end points are stored explicitely in the data file.
 */

#define TAG_SECONDARYAXIS	2005	/* short: axis orthog. to scanlines */
#define TAG_POINTAXES		2006	/* short: axes represented by data */

/* there had better be exactly three bits set in (PRI|SEC|POINT) axes ! */

#define TAG_LINEARUNITS		2007	/* short: linear quantize */
#define     LU_MICRONS		 4
#define     LU_MILLIMETRES	 7
#define     LU_CENTIMETRES	 8

#define     LU_METRES		10
#define     LU_KILOMETRES	13

#define TAG_ANGULARUNITS	2008	/* short: angular quantize */
#define     AU_MICRORADIAN	 4
#define     AU_MILLIRADIAN	 7
#define     AU_RADIAN		10	/* 6.28 or so in a full circle */
#define     AU_SECOND		1008
#define     AU_MINUTE		1009
#define     AU_DEGREE		1010

#define TAG_INTERLINESPACING	2009	/* short: adjacent lines' delta */
#define     IS_CONSTANT		0	/* adjacent lines are parallel */
#define     IS_VARIABLE		1	/* parallel but not regular */
#define     IS_ARBITRARY	2	/* neither parallel nor regular */
#define     IS_NOTLINES		3	/* for completeness - scattered data */

#define TAG_PROBETYPE		2010	/* short: mechanical probe type */
#define     PT_SPHERE		0	/* spherical probe */
#define     PT_CYLINDER		1	/* cylindrical probe */
#define     PT_ROUNDED_CYLINDER	2	/* ... */

#define TAG_PROBERADIUS		2011	/* float: the radius of the probe */

/*
 * Attribute Tags: describe non-geometrical information
 */

#define TAG_COLOURFORMAT	3001	/* short */
#define     CF_NONE		0	/* no colour information (default) */
#define     CF_RGB		1	/* colour represented by 3 * bytes */

#define TAG_COLOURDATA		3002	/* dirarray: colour data */
#define TAG_INTENSITYFORMAT	3003	/* short: data type of intensity data */
#define TAG_INTENSITYDATA	3004	/* dirarray: intensity data */
#define TAG_ERRORFORMAT		3005	/* short: data type of error data */
#define TAG_ERRORDATA		3006	/* dirarray: to error data */
#define TAG_ORIENTFORMAT	3007	/* short: data type of orientation data */
#define TAG_ORIENTDATA		3008	/* dirarray: orientation data */

/*
 * Range Image Tags: details of the coordinate system and scanned data
 */

#define TAG_SCANFORMAT		4000	/* short */
#define SF_FIXEDLENGTH		0
#define SF_VARIABLELENGTH	1

#define TAG_SCANDATA		4001	/* the scan data, see PointAxes */
#define TAG_SCANENDSDATA	4002	/* fully-spec'd scanline endpoints */
					/* (PRI|SEC) axes ordinates */

#define TAG_INVALIDSCANDATA	4003	/* any: a sample invalid value */

#define TAG_RANGESCALEFACTOR	4007	/* float: manifest scale factor */
#define TAG_LINECOUNT		4008	/* long: number of lines in file */
#define TAG_LINELENGTH		4009	/* long: line length (if fixed) */

#define TAG_BOUNDARYPOLY	4100	/* float[] */
#define TAG_BOUNDARYVOLUME	4101	/* float[] */

#endif /* __TAGNUMS_H */
