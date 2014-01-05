/*
 * 
 *  Spectra - Tool for visual analysis of random data
 * 
 * This is a tool for visualing sets of potentially random data to look for
 * any patterns. The human eye and mind is exceptionally good at picking out
 * visible patterns, which can be used to find repetitious patterns which might
 * otherwise be difficult to find mathematically.
 * 
 *  Written by Tom Nardi (MS3FGX@gmail.com), released under the GPLv2.
 *  For more information, see: www.digifail.com
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "gd.h"

#define VERSION	"1.3"
#define APPNAME "Spectra"

static void help(void)
{
	printf("%s (v%s) by MS3FGX\n", APPNAME, VERSION);
	printf("----------------------------------------------------------------\n");
	printf("Spectra is designed to read the output from a TRNG or PRNG under\n"
	"examination and visualize it's output by plotting data as an image file.\n"
	"As the human mind easily picks up on visual patterns that might otherwise\n"
	"be difficult to detect mathematically, Spectra enables the user to make\n"
	"a quick evaluation of the data's true randomness. For example, it is easy\n"
	"for a non-random file to appear random to a tool like ENT, but the same\n"
	"data could appear obviously flawed when viewed through Spectra.\n");
	printf("\n");
	printf("For more information, see www.digifail.com\n");
	printf("\n");
	printf("Options:\n"
		"\t-o <filename>      Sets output filename, default is output.png\n"
		"\t-x <pixels>        Sets output image to be this many pixels wide\n"
		"\t-y <pixels>        Sets output image to be this many pixels high\n"
		"\n");
}

static struct option main_options[] = {
	{ "input", 1, 0, 'i' },
	{ "output",	1, 0, 'o' },
	{ "xsize", 1, 0, 'x' },
	{ "ysize", 1, 0, 'y' },
	{ "help", 0, 0, 'h' },
	{ 0, 0, 0, 0 }
};
 
int main(int argc, char *argv[])
{
	// Basic variables
	int opt, i, x, y;
	
	// Image size
	int xsize = 640;
	int ysize = 480;
	
	// File input and output
	char *infilename = NULL;
	char *outfilename = "output.png"; // Assign default filename
	FILE *outfile, *infile;
	
	// Buffer for current character from input file
	int inchar;
	
	// Array to hold occurrences
	unsigned long occurs[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	// Image Pointer
	gdImagePtr img;

	// Colors, will be allocated once image is created
	int black, white, red, orange, yellow, green, blue, aqua, pink, purple;
	
	// Variables to calculate file size
	struct stat fileStat;
	int file_num;

	while ((opt=getopt_long(argc, argv, "+o:i:x:y:h", main_options, NULL)) != EOF)
	{
		switch (opt)
		{
		case 'i':
			infilename = strdup(optarg);
			break;
		case 'o':
			outfilename = strdup(optarg);
			break;
		case 'x':
			xsize = atoi(optarg);
			if ( xsize > 3000 || xsize <= 0 )
			{
				printf("Invalid X dimension.\n");
				exit(1);
			}
			break;
		case 'y':
			ysize = atoi(optarg);
			if ( ysize > 3000 || ysize <= 0 )
			{
				printf("Invalid Y dimension.\n");
				exit(1);
			}
			break;
		case 'h':
			help();
			exit(0);
		default:
			printf("Unknown option. Use -h for help, or see README.\n");
			exit(0);
		}
	}

	// Exit if no input file specified
	if ( infilename == NULL )
	{
		printf("You must provide Spectra with an input file to process with the -i option.\n");
		exit(1);
	}
	
	// Boilerplate
	printf("%s (v%s) by MS3FGX\n", APPNAME, VERSION);
	printf("---------------------------\n");
	
	// Open output file
	printf("Opening input file: %s...", infilename);
	if ((infile = fopen(infilename,"r+")) == NULL )
	{
		printf("\n");
		printf("Error opening input file!\n");
		exit(1);
	}
	printf("OK\n");
	
	// Get file size
	printf("Analyzing input file...");
    file_num = fileno(infile);
    if (fstat(file_num,&fileStat) == -1)    
    {
		printf("\n");
		printf("Failed to calculate file size!\n");
		exit(1);
	}
	printf("OK (%lld bytes)\n",(long long) fileStat.st_size);
    
    // Make sure it's big enough
    if ((xsize * ysize) > fileStat.st_size)
    {
		printf("\n");
		printf("Error!\n");
		printf("The input file is not large enough for the given resolution.\n");
		printf("Either chose a lower resolution, or collect more sample data.\n");
		exit(1);
	}

	// Open output file
	printf("Creating output file: %s...", outfilename);
	if ((outfile = fopen(outfilename,"wb")) == NULL )
	{
		printf("\n");
		printf("Error opening output file!\n");
		exit(1);
	}
	printf("OK\n");
	
	// Create the image in memory
	img = gdImageCreate(xsize, ysize);

	// Assign colors to variables
	// Background color is whatever is assigned first (black)
	black = gdImageColorAllocate(img, 0, 0, 0);
	white = gdImageColorAllocate(img, 255, 255, 255);
	red = gdImageColorAllocate(img, 255, 0, 0);
	orange = gdImageColorAllocate(img, 255, 100, 0);
	yellow = gdImageColorAllocate(img, 255, 255, 0);
	green = gdImageColorAllocate(img, 0, 255, 0);
	blue = gdImageColorAllocate(img, 0, 0, 255);
	aqua = gdImageColorAllocate(img, 0, 255, 255);
	pink = gdImageColorAllocate(img, 255, 0, 255);
	purple = gdImageColorAllocate(img, 128, 0, 128);
	
	printf("Generating %ix%i image...",xsize, ysize);
	for ( y = 0; y <= ysize; y++ )
	{
		for ( x = 0; x <= xsize; x++ )
		{
			/*
			 * fgetc() returns values as int, not their string representations
			 * So we need to compare them to the ASCII values of the characters
			 * we are looking for. 
			 * 
			 * 48 - zero
			 * 57 - nine
			 *
			 */
			inchar = fgetc(infile);
			switch (inchar)
			{
			case 48: //Zero
				gdImageSetPixel(img, x, y, black); //Not needed, but fixes warning
				occurs[0]++;
				break;
			case 49: // One
				gdImageSetPixel(img, x, y, white);
				occurs[1]++;
				break;
			case 50: // Two
				gdImageSetPixel(img, x, y, red);
				occurs[2]++;
				break;
			case 51: // Three
				gdImageSetPixel(img, x, y, orange);
				occurs[3]++;
				break;
			case 52: // Four
				gdImageSetPixel(img, x, y, yellow);
				occurs[4]++;
				break;
			case 53: // Five
				gdImageSetPixel(img, x, y, green);
				occurs[5]++;
				break;
			case 54: // Six
				gdImageSetPixel(img, x, y, blue);
				occurs[6]++;
				break;
			case 55: // Seven
				gdImageSetPixel(img, x, y, aqua);
				occurs[7]++;
				break;
			case 56: // Eight
				gdImageSetPixel(img, x, y, pink);
				occurs[8]++;
				break;
			case 57: // Nine
				gdImageSetPixel(img, x, y, purple);
				occurs[9]++;
				break;
			case 10: // Newline
				printf("Error!\n");
				printf("Input file contains line breaks.\n");
				printf("File must be a continuous stream of ASCII numbers, see README.\n");
				exit(1);
			case -1: //EOF
				printf("Error!\n");
				printf("Spectra reached end of file before generating image.\n"
						"Please check input file, or see README.\n");
				exit(1);				
			default:
				printf("Error!\n");
				printf("Unsupported character. Please check input file, or see README.\n");
				exit(1);
			}
		}
	}
	printf("Done\n");
	 
	// Copy image from memory to output file
	gdImagePng(img, outfile);
	
	printf("\n");
	printf("Image Analysis\n");
	printf("---------------------------\n");
	printf("Occurrences out of %i:\n", (xsize * ysize));
	for ( i = 0; i <= 9; i++ )
		if ( occurs[i] > 0 )
			printf("Character: %i - %lu\n", i, occurs[i]);
	printf("\n");
 
	// Close the files
	printf("Closing files...\n");
	fclose(outfile);
	fclose(infile);
	
	// Destroy image in memory
	printf("Flushing memory...\n");
	gdImageDestroy(img);
	
	// Home free
	printf("Done.\n");
	exit(0);
}
