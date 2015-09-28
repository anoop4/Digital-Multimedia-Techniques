#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>

#pragma pack(push,1)

#define PI 3.141592654
#define block_size 8

typedef struct Header
{
	unsigned short int type;
	unsigned int filesize;
	unsigned int reserved;
	unsigned int offset;
}Header1;

typedef struct Infoheader
{
	unsigned int size;
	int width,height;
	unsigned short int planes;
	unsigned short int bits;
	unsigned int compression;
	unsigned int imagesize;
	int xres,yres;
	unsigned int coloursused;
	unsigned int importantcolours;
}Header2;

#pragma pack(pop)

void vector2matrix(unsigned char** red, unsigned char** green, unsigned char** blue,unsigned char* image_contents_rgb,int height,int width)
{
	int i,j,k;
	for(i=0,j=0,k=0; i< height*width*3 ; i+=3) /*storing image contents into three seperate vectors corresponding to three colors*/
	{
		blue[j][k] 	= image_contents_rgb[i];
		green[j][k] = image_contents_rgb[i+1];
		red[j][k] 	= image_contents_rgb[i+2];
		k++;
		if(k == width)
		{
			j++;
			k=0;	
		}
	}
}

void rgb2yuv(unsigned char** red,unsigned char** green,unsigned char** blue,unsigned char** Y,unsigned char** U,unsigned char** V, int height,int width)
{
	int i,j;
	float y,u,v;
	for(i=0 ; i<height; ++i)			/* conversion from RGB to YUV space*/
	{
		for(j=0; j<width; ++j)
		{
			y = (float)(red[i][j] * 0.2999 + green[i][j] * 0.587 + blue[i][j] * 0.114);
			if(y > 255)
				y = 255;
			if(y< 0)
				y = 0;
			Y[i][j] = (unsigned char)y;

			u = (float)(red[i][j] * (-0.168736) + green[i][j] * (-0.331264) + blue[i][j] * (0.500002) + 128);
			if(u > 255)
				u = 255;
			if(u < 0)
				u = 0;
			U[i][j] = (unsigned char)u;

			v = (float)(red[i][j] * 0.5 + green[i][j] * (-0.418688) + blue[i][j] * (-0.081312) + 128);
			if(v > 255 )
				v = 255;
			if(v < 0)
				v = 0;
			V[i][j] = (unsigned char)v; 
		}
	}
	
}


void yuv2rgb(unsigned char** red,unsigned char** green,unsigned char** blue,unsigned char** Y,unsigned char** U,unsigned char** V, int height,int width)
{
	int i,j;
	float r,g,b;
	for(i=0 ; i<height ; ++i)
	{
		for(j=0 ; j<width ; ++j)
		{
			
			g = (float) (Y[i][j]+(U[i][j] - 128 ) * (-0.34414) + (V[i][j] - 128) * (-0.71414));
			if(g > 255)
				g = 255;
			if( g < 0)
				g = 0;
			green[i][j] = (unsigned char)g;

			r = (float)(Y[i][j] +(V[i][j] - 128) * 1.4021);						/* conversion from YUV back to RGB*/
			if(r > 255)
				r = 255;
			if(r < 0)
				r = 0;
			red[i][j] = (unsigned char)r;

			b = (float)(Y[i][j] + (U[i][j] - 128) * 1.77180);
			if(b > 255)
				b = 255;
			if(b < 0)
				b = 0;
			blue[i][j] = (unsigned char)b; 	 
		}
	}
}

void dct(int array[8][8])
{
	int x,y,u,v;
	float result[8][8], C_u, C_v, sum;
	for (x=0; x<block_size; ++x)
	{
		printf("\n");
		for(y=0; y<block_size; ++y)
		{
			printf("%d\t",array[x][y]);
		}
	}

	printf("\n ****************************** DCT matrix *************************** \n");
	for(u=0; u< block_size; ++u)
	{
		C_u = (u == 0)? 1/sqrt(2) : 1;
		for(v=0; v<block_size ;++v)
		{
			C_v = (v == 0)? 1/sqrt(2) : 1;
			sum = 0;
			for(x=0; x<block_size; ++x)
			{
				for(y=0; y<block_size; ++y)
				{
					sum += array[x][y] * cos ( PI * (2*x + 1) * u / (float)16) * cos( PI * (2*y + 1) * v / (float)16);
				}
				result[u][v] = 0.25 * sum * C_u * C_v;
			}
		}
	}

	for (u=0; u<block_size; ++u)
	{
		printf("\n");
		for(v=0; v<block_size; ++v)
		{
			printf("%.2f \t",result[u][v]);
		}
	}
}

void matrix2vector_rgb( unsigned char** red, unsigned char** green, unsigned char** blue,unsigned char* image_contents_rgbt,int height,int width)
{
	int i,j,k;
	for(i=0,j=0,k=0 ; i<height*width*3 ; i+=3)
	{
		image_contents_rgbt[i] 	= blue[j][k];	/* storing back the quantized value into image_contents */
		image_contents_rgbt[i+1] = green[j][k];
		image_contents_rgbt[i+2] = red[j][k];
		k++;
		if(k == width)
		{
			k=0;
			++j;
		}
	}
}

void matrix2vector_yuv( unsigned char** Y, unsigned char** U,unsigned char** V,unsigned char* image_contents_yuv,int height,int width)
{
	int i,j,k;
	for(i=0,j=0,k=0 ; i<height*width*3 ; i+=3)
	{
		image_contents_yuv[i] 	= V[j][k];										/* storing back the quantized value into image_contents */
		image_contents_yuv[i+1] = U[j][k];
		image_contents_yuv[i+2] = Y[j][k];
		k++;
		if(k == width)
		{
			k=0;
			++j;
		}
	}
}

int main(int argc, char const *argv[])
{
	FILE*fp,*image;
	unsigned char** red,**green,**blue,**Y,**U,**V,*image_contents_rgb, *image_contents_yuv, *image_contents_rgbt;
	int i,j,x,y,row_index, col_index,ERROR;
	Header1 header1;
	Header2 header2;
	/* -----------------------------				Task 1			-------------------------------------- */
	 int array[8][8] = {{48,39,40,68,60,38,50,121},
	{149,82,79,101,113,106,27,62},
	{58,63,77,69,124,107,74,125},
	{80,97,74,54,59,71,91,66},
	{18,34,33,46,64,61,32,37},
	{149,108,80,106,116,61,73,92},
	{211,233,159,88,107,158,161,109},
	{212,104,40,44,71,136,113,66}};
	printf("\n\n *************************** Activity 1: SAMPLE BLOCK *************** \n");
	dct(array);
	/*----------------------------					Task2 			----------------------------------------- */
	srand(time(NULL));
	printf("\n\n *************************** Activity 2: RANDOM BLOCK **************** \n\n");
	for(i =0; i<block_size; ++i)
	{
		for(j=0; j<block_size; ++j)
		{
			array[i][j] = rand()%150;
		}
	}
	dct(array);
	printf("\n");
	/*------------------------------			Task3 				------------------------------------------- */
	if(argc!= 2)																/* syntax error check*/
	{
		printf("\n The format is ./quantizer [file-name]");
		return -1;
	}

	if((fp = fopen(argv[1],"rb"))== NULL) 										/* open the .bmp file for reading*/
	{
		printf("\n Error opening the file specified. Please check if the file exists !!\n");
		fclose(fp);
		return -1;
	}

	if(fread(&header1,sizeof(header1),1,fp)!=1) 								/* Read the primary header from the bmp file */
	{
		printf("\n Error reading header1 \n");
		fclose(fp);
		return -1;
	}

	if(header1.type!= 19778)													/* check if its a valid bmp file*/
	{
		printf("\n Not a bmp file. Exiting !! \n");
		fclose(fp);
		return -1;
	}

	if(fread(&header2,sizeof(header2),1,fp)!=1 )
	{
		printf("\n Error reading header 2");
		fclose(fp);
		return -1;
	} 

	image_contents_rgb = (unsigned char*)malloc(sizeof(char) * header2.imagesize); 
	image_contents_rgbt = (unsigned char*)malloc(sizeof(char) * header2.imagesize);	/*allocate memory to store image data*/
	image_contents_yuv = (unsigned char*)malloc(sizeof(char) * header2.imagesize);

	fseek(fp,header1.offset,SEEK_SET); 	

	if((ERROR=fread(image_contents_rgb,header2.imagesize,1,fp))!=1)
	{
		printf("\nError reading contents\n");
		free(image_contents_rgb);
		fclose(fp);
		return -1;
	} 

	fclose(fp);

	red = (unsigned char**)malloc(sizeof(char*) * header2.height);
	green = (unsigned char**)malloc(sizeof(char*) * header2.height);
	blue = (unsigned char**)malloc(sizeof(char*) * header2.height);
	Y = (unsigned char**)malloc(sizeof(char*) * header2.height);
	U = (unsigned char**)malloc(sizeof(char*) * header2.height);
	V = (unsigned char**)malloc(sizeof(char*) * header2.height);

	for(i=0 ; i<header2.height ;i++)
	{
		red[i] = (unsigned char*)malloc( sizeof(char) * header2.width);
		green[i]= (unsigned char*)malloc( sizeof(char) * header2.width);
		blue[i]= (unsigned char*)malloc( sizeof(char) * header2.width);
		Y[i] = (unsigned char*)malloc( sizeof(char) * header2.width);
		U[i] = (unsigned char*)malloc( sizeof(char) * header2.width);
		V[i] = (unsigned char*)malloc( sizeof(char) * header2.width);
	}	
	
	vector2matrix(red,green,blue,image_contents_rgb,header2.height,header2.width); 		/* call to store image contents as matrix */
	printf("\n\n *********************** Activity 3: RGB Image Block *********************\n");
	printf(" Enter the starting row index and coloumn index respectively: ");
	scanf("%d %d",&row_index,&col_index);
	for(i = 0, x = row_index; i < block_size && x<row_index+8; ++i, ++x)
	{
		for(j= 0, y = col_index; j < block_size && y<col_index+8; ++j, ++y)
		{
			if(x<512 && y<512)
				array[i][j] = blue[x][y];
			else
				array[i][j] = 0;
		}
	}
	dct(array);
	rgb2yuv(red,green,blue,Y,U,V,header2.height,header2.width);								/* Downsampling*/
	printf("\n\n *********************** Activity 4: YUV Image Block *********************\n");
	printf(" Enter the starting row index and coloumn index respectively: ");
	scanf("%d %d",&row_index,&col_index);
	for(i = 0, x = row_index; i < block_size && x<row_index+8; ++i, ++x)
	{
		for(j= 0, y = col_index; j < block_size && y<col_index+8; ++j, ++y)
		{
			if(x<512 && y<512)
				array[i][j] = V[x][y];
			else
				array[i][j] = 0;
		}
	}
	dct(array);
	yuv2rgb(red,green,blue,Y,U,V,header2.height,header2.width);							/* convert back to RGB*/									
	matrix2vector_rgb(red,green,blue,image_contents_rgbt,header2.height,header2.width); /* convert back from matrix to vector*/
	matrix2vector_yuv(Y,U,V,image_contents_yuv,header2.height,header2.width);

	if((image = fopen("Output_image","wb")) == NULL)
	{
		printf("\n ERROR opening the file to write quantized image !!\n");
		fclose(image);
		return -1;
	}

	if(fwrite(&header1,sizeof(header1),1,image)!= 1)
	{
		printf("\n ERROR writing header 1 into destination file !!\n");
		fclose(image);
		return -1;
	}

	if(fwrite(&header2,sizeof(header2),1,image)!= 1)
	{
		printf("\n ERROR writing header 2 into destination file !! \n");
		fclose(image);
		return -1;
	}

	fseek(image, header1.offset, SEEK_SET);

	if(fwrite(image_contents_rgbt,header2.imagesize,1,image)!=1)					/* Change the vector to write into the bmp file here*/
	{
		printf("\n ERROR writing image contents into destination file \n");
		fclose(image);
		return -1;
	}

	free(red);
	free(green);
	free(blue);
	free(Y);
	free(U);
	free(V);
	free(image_contents_rgb);
	free(image_contents_yuv);
	fclose(image);
	return 0;
}
