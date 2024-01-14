/*
*  Filename: libtsmapreader.h
*  Purpose: TSMC Traffic Signal Map Reader
*
*  Copyright (C) 2021 DENSO International America, Inc.
*
*/

// Function definitions
int tsmr_open(char * filename);
int tsmr_readnext(char * v_or_o, int * src_int, int * dest_int, char * r_or_e);
void tsmr_close();

// Some constants for the Vehicle/Overlap and Protected/Permitted values
#define TSMAP_VEHICLE 'v'
#define TSMAP_OVERLAP 'o'
#define TSMAP_PROTECTED 'r'
#define TSMAP_PERMITTED 'e'
