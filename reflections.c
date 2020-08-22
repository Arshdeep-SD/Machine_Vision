#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ROWS 480
#define COLS 640

void clear( unsigned char image[][COLS] );
void header( int row, int col, unsigned char head[32] );

void assign_H ( float H[3], float S[3], float V[3] );
void assign_N ( float N[3], int x, int y, int r );
float assign_L ( float H[3], float S[3], float N[3], float a, float m ); 

int main( int argc, char **argv )
{
	int		i, j;
	int 	x, y;
	unsigned char	image[ROWS][COLS], head[32];
	FILE		*fp;
	
	float 	S[3], V[3] = {0,0,1}, H[3], N[3];  
	float	L;
	int 	r;
	float 	a, m;
	

	if ( argc != 2 )
	{
	  fprintf( stderr, "usage: %s output-file\n", argv[0] );
	  exit( 1 );
	}
	
	clear(image);
	
	switch(argv[1][6])
	{
	  case 'a':
	  {
	    S[0] = S[1] = 0;
	    S[2] = 1;
	    r = 50;
	    a = 0.5;
	    m = 1;
		break;
	  }
	
	  case 'b':
	  {
	    S[0] = S[1] = S[2] = sqrt(0.33333);
	    r = 50;
	    a = 0.5;
	    m = 1;
		break;
	  }

	  case 'c':
	  {
	    S[0] = 1;
	    S[1] = S[2] = 0;
	    r = 50;
	    a = 0.5;
	    m = 1;
		break;		
	  }

	  case 'd':
	  {
	    S[0] = S[1] = 0;
	    S[2] = 1;
	    r = 10;
	    a = 0.5;
	    m = 1;
		break;
	  }

	  case 'e':
	  {
		S[0] = S[1] = 0;
		S[2] = 1;
		r = 100;
		a = 0.5;
		m = 1;
		break;
	  }		
	
	  case 'f':
	  {
		S[0] = S[1] = 0;
		S[2] = 1;
		r = 50;
		a = 0.1;
		m = 1;	
		break;
	  }
	  
	  case 'g':
	  {
		S[0] = S[1] = 0;
		S[2] = 1;
		r = 50;
		a = 1;
		m = 1;
		break;
	  }		
	
	  case 'h':
	  {
		S[0] = S[1] = 0;
		S[2] = 1;
		r = 50;
		a = 0.5;
		m = 0.1;
		break;
	  }		
	
	  case 'i':
	  {
		S[0] = S[1] = 0;
		S[2] = 1;
		r = 50;
		a = 0.5;
		m = 10000;
		break;
	  }
	  
	  default:
	  {
		S[0] = S[1] = 0;
	    S[2] = 1;
	    r = 50;
	    a = 0.5;
	    m = 1;
	  }
	}
	
	for (i=-(ROWS/2); i<(ROWS/2); i++)
	{
	  for (j=-(COLS/2); j<(COLS/2); j++)
	  {
	    assign_H(H, S, V);
		assign_N(N, j, i, r);
		L = assign_L(H, S, N, a, m);
		
		if (L < 0)
		  L = 0;
		if (L > 1)
		  L = 1;
		image[i+(ROWS/2)][(COLS/2)-j-1] = L * 255;
	  }
	}
	


	header(ROWS, COLS, head);
	if (!( fp = fopen( argv[1], "wb" ) ))
	{
	  fprintf( stderr, "error: could not open %s\n", argv[1] );
	  exit( 1 );
	}
	fwrite(head, 4, 8, fp);
	for ( i = 0 ; i < ROWS ; i++ )
	  fwrite( image[i], sizeof(char), COLS, fp );
	fclose( fp );

	return 0;
}

void assign_H ( float H[3], float S[3], float V[3] )
{
	int i;
	float temp[3];
	for (i=0; i<3; i++)
	{
	  H[i] = S[i] + V[i];
	  temp[i] = H[i];
	}
	for (i=0; i<3; i++)
	{
	  H[i] = temp[i] / sqrt(pow(temp[0],2)+pow(temp[1],2)+pow(temp[2],2));
	}
}

void assign_N ( float N[3], int x, int y, int r )
{
	int i;
	float temp[3];
	if (pow(x,2)+pow(y,2) == pow(r,2))
	{
	  N[0] = -1;
	  N[1] = -1;
	  N[2] = 0;
	}
	else
	{
	  N[0] = -(x / sqrt(pow(r,2) - pow(x,2) - pow(y,2)));
	  N[1] = -(y / sqrt(pow(r,2) - pow(x,2) - pow(y,2)));
	  N[2] = 1;
	}
	for (i=0; i<3; i++)
	{
	  temp[i] = N[i];
	}
	for (i=0; i<3; i++)
	{
	  N[i] = temp[i] / sqrt(pow(temp[0],2)+pow(temp[1],2)+pow(temp[2],2));
	}
}

float assign_L ( float H[3], float S[3], float N[3], float a, float m )
{
	float L_l, alpha, L_s;
	L_l = S[0]*N[0] + S[1]*N[1] + S[2]*N[2];
	alpha = acos(H[0]*N[0] + H[1]*N[1] + H[2]*N[2]);
	L_s = exp(-pow((alpha/m),2));
	return ((a*L_l)+((1-a)*L_s));
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