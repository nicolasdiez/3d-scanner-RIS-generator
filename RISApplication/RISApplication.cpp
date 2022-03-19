// Description: Create a RIS file

#include "stdafx.h"
#include "RISFILE.H"
#include "RISFILE.C"
#include "RISTAGS.H"
#include "RISTAGS.C"

#include "RISFILE.HPP"
#include "RIS.HPP"

#include "math.h"

#include "FreeImage.h"

void createRis_C_Syntax();
void createRisFile_C_Syntax();

const int imageWidth = 80;
const int imageHeight = 80;
float depthBuffer[imageWidth*imageHeight];


const float NULO = -3.37e+38;	// "Invalid Data" code
float z_tmp[imageWidth*imageHeight];
float data_out[5201][5201];

int _tmain(int argc, _TCHAR* argv[])
{
	
	// Load TIFF
	FIBITMAP* inputTIF = FreeImage_Load( FIF_TIFF, "..\\data\\clouds.tif", 0);
	
	float*  pixels;
	float height = 0.0f;
	float outputValue  = 0.0f;
	
	for(int y = 0; y < imageHeight; y++)
	{
		pixels = (float *)FreeImage_GetScanLine(inputTIF, y);
		for(int x = 0; x < imageWidth; x++)
		{
			// Get height from image
			height = pixels[x];
			//printf( "\nZ: %f", height );
			depthBuffer[x*imageWidth+y] = (float)height*1.0f;
		}
	}



	// Fill z_tmp[i][j] with NULL data
	for (int i=0; i< 5201; i++){
		for (int j=0; j< 5201; j++){
			z_tmp[i*imageWidth+j] = NULO;
		}
	}


	// Copy to a 5201 x 5201 tile and scale back. (0, 1) RAW range must be mapped to (-250, 250) RIS range
	for (int i=0; i< 5201; i++){
		for (int j=0; j< 5201; j++){
			z_tmp[i*imageWidth+j] = 500 * depthBuffer[i*imageWidth+j] - 250;
		}
	}

	/*
	// rotate and translate before exporting to "*.ris"
	for (int i=0; i< 5201; i++){
		for (int j=0; j< 5201; j++){
			if (z_tmp[5200 - i][j] == -250 || z_tmp[5200 - i][j] > 250 || z_tmp[5200 - i][j] < -250){  
				data_out[j][i] = NULO;												// "0" or "out of range values" must be mapped into NULO
			}
			else{
				data_out[j][i] = z_tmp[5200 -i][j];
			}
		}
	}
	*/
	//depthBuffer = z_tmp;
/*
	// Generate depth data
	float angle = 0.0f;
	for (int i=0;i<5201*5201;i++)
	{
		depthBuffer[i] = sin(angle);
		angle += 0.00001f;
	}
*/
	Ris* destFile = new Ris( "test_3.ris", 1 );

	if ( !destFile->Valid() )
	{
		printf( "ERROR: Error creating file" );
		return -1;
	}

	RIS* tags = destFile->tags;
	
	*tags->r_linecount.as_long = imageHeight;
	tags->r_linecount.v_count = 1;

	*tags->r_linelength.as_long = imageWidth;
	tags->r_linelength.v_count = 1;

	*tags->r_primaryaxis .as_short = AX_SENSOR_X;
	tags->r_primaryaxis.v_count = 1;

	*tags->r_secondaryaxis.as_short = AX_SENSOR_Y;
	tags->r_secondaryaxis.v_count = 1;

	*tags->r_pointaxes.as_short = AX_SENSOR_Z;
	tags->r_pointaxes.v_count = 1;

	*tags->r_scanformat.as_short = SF_FIXEDLENGTH;
	tags->r_scanformat.v_count = 1;

	*tags->r_interlinespacing.as_short = IS_CONSTANT;
	tags->r_interlinespacing.v_count = 1;

	*tags->r_linearunits.as_short = LU_MILLIMETRES;
	tags->r_linearunits.v_count = 1;
/*
	*tags->r_primaryaxis .as_short = AX_PLATFORM_X;
	tags->r_primaryaxis.v_count = 1;

	*tags->r_secondaryaxis.as_short = AX_PLATFORM_Y;
	tags->r_secondaryaxis.v_count = 1;
	
	*tags->r_pointaxes.as_short = AX_PLATFORM_Z;
	tags->r_pointaxes.v_count = 1;
*/
	// Write not-a-number bit pattern as a long (although it is
	// really a float) since some machines will trow an exception
	// when they access a NAN floating point number
/*
	*tags->r_invalidscandata.as_long = 0; // NAN
	tags->r_invalidscandata.v_type = RT_FLOAT;
	tags->r_invalidscandata.v_count = 1;
*/
	// Lengths of strings are inferred
/*	
	tags->r_imagedescription.as_ascii = "RisTile_2m x 2m";
	tags->r_software.as_ascii = "Lucida Scanner";
	tags->r_datetime.as_ascii = "2013:04:15 12:00:00";

	tags->r_artist.as_ascii = "Jorge Cano";
	tags->r_make.as_ascii = "Test";
*/
	// Copy depth data

	tags->r_scandata.v_type = RT_FLOAT;
	tags->r_scandata.as_float = z_tmp;
	tags->r_scandata.v_count = *tags->r_linecount.as_long * *tags->r_linelength.as_long;

	// A few more bits and pieces
	*tags->r_rangescalefactor.as_float = 1.0f;
	tags->r_rangescalefactor.v_count = 1;


	/*

	// Fabricate fully-qualified endpoint coordinates
	// (x, y, z) for start and end of each line

	tags->r_scanendsdata.v_count = 6* *tags->r_linecount.as_long;
	float* t = (float*) malloc( sizeof(float) * tags->r_scanendsdata.v_count );
	tags->r_scanendsdata.v_type = RT_FLOAT;
	tags->r_scanendsdata.as_float = t;

	*/

	destFile->SaveAllTags();

	//free( (char*)tags->r_scanendsdata.as_float );

	// The following call will append all the above tags
	// to the destination file, and write the master directory entry.

	

	// The default types for most of these variables are correct
	// alreadyt so there is no need to set the v_type fields.

	// Example ussage using the 'high-level' interface and C syntax
	//createRis_C_Syntax();

	// Example ussage using the 'low-level' interface and C syntax
	//createRisFile_C_Syntax();

	return 0;
}



void createRis_C_Syntax()
{
	// Set file parameters (are taken from documentation)
	int oFlag = O_RDWR;
	int pMode = 0666;

	// Create Ris
	RIS* tRis = RisCreate( "test_1.RIS", oFlag, pMode );

	if ( tRis == NULL )
	{
		printf( "ERROR: Error creating file" );
		return;
	}

	// Save tags
	RisSaveAllTags( tRis );

	// Writes file to disk
	int success = RisClose( tRis );

	// Security check
	if ( success == -1 )
		printf( "ERROR: Error flushing file" );
}

void createRisFile_C_Syntax()
{
	// Set file parameters (are taken from documentation)
	int oFlag = O_RDWR;
	int pMode = 0666;

	// Files have an 'endianness' encoded into them, that is:
	// Intel ('I') or MOTOROLA ('M') machines
	risparams_t risParams;
	risParams.rp_endian = 'I';

	// Create file
	RISFILE* tRisFile = RisFileCreate( "test_2.RIS", oFlag, pMode, &risParams );

	// Flush to disk
	int success = RisFileFlush( tRisFile );
}


