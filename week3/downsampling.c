#include<stdio.h>
#include<stdlib.h>
#include<math.h>

#pragma pack(push,1)

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
		image_contents_yuv[i] 	= V[j][k]; /* storing back the quantized value into image_contents */
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

void downsampling(unsigned char** Y, unsigned char** U, unsigned char** V, int height, int width)
{
	int j,k,ch;
	unsigned char max;

	printf("\n Enter the type of Quantization \n 1. 4:1:1 Quantization \n 2. 4:2:2 Quantization \n 3. 4:2:0 Quantization\n ");
	scanf("%d",&ch);
	if(ch == 1)			
	{	
		/* 4:1:1 Downsampling*/
		for(j=0; j<512; j++)
		{	
			for(k=0; k<512; k+=4)
			{	
				max = (U[j][k+1]>U[j][k+2]) ? U[j][k+1] : U[j][k+2];
				max = (U[j][k+3]>max) ? U[j][k+3] : max;
				U[j][k+1] = U[j][k+2] = U[j][k+3] = U[j][k] = max;
				max = (V[j][k+1]>V[j][k+2]) ? V[j][k+1] : V[j][k+2];
				max = (V[j][k+3]>max) ? V[j][k+3] : max;
				V[j][k+1] = V[j][k+2] = V[j][k+3] = V[j][k] = max;
			}
		}

	}
	else if(ch == 2)
	{
		/* 4:2:2 Downsampling*/ 
		for(j=0; j<512; j++)
		{	
			for(k=0; k<512; k+=2)
			{
				if(U[j][k]>U[j][k+1])
					U[j][k+1] = U[j][k];
				else
					U[j][k] = U[j][k+1];
				if(V[j][k] > V[j][k+1])
				 	V[j][k+1] = V[j][k];
				else
				 	V[j][k] = V[j][k+1];
			}
		}
	}	

	else if(ch == 3)
	{
		/* 4:2:0 Downsampling */
		for(j=0; j<512; j+=2)
		{
			for(k=0; k<512; k+=2)
			{
				max = (U[j][k+1] > U[j+1][k])? U[j][k+1]:U[j+1][k];
				max = (U[j+1][k+1] > max)? U[j+1][k+1]:max;
				U[j][k] = U[j+1][k] = U[j][k+1] = U[j+1][k+1] = max;
				max = (V[j][k+1] > V[j+1][k])? V[j][k+1]:V[j+1][k];
				max = (V[j+1][k+1] > max)? V[j+1][k+1]:max;
				V[j][k] = V[j+1][k] = V[j][k+1] = V[j+1][k+1] = max;
			}
		}
	}

	else
		printf("\n Invalid option !!");
}

void snr(unsigned char* image_contents_rgb, unsigned char* image_contents_rgbt, int height, int width)
{
	int j;
	float mse=0;
	float psnr;

	for(j=0; j<(height*width*3); ++j)
		{				
			mse += pow((int)image_contents_rgb[j] - (int)image_contents_rgbt[j], 2);
		}
	mse = mse/(float)(height*width*3);
	printf("\n MSE: %f",mse);
	psnr = (20 * log10(255)) - (10 * log10(mse));
	printf("\n PSNR: %0.2f db\n", psnr);
}

int main(int argc, char const *argv[])
{
	FILE*fp,*image;
	unsigned char** red,**green,**blue,**Y,**U,**V,*image_contents_rgb, *image_contents_yuv, *image_contents_rgbt, *image_contents_ds;
	int i, ERROR;
	Header1 header1;
	Header2 header2;

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
	image_contents_ds = (unsigned char*)malloc(sizeof(char) * header2.imagesize);

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
	rgb2yuv(red,green,blue,Y,U,V,header2.height,header2.width); 						/* convert from RGB to YUV*/
	downsampling(Y,U,V,header2.height,header2.width);	
	matrix2vector_yuv(Y,U,V,image_contents_yuv,header2.height,header2.width);			/* Store YUV into vector */								
	yuv2rgb(red,green,blue,Y,U,V,header2.height,header2.width); 						/* convert back to RGB*/
	matrix2vector_rgb(red,green,blue,image_contents_rgbt,header2.height,header2.width); /* convert back from matrix to vector*/
	snr(image_contents_rgb, image_contents_rgbt, header2.height, header2.width);		/* Calculate SNR */

	if((image = fopen("quant_image","wb")) == NULL)
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

	if(fwrite(image_contents_rgbt,header2.imagesize,1,image)!=1)					/* CHnage the vector to write into the bmp file here*/
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
