#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define ROWS	480
#define COLS	640
#define PI 3.14159265358979323846

#define sqr(x)	((x)*(x))




void clear( unsigned char image[][COLS] );
void header( int row, int col, unsigned char head[32] );

int main( int argc, char** argv )
{
	
	int				i,j, sgmmax;
	int 			x,y;
	int 			temp;
	// localmax: number in the three bucket of voting array corrsponding to three local maxima
	// index[3][2]: used for store rho and theta
	int				dedx, dedy, sgm, localmax[3] = {0, 0, 0}, index[3][2] = { 0, 0, 0, 0, 0, 0 }, ma;
	// voting; voting array
	int				sgm_threshold, hough_threshold, voting[180][400];
	FILE*			fp;
	unsigned char	image[ROWS][COLS], simage[ROWS][COLS], sgmimage[ROWS][COLS], bimage[ROWS][COLS], head[32],
					vimage[180][400];
	char			filename[50], ifilename[50], ch;
	float           theta, rho;
	
	
	
	clear(simage);
	strcpy ( ifilename, "image");
	memset ( voting, 0, sizeof(int) * 180 * 400 );
	header(ROWS, COLS, head);
	
	sgm_threshold = 160;
	hough_threshold = 150;

	/* Read in the image */
	if (( fp = fopen( strcat(ifilename, ".raw"), "rb" )) == NULL )
	{
	  fprintf( stderr, "error: couldn't open %s\n", ifilename );
	  exit( 1 );
	}			
	for ( i = 0; i < ROWS ; i++ )
	  if ( fread( image[i], sizeof(char), COLS, fp ) != COLS )
	  {
		fprintf( stderr, "error: couldn't read enough stuff\n" );
		exit( 1 );
	  }
	fclose( fp );





    /* Compute SGM */
	for (i=1; i<ROWS-1; i++)
	{
	  for (j=1; j<COLS-1; j++)
	  {
	    dedx = abs((2*image[i][j+1])+image[i+1][j+1]+image[i-1][j+1]
					   - ((2*image[i][j-1])+image[i-1][j-1]+image[i+1][j-1]));	   
		dedy = abs((2*image[i+1][j])+image[i+1][j+1]+image[i+1][j-1]
					   - ((2*image[i-1][j])+image[i-1][j+1]+image[i-1][j-1]));
		sgm = dedx*dedx + dedy*dedy;
		
		if (sgmmax <= sgm)
		{
		  sgmmax = sgm;
		}		
	  }
	}
	clear(sgmimage);
	for (i=1; i<ROWS-1; i++)
	{
	  for (j=1; j<COLS-1; j++)
	  {
		dedx = abs((2*image[i][j+1])+image[i+1][j+1]+image[i-1][j+1]
					   - ((2*image[i][j-1])+image[i-1][j-1]+image[i+1][j-1]));	   
		dedy = abs((2*image[i+1][j])+image[i+1][j+1]+image[i+1][j-1]
					   - ((2*image[i-1][j])+image[i-1][j+1]+image[i-1][j-1]));
	    sgm = dedx*dedx + dedy*dedy;
		
		sgmimage[i][j] = (sgm * 255) / sgmmax;
	  }
	}
	
	for (i=0; i<ROWS; i++)
	{
	  sgmimage[i][0] = sgmimage[i][COLS-1] = 0;
	}
	
	for (j=0; j<COLS; j++)
	{
	  sgmimage[0][j] = sgmimage[ROWS-1][j] = 0;
	}


	

	/* build up voting array */
	for (i=0; i<180; i++)
	{
	  theta = -(i * PI) / 180;
	  for (y=0; y<ROWS; y++)
	  {
		for (x=0; x<COLS; x++)
		{
		  if (sgmimage[y][x] >= sgm_threshold)
		  {
			rho = (y * cos(theta)) - (x * sin(theta));
			voting[i][abs((int)rho)] += 1;		
		  }
		}
	  }
	}
	for (j=0; j<400; j++)
	{
	  for (i=0; i<180; i++)
	  {
		if (voting[i][j] > hough_threshold)
		{
		  if (localmax[2] < voting[i][j])
		  {
		    localmax[2] = voting[i][j];
		    if (localmax[1] < localmax[2])
		    {
			  temp = localmax[1];
			  localmax[1] = localmax[2];
			  localmax[2] = temp;
			  if (localmax[0] < localmax[1])
		      {
			    temp = localmax[0];
			    localmax[0] = localmax[1];
			    localmax[1] = temp;
		      }
		    }
		  }
		}
	  }
	}	 
	for (i=0; i<180; i++)
	{
	  for (j=0; j<400; j++)
	  {
		vimage[i][j] = (voting[i][j] * 255) / localmax[2] ;
	  }
	}

	

	/* Save SGM to an image */
	strcpy(filename, "image");
	if (!(fp = fopen(strcat(filename, "-sgm.ras"), "wb")))
	{
		fprintf(stderr, "error: could not open %s\n", filename);
		exit(1);
	}
	fwrite(head, 4, 8, fp);
	for (i = 0; i < ROWS; i++)
		fwrite(sgmimage[i], sizeof(char), COLS, fp);
	fclose(fp);


	/* Compute the binary image */
	clear(bimage);
	for (i=0; i<ROWS; i++)
	{
	  for (j=0; j<COLS; j++)
	  {
		if (sgmimage[i][j] <= sgm_threshold)
		{
		  bimage[i][j] = 0;
		}
		else
		{
		  bimage[i][j] = 255;
		}
	  }
	}





	/* Save the thresholded SGM to an image */
	strcpy(filename, "image");
	if (!(fp = fopen(strcat(filename, "-binary.ras"), "wb")))
	{
		fprintf(stderr, "error: could not open %s\n", filename);
		exit(1);
	}
	fwrite(head, 4, 8, fp);
	for (i = 0; i < ROWS; i++)
		fwrite(bimage[i], sizeof(char), COLS, fp);
	fclose(fp);




	/* Save original voting array to an image */
	strcpy(filename, "image");
	header(180, 400, head);
	if (!(fp = fopen(strcat(filename, "-voting_array.ras"), "wb")))
	{
		fprintf(stderr, "error: could not open %s\n", filename);
		exit(1);
	}
	fwrite(head, 4, 8, fp);
	for (i = 0; i < 180; i++)
		fwrite(vimage[i], sizeof(char), 400, fp);
	fclose(fp);


	/* Threshold the voting array */
	for (i=0; i<180; i++)
	{
	  for (j=0; j<400; j++)
	  {
		if (vimage[i][j] <= hough_threshold)
		{
		  vimage[i][j] = 0;
		}
		else
		{
		  vimage[i][j] = 255;
		}
	  }
	}
	







	/* Write the thresholded voting array to a new image */
	strcpy(filename, "image");
	header(180, 400, head);
	if (!(fp = fopen(strcat(filename, "-voting_array_t.ras"), "wb")))
	{
		fprintf(stderr, "error: could not open %s\n", filename);
		exit(1);
	}
	fwrite(head, 4, 8, fp);

	for (i = 0; i < 180; i++)
		fwrite(vimage[i], sizeof(char), 400, fp);
	fclose(fp);


	/* Local max indicies */
	for (i=0; i<180; i++)
	{
	  for (j=0; j<400; j++)
	  {
		if (voting[i][j] == localmax[0])
		{
		  index[0][0] = i;
		  index[0][1] = j;
		  theta = -(index[0][0] * PI) / 180;
		  temp = (int)((x*sin(theta))-(y*cos(theta)));
	      if (temp > 0)
		  {
			ma = 0;
		  }
		}
		if (voting[i][j] == localmax[1])
		{
		  index[1][0] = i;
		  index[1][1] = j;
		  theta = -(index[1][0] * PI) / 180;
		  temp = (int)((x*sin(theta))-(y*cos(theta)));
	      if (temp > 0)
		  {
			ma = 1;
		  }
		}
		if (voting[i][j] == localmax[2])
		{
		  index[2][0] = i;
		  index[2][1] = j;
		  theta = -(index[2][0] * PI) / 180;
		  temp = (int)((x*sin(theta))-(y*cos(theta)));
	      if (temp > 0)
		  {
			ma = 2;
		  }
		}
	  }
	}


	printf("Hough threshold: %d\n", hough_threshold);
	printf("%d %d %d\n%d %d %d\n%d %d %d\n", index[0][0], index[0][1], localmax[0],
											 index[1][0], index[1][1], localmax[1],
											 index[2][0], index[2][1], localmax[2]);
											 
	
	/* Reconstruct an image from the voting array */
	clear(simage);
	for (i=0; i<3; i++)
	{
	  theta = -(index[i][0] * PI) / 180;
	  for (y=0; y<ROWS; y++)
	  {
	    for (x=0; x<COLS; x++)
	    {
		  temp = (int)((x*sin(theta))-(y*cos(theta)));
	      if (i == ma)
		  {
			temp = (int)((x*sin(theta))-(y*cos(theta))-index[i][1]);
		  }
		  else
		  {
			temp = (int)((x*sin(theta))-(y*cos(theta))+index[i][1]);
		  }
		  if (temp == 0)
		  {
		    simage[y][x] = 255;
		  } 
		}
	  }
	}



	/* Write the reconstructed figure to an image */
	strcpy(filename, "image");
	header(ROWS, COLS, head);
	if (!(fp = fopen(strcat(filename, "-reconstructed_image.ras"), "wb")))
	{
		fprintf(stderr, "error: could not open %s\n", filename);
		exit(1);
	}
	fwrite(head, 4, 8, fp);
	for (i = 0; i < ROWS; i++)
		fwrite(simage[i], sizeof(char), COLS, fp);
	fclose(fp);

	//printf("Press any key to exit: ");
	//gets(&ch);

	return 0;
}

void clear( unsigned char image[][COLS] )
{
	int	i,j;
	for ( i = 0 ; i < ROWS ; i++ )
		for ( j = 0 ; j < COLS ; j++ ) image[i][j] = 0;
}

void header( int row, int col, unsigned char head[32] )
{
	int *p = (int *)head;
	char *ch;
	int num = row * col;

	/* Choose little-endian or big-endian header depending on the machine. Don't modify this */
	/* Little-endian for PC */
	
	*p = 0x956aa659;
	*(p + 3) = 0x08000000;
	*(p + 5) = 0x01000000;
	*(p + 6) = 0x0;
	*(p + 7) = 0xf8000000;

	ch = (char*)&col;
	head[7] = *ch;
	ch ++; 
	head[6] = *ch;
	ch ++;
	head[5] = *ch;
	ch ++;
	head[4] = *ch;

	ch = (char*)&row;
	head[11] = *ch;
	ch ++; 
	head[10] = *ch;
	ch ++;
	head[9] = *ch;
	ch ++;
	head[8] = *ch;
	
	ch = (char*)&num;
	head[19] = *ch;
	ch ++; 
	head[18] = *ch;
	ch ++;
	head[17] = *ch;
	ch ++;
	head[16] = *ch;
	

	/* Big-endian for unix */
	/*
	*p = 0x59a66a95;
	*(p + 1) = col;
	*(p + 2) = row;
	*(p + 3) = 0x8;
	*(p + 4) = num;
	*(p + 5) = 0x1;
	*(p + 6) = 0x0;
	*(p + 7) = 0xf8;
*/
}

