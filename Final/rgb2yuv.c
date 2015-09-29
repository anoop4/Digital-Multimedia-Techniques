/* Program to convert the RGB image into YUV and store it into a file*/

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

/******************************************************************************************************************************************/
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

  /***************************************************************************************************************************************/
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
	printf("\n RGB to YUV Conversion ========> Successfull :)");
  
  }

  /**************************************************************************************************************************************/
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

			r = (float)(Y[i][j] +(V[i][j] - 128) * 1.4021);				/* conversion from YUV back to RGB*/
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

 /***************************************************************************************************************************************/
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

/***********************************************************************************************************************************************/
  void matrix2vector_yuv( unsigned char** Y, unsigned char** U,unsigned char** V,unsigned char* image_contents_yuv,int height,int width)
  {
	
	int i,j,k;
	for(i=0,j=0,k=0 ; i<height*width*3 ; i+=3)
	{
		image_contents_yuv[i] 	= V[j][k];			/* storing back the quantized value into image_contents */
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

/*****************************************************************************************************************************************/
  void downsampling(unsigned char** Y, unsigned char** U, unsigned char** V, int height, int width)
  {
  	
  	int row, col;
  	for(row=0; row<height; row+=2)
		{
			for(col=0; col<width; col+=2)
			{
				  U[row+1][col] = U[row][col+1] = U[row+1][col+1] = U[row][col];
				  V[row+1][col] = V[row][col+1] = V[row+1][col+1] = V[row][col];
			}
		}
	printf("\n 4:2:0 Downsampling ======> Success :)");
  
  }

/*****************************************************************************************************************************************/
  void FileWrite(unsigned char** red, unsigned char** green, unsigned char** blue, FILE* image, int height, int width)
  {
	
	int outer_r, outer_c, inner_r, inner_c;
	for(outer_r =0; outer_r < height; outer_r += 8)
	{
		for(outer_c = 0; outer_c < width; outer_c += 8)
		{
			for(inner_r = outer_r; inner_r < outer_r+8; inner_r++)
			{
				for(inner_c = outer_c; inner_c < outer_c+8; inner_c++)
				{
					fprintf(image,"%d\t",red[inner_r][inner_c]);
				}
				fprintf(image,"\n");
			}
		}
	}

	for(outer_r =0; outer_r < height; outer_r += block_size)
	{
		for(outer_c = 0; outer_c < width; outer_c += block_size)
		{
			for(inner_r = outer_r; inner_r < outer_r+block_size; inner_r++)
			{
				for(inner_c = outer_c; inner_c < outer_c+block_size; inner_c++)
				{
					fprintf(image,"%d\t",green[inner_r][inner_c]);
				}
				fprintf(image,"\n");
			}
		}
	}

	for(outer_r =0; outer_r < height; outer_r += block_size)
	{
		for(outer_c = 0; outer_c < width; outer_c += block_size)
		{
			for(inner_r = outer_r; inner_r < outer_r+block_size; inner_r++)
			{
				for(inner_c = outer_c; inner_c < outer_c+block_size; inner_c++)
				{
					fprintf(image,"%d\t",blue[inner_r][inner_c]);
				}
				fprintf(image,"\n");
			}
		}
	}
	fclose(image);
}

/**************************************************************************************************************************************/
void dct(FILE* DCT, int height, int width)
{
	
	FILE* image;
	if((image = fopen("Down_Sampled_YUV", "rb")) == NULL)
	{
		printf("\n In function dct() --> Error opening the Down_Sampled_YUV file !!");
		exit(0);
	}
	int count,x,y,u,v;
	int array[8][8];
	float result[8][8], C_u, C_v, sum;

	for(count=0; count<12288; ++count)						/* 12288 is the total number of 8*8 grids in the image*/ 
	{
		for(x=0; x<8; x++)
		{
			for(y=0; y<8; ++y)
			{
				fscanf(image,"%d",&array[x][y]);
			}
		}

		/* Perform DCT on the chosen 8*8 grid */

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

		/* write the DCT result into the file*/

		for(x=0; x<8; x++)
		{
			for(y=0; y<8; y++)
			{
				fprintf(DCT,"%.2f\t",result[x][y]);
			}
			fprintf(DCT,"\n");
		}
	}
	fclose(image);
	fclose(DCT);
	printf("\n DCT operation =======> Sucessfull :) ");
}

/************************************************************************************************************************************
void update_yuv(unsigned char** Y, unsigned char** U, unsigned char** V, int height, int width)
{
	FILE* fp;
	int outer_r, inner_r, outer_c, inner_c;

	if((fp = fopen("Down_Sampled_YUV","rb")) == NULL)
	{
		printf("\n In update_yuv() Error opening DCT_output file !");
		exit(0);
	}

	for(outer_r =0; outer_r < height; outer_r += 8)
	{
		for(outer_c = 0; outer_c < width; outer_c += 8)
		{
			for(inner_r = outer_r; inner_r < outer_r+8; inner_r++)
			{
				for(inner_c = outer_c; inner_c < outer_c+8; inner_c++)
				{
					fscanf(fp,"%d\t",Y[inner_r][inner_c]);
				}
			}
		}
	}

	for(outer_r =0; outer_r < height; outer_r += 8)
	{
		for(outer_c = 0; outer_c < width; outer_c += 8)
		{
			for(inner_r = outer_r; inner_r < outer_r+8; inner_r++)
			{
				for(inner_c = outer_c; inner_c < outer_c+8; inner_c++)
				{
					fscanf(fp,"%d\t",U[inner_r][inner_c]);
				}
			}
		}
	}

	for(outer_r =0; outer_r < height; outer_r += 8)
	{
		for(outer_c = 0; outer_c < width; outer_c += 8)
		{
			for(inner_r = outer_r; inner_r < outer_r+8; inner_r++)
			{
				for(inner_c = outer_c; inner_c < outer_c+8; inner_c++)
				{
					fscanf(fp,"%d\t",V[inner_r][inner_c]);
				}
			}
		}
	}

}

***************************************************************************************************************************************/
  int main(int argc, char const *argv[])
   {
	FILE*fp, *image_rgb, *image_yuv, *down_sampled, *DCT, *image;
	unsigned char** red, **green, **blue, **Y, **U, **V, *image_contents_rgb, *image_contents_yuv, *image_contents_rgbt;
	int i,ERROR;
	Header1 header1;
	Header2 header2;
	
	if((image_rgb = fopen("Image_Contents_RGB","wb")) == NULL)	/*For storing RGB contents*/
	{
		printf("\n Error creating Image_Contents_RGB file !!");
		return -1;
	}
	
	if((image_yuv = fopen("Image_Contents_YUV","wb")) == NULL)	/* For storing YUV contents*/
	{
		printf("\n Error creating Image_Contents_YUV file !!");
		return -1;
	}

	if((down_sampled = fopen("Down_Sampled_YUV","wb")) == NULL)	/* For storing down sampled YUV contents*/
	{
		printf("\n Error creating Down_Sampled_YUV file !!");
		return -1;
	}

	if((DCT = fopen("DCT_Output","wb")) == NULL)				/* For storing DCT contents*/
	{
		printf("\n Error creating Down_Sampled_YUV file !!");
		return -1;
	}	

	if(argc!= 2)												/* syntax error check*/
	{
		printf("\n The format is ./quantizer [file-name]");
		return -1;
	}

	if((fp = fopen(argv[1],"rb"))== NULL) 						/* open the .bmp file for reading*/
	{
		printf("\n Error opening the file specified. Please check if the file exists !!\n");
		fclose(fp);
		return -1;
	}

	if(fread(&header1,sizeof(header1),1,fp)!=1) 				/* Read the primary header from the bmp file */
	{
		printf("\n Error reading header1 \n");
		fclose(fp);
		return -1;
	}

	if(header1.type!= 19778)									/* check if its a valid bmp file*/
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

	fseek(fp,header1.offset,SEEK_SET); 												/* To assign file pointer to start of image byte*/ 	

	if((ERROR = fread(image_contents_rgb,header2.imagesize,1,fp))!=1)
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
	
	vector2matrix(red, green, blue, image_contents_rgb, header2.height, header2.width); 	/* Store image contents as matrix */
	FileWrite(red, green, blue, image_rgb, header2.height, header2.width);					/* Optional step*/
	rgb2yuv(red,green,blue,Y,U,V,header2.height,header2.width);								/* RGB to YUV conversion*/
	FileWrite(Y, U, V, image_yuv, header2.height, header2.width);							/* Writing YUV into file*/			
	downsampling(Y, U, V, header2.height, header2.width);									/* 4:2:0 Downsampling and update YUV */ 
	FileWrite(Y, U, V, down_sampled, header2.height, header2.width);						/* Write downsampled YUV into file*/
	dct(DCT, header2.height, header2.width);												/* Perform dct and store the result into a file*/
	printf("\n");
	
	yuv2rgb(red,green,blue,Y,U,V,header2.height,header2.width);								/* Optional step*/
	matrix2vector_rgb(red,green,blue,image_contents_rgbt,header2.height,header2.width); 	/* convert back from matrix to vector*/
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

	if(fwrite(image_contents_rgbt,header2.imagesize,1,image)!=1)		/* Change the vector to write into the bmp file here*/
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
	fclose(image);
	return 0;
}
