
/* Range Image Standard (RIS)
 *
 * Created 22/07/94
 */

/*
 * The FORALLTHETAGS macro defines the name and (initial) type of each
 * tag in the RIS file.  The format should be obvious, but for completeness:
 * the first macro argument is the 'C' variable name; the second is the
 * ris-tag type (from risfile.h) and the third is the tag number (from
 * tagnums.h).  For sanity the first and the third names should be
 * related.  This macro is used to generate and access the RIS structure.
 * The order is not important; if a corresponding tag exists in the data
 * file it will be loaded; otherwise the tag type will become RT_EMPTY.
 */

#define FORALLTHETAGS \
Y(r_artist, RT_ASCII, TAG_ARTIST)\
Y(r_datetime, RT_ASCII, TAG_DATETIME)\
Y(r_hostcomputer, RT_ASCII, TAG_HOSTCOMPUTER)\
Y(r_imagedescription, RT_ASCII, TAG_IMAGEDESCRIPTION)\
Y(r_make, RT_ASCII, TAG_MAKE)\
Y(r_model, RT_ASCII, TAG_MODEL)\
Y(r_software, RT_ASCII, TAG_SOFTWARE)\
Y(r_pagenumber, RT_SHORT, TAG_PAGENUMBER)\
Y(r_primaryaxis, RT_SHORT, TAG_PRIMARYAXIS)\
Y(r_secondaryaxis, RT_SHORT, TAG_SECONDARYAXIS)\
Y(r_pointaxes, RT_SHORT, TAG_POINTAXES)\
Y(r_linearunits, RT_SHORT, TAG_LINEARUNITS)\
Y(r_angularunits, RT_SHORT, TAG_ANGULARUNITS)\
Y(r_interlinespacing, RT_SHORT, TAG_INTERLINESPACING)\
Y(r_probetype, RT_SHORT, TAG_PROBETYPE)\
Y(r_proberadius, RT_FLOAT, TAG_PROBERADIUS)\
Y(r_colourformat, RT_SHORT, TAG_COLOURFORMAT)\
Y(r_intensityformat, RT_SHORT, TAG_INTENSITYFORMAT)\
Y(r_errorformat, RT_SHORT, TAG_ERRORFORMAT)\
Y(r_orientformat, RT_SHORT, TAG_ORIENTFORMAT)\
Y(r_scanformat, RT_SHORT, TAG_SCANFORMAT)\
Y(r_colourdata, RT_DIRARRAY, TAG_COLOURDATA)\
Y(r_intensitydata, RT_DIRARRAY, TAG_INTENSITYDATA)\
Y(r_errordata, RT_DIRARRAY, TAG_ERRORDATA)\
Y(r_orientdata, RT_DIRARRAY, TAG_ORIENTDATA)\
Y(r_scandata, RT_DIRARRAY, TAG_SCANDATA)\
Y(r_scanendsdata, RT_DIRARRAY, TAG_SCANENDSDATA)\
Y(r_rangescalefactor, RT_FLOAT, TAG_RANGESCALEFACTOR)\
Y(r_linecount, RT_LONG, TAG_LINECOUNT)\
Y(r_linelength, RT_LONG, TAG_LINELENGTH)\
Y(r_invalidscandata, RT_LONG, TAG_INVALIDSCANDATA)\
Y(r_boundarypoly, RT_FLOAT, TAG_BOUNDARYPOLY)\
Y(r_boundaryvolume, RT_FLOAT, TAG_BOUNDARYVOLUME)
