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

void quantizer( unsigned char** Y, unsigned char** U, unsigned char** V, int height, int width)
{
	int i,j, quant_value;
	printf("\n Enter number of bits to be quantized (<=8): ");
	scanf("%d",&quant_value);
	unsigned char temp;
	temp = (unsigned char)(0xFF << quant_value);
	for(i=0; i<height; ++i)													/* quantize each pixel value*/
	{
		for(j=0; j < width; ++j)
		{
			Y[i][j] &= 0xFF;
			U[i][j] &= temp;
			V[i][j] &= temp;
		}
	}
}

void downsampling(unsigned char** Y, unsigned char** U, unsigned char** V, int height, int width)
{
	int j,k,ch;
	unsigned char max;

	printf("\n Enter the type of Downsampling \n 1. 4:4:4 Downsampling \n 2. 4:1:1 Downsampling \n 3. 4:2:2 Downsampling \n 4. 4:2:0 Downsampling\n ");
	scanf("%d",&ch);

	if(ch == 1)
	{

	}

	else if(ch == 2)			
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

	else if(ch == 3)
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

	else if(ch == 4)
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

void entropy_rgb(unsigned char* image_contents_rgbt, int freq_r[256], int freq_g[256], int freq_b[256], int freq_image[256], int rgboryuv)
{
	int i;
	float pi,ent_r,ent_g,ent_b,ent_image =0;
	pi = ent_r = ent_g = ent_b = ent_image = 0;
	for(i=0; i<256; ++i)
	{
		freq_r[i] = freq_g[i] =freq_b[i] =freq_image[i] = 0;
	}

	for(i=0; i<512*512*3; i+=3)
	{
		freq_b[(unsigned char)image_contents_rgbt[i]]++;
		freq_g[(unsigned char)image_contents_rgbt[i+1]]++;
		freq_r[(unsigned char)image_contents_rgbt[i+2]]++;
		freq_image[(unsigned char)image_contents_rgbt[i]]++;
		freq_image[(unsigned char)image_contents_rgbt[i+1]]++;
		freq_image[(unsigned char)image_contents_rgbt[i+2]]++;
	}
	
	for(i=0; i<256; ++i)
	{
		pi = (float)freq_r[i] / (512*512);
		if(pi!=0)
		{
			ent_r += (pi * (log(1/pi)) / log(2));
		}

		pi = (float)freq_g[i] / (512*512);
		if(pi!=0)
		{
			ent_g += (pi * (log(1/pi)) / log(2));
		} 

		pi = (float)freq_b[i] / (512*512);
		if(pi!=0)
		{
			ent_b += (pi * (log(1/pi)) / log(2));
		}

		pi = (float)freq_image[i] / (3*512*512);
		if(pi!=0)
		{
			ent_image += (pi * (log(1/pi)) / log(2));
		} 
	}

	if(rgboryuv == 0)
	{
		printf("\n Entropy of Red = %f bits", ent_r);
		printf("\n Entropy of Green = %f bits", ent_g);
		printf("\n Entropy of Blue = %f bits", ent_b);
		printf("\n Entropy of image = %f bits\n\n", ent_image);
	}

	else if(rgboryuv == 1)
	{
		printf("\n Entropy of Y = %f bits", ent_r);
		printf("\n Entropy of U (Cr) = %f bits", ent_g);
		printf("\n Entropy of V (Cb) = %f bits", ent_b);
		printf("\n Entropy of image = %f bits\n\n", ent_image);
	}
}

int main(int argc, char const *argv[])
{
	FILE*fp,*image;
	int freq_r[256], freq_g[256], freq_b[256], freq_image[256];
	unsigned char** red,**green,**blue,**Y,**U,**V,*image_contents_rgb, *image_contents_yuv, *image_contents_rgbt;
	int i, ERROR, option;
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
	

	/* Calculate the entropy value for the RGB image at different downsampling value */

	printf("\n Entropy calculation of the RGB image: \n");
	vector2matrix(red,green,blue,image_contents_rgb,header2.height,header2.width);		/* call to store image contents as matrix */
	downsampling(red,green,blue,header2.height,header2.width);							/* Downsampling*/							
	matrix2vector_rgb(red,green,blue,image_contents_rgbt,header2.height,header2.width); /* convert back from matrix to vector*/
	entropy_rgb(image_contents_rgbt, freq_r,freq_g,freq_b, freq_image,0);				/* Calulate the entropy*/	
	
	/* calculate the entropy values for YUV components*/
	printf("\n Entropy calculation of the YUV components: \n");
	vector2matrix(red,green,blue,image_contents_rgb,header2.height,header2.width); 		/* call to store image contents as matrix */
	rgb2yuv(red,green,blue,Y,U,V,header2.height,header2.width);
	printf("\n 1. Quantization 2. Downsampling -------> Choose any one:  ");
	scanf("%d",&option);
	if(option == 1)	 									
		quantizer(Y,U,V,header2.height, header2.width);									/* Quantize the YUV values*/
	else if (option ==2)										
		downsampling(Y,U,V,header2.height,header2.width);								/* Downsampling*/
	matrix2vector_yuv(Y,U,V,image_contents_yuv,header2.height,header2.width);			/* Store YUV into vector */
	entropy_rgb(image_contents_yuv, freq_r,freq_g,freq_b, freq_image,1);				/* Calculate the entropy */
	yuv2rgb(red,green,blue,Y,U,V,header2.height,header2.width);							/* convert back to RGB*/									
	matrix2vector_rgb(red,green,blue,image_contents_rgbt,header2.height,header2.width); /* convert back from matrix to vector*/

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
