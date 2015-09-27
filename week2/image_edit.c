/*	This program is done as part of the DIgital Multimedia course @ IIIT-Bangalore (Week2)
	Author:	Srijith V	
	Program: To calculate the PSNR of an edited image with respect to the original image
	Date: 27th September 2015
*/

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
	/*storing image contents into three seperate vectors corresponding to three colors*/
	int i,j,k;
	for(i=0,j=0,k=0; i< height*width*3 ; i+=3)
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

	/* to make a horizontal black line in the image */
	for(j=0; j<512 ; ++j)
	{
		for(k=0; k<512; ++k)
		{
			if(j>200 && j<250)
			{
				blue[j][k] = red[j][k] = green[j][k] = 0;
			}
		}
	}
}


void matrix2vector_rgb( unsigned char** red, unsigned char** green, unsigned char** blue,unsigned char* image_contents_rgbt,int height,int width)
{
  /* storing back the quantized value into image_contents */
	int i,j,k;
	for(i=0,j=0,k=0 ; i<height*width*3 ; i+=3)
	{
		image_contents_rgbt[i] 	= blue[j][k];
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


void snr(unsigned char* image_contents_rgb, unsigned char* image_contents_rgbt, int height, int width)
{
  /*PSNR caluclation*/
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
	unsigned char** red,**green,**blue, *image_contents_rgb, *image_contents_rgbt;
	int i, ERROR;
	Header1 header1;
	Header2 header2;

	if(argc!= 2)												
	{
	  /* syntax error check*/
		printf("\n The format is ./quantizer [file-name]");
		return -1;
	}

	if((fp = fopen(argv[1],"rb"))== NULL)
	{
	  /* open the .bmp file for reading*/
		printf("\n Error opening the file specified. Please check if the file exists !!\n");
		fclose(fp);
		return -1;
	}

	if(fread(&header1,sizeof(header1),1,fp)!=1)				
	{
	  /* Read the primary header from the bmp file */
		printf("\n Error reading header1 \n");
		fclose(fp);
		return -1;
	}

	if(header1.type!= 19778)
	{
	  /* check if its a valid bmp file*/
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

	for(i=0 ; i<header2.height ;i++)
	{
		red[i] = (unsigned char*)malloc( sizeof(char) * header2.width);
		green[i]= (unsigned char*)malloc( sizeof(char) * header2.width);
		blue[i]= (unsigned char*)malloc( sizeof(char) * header2.width);
	}
	
	vector2matrix(red,green,blue,image_contents_rgb,header2.height,header2.width); 	/* call to store image contents as matrix */
	matrix2vector_rgb(red,green,blue,image_contents_rgbt,header2.height,header2.width); /*convert back from matrix to vector*/
	snr(image_contents_rgb, image_contents_rgbt, header2.height, header2.width);        /* Calulate PSNR*/

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

	if(fwrite(image_contents_rgbt,header2.imagesize,1,image)!=1)
	{
	  /*Store the edited RGB components into a bmp file*/
		printf("\n ERROR writing image contents into destination file \n");
		fclose(image);
		return -1;
	}

	free(red);
	free(green);
	free(blue);
	free(image_contents_rgb);
	fclose(image);
	return 0;
}
