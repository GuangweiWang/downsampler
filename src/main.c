/*!
 * \copy
 *     Copyright (c)  2013, Cisco Systems
 *     All rights reserved.
 *
 *     Redistribution and use in source and binary forms, with or without
 *     modification, are permitted provided that the following conditions
 *     are met:
 *
 *        * Redistributions of source code must retain the above copyright
 *          notice, this list of conditions and the following disclaimer.
 *
 *        * Redistributions in binary form must reproduce the above copyright
 *          notice, this list of conditions and the following disclaimer in
 *          the documentation and/or other materials provided with the
 *          distribution.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *     "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *     FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *     COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *     BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *     CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *     LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *     ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *     POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "downsample.h"
#include "measure_time.h"

#define COMPARETWOFUNCS
#define DUMPDETAILS

#define WELS_ABS(a) ((a)>0 ? (a):-(a))

typedef enum 
{
	HALF = 2,
	ONETHIRD = 3,
	QUARTER = 4,
	RANDOM = 0,

}DOWNSAMPLE_RATIO;

static SDownsampleFuncs downsample_funs_c[] = {
	{NULL,
	GeneralBilinearFastDownsampler_c, GeneralBilinearAccurateDownsampler_c },

	{ DyadicBilinearDownsampler_c, 
	GeneralBilinearFastDownsampler_c, GeneralBilinearAccurateDownsampler_c },

	{ BilinearDownsamplerOneThird_c, 
	GeneralBilinearFastDownsampler_c, GeneralBilinearAccurateDownsampler_c },

	{ BilinearDownsamplerQuarter_c,
	GeneralBilinearFastDownsampler_c, GeneralBilinearAccurateDownsampler_c },
};

static int getIndex[] = {0,0,1,2,3};

DOWNSAMPLE_RATIO getDownsampleRatio(uint64_t srcFrameSize, uint64_t dstFrameSize){
	DOWNSAMPLE_RATIO ret;
	float ratio = 1.0;
	ratio = ((float)srcFrameSize / (float)dstFrameSize);
	if (ratio == 4.0){ //1:2
		ret = HALF;
		printf("down sample ratio is 1:2\n");
	}
	else if (ratio == 9.0){ //1:3
		ret = ONETHIRD;
		printf("down sample ratio is 1:3\n");
	}
	else if (ratio == 16.0){ //1:4
		ret = QUARTER;
		printf("down sample ratio is 1:4\n");
	}
	else{
		ret = RANDOM; //
	}
	return ret;
}

void compareOutput(FILE *flog, uint8_t *dst_o, uint8_t *dst_n, int dst_width, int dst_height, int frame, char *yuvComponent){
	float k = 0.0;
	int i, j;

	for (i = 0; i < dst_height - 1; i++){
		for (j = 0; j < dst_width - 1; j++){
			if (dst_o[i*dst_width + j] != dst_n[i*dst_width + j]){
				k++;
				if (WELS_ABS(dst_o[i*dst_width + j] - dst_n[i*dst_width + j]) != 1)
					fprintf(flog, "coordinat %d,%d:dst_o=%d, dst_n=%d\n", i, j, dst_o[i*dst_width + j], dst_n[i*dst_width + j]);
				//break;
			}
		}
	}
#if defined(DUMPDETAILS)
	if (!strcmp(yuvComponent,"Y"))
		fprintf(flog, "\nframe = %d\n", frame);

	fprintf(flog, "%s:\n", yuvComponent);
	fprintf(flog, "different pixels = %f\n", k);
	fprintf(flog, "different pixels/total pixels = %d%%\n", (int)(k / (dst_width * dst_height) * 100));
#endif
}

int main (int argc, char **argv){
	int index;
	//int bufPost = 0;
	//int inFileSize = 0;
	int size = 0;
	int frameNum = 0;
	unsigned int compareTwoFuns = 0;
	DOWNSAMPLE_RATIO ratio;

	FILE *fIn = NULL;
	FILE *fOutA = NULL, *fOutB = NULL;
	FILE *flog = NULL;

	char *inFileName = NULL, *outFileName = NULL;
	char *outFileNameA = NULL, *outFileNameB = NULL;

	sPixMap SrcPix;
	sPixMap DstPix;

	sPixMap DstPix_general; //original
	sPixMap DstPix_specific; //new
	sPixMap *DstPix_a = &DstPix_general;
	sPixMap *DstPix_b = &DstPix_specific;

	uint64_t inFrameSizeY = 0, outFrameSizeY = 0;
	uint64_t inFrameSizeU = 0, outFrameSizeU = 0;
	uint64_t inFrameSizeV = 0, outFrameSizeV = 0;

	double iStartTime_g = 0, iStartTime_s = 0;
	double iTotalTime_g = 0, iTotalTime_s = 0;

	//parse command parameter
	if (argc != 8){
		printf("usage:\ndownsampler inFileName.yuv src_width src_height outFileName_general.yuv outFileName_specific.yuv dst_width dst_height\n");
		printf("Example:\ndownsampler infile.yuv 640 360 outfile_general.yuv outfile_specific.yuv 160 90\n");
		return -1;
	}
	else{
		inFileName = argv[1];
		SrcPix.width = atoi(argv[2]);
		SrcPix.height = atoi(argv[3]);

		//outFileName = argv[4];
		outFileNameA = argv[4];
		outFileNameB = argv[5];
		DstPix.width = atoi(argv[6]);
		DstPix.height = atoi(argv[7]);
	}

	if(DstPix.width == 0 || DstPix.height == 0)
		exit(1);
	
	//init parameters and assign memory for IO
	DstPix_a->width = DstPix_b->width = DstPix.width;
	DstPix_a->height = DstPix_b->height = DstPix.height;

	inFrameSizeY = SrcPix.width * SrcPix.height;
	inFrameSizeU = inFrameSizeV = inFrameSizeY >> 2;
	outFrameSizeY = DstPix.width * DstPix.height;
	outFrameSizeU = outFrameSizeV = outFrameSizeY >> 2;

	SrcPix.pPixel[0] = (uint8_t *)malloc((size_t)inFrameSizeY);
	SrcPix.pPixel[1] = (uint8_t *)malloc((size_t)inFrameSizeU);
	SrcPix.pPixel[2] = (uint8_t *)malloc((size_t)inFrameSizeV);

	DstPix_a->pPixel[0] = (uint8_t *)malloc((size_t)outFrameSizeY);
	DstPix_a->pPixel[1] = (uint8_t *)malloc((size_t)outFrameSizeU);
	DstPix_a->pPixel[2] = (uint8_t *)malloc((size_t)outFrameSizeV);

	DstPix_b->pPixel[0] = (uint8_t *)malloc((size_t)outFrameSizeY);
	DstPix_b->pPixel[1] = (uint8_t *)malloc((size_t)outFrameSizeU);
	DstPix_b->pPixel[2] = (uint8_t *)malloc((size_t)outFrameSizeV);

	//calculate down sample ratio
	ratio = getDownsampleRatio(inFrameSizeY, outFrameSizeY);
	if (ratio == RANDOM)
		compareTwoFuns = 0;
	else
		compareTwoFuns = 1;

	//open and parse IO files
	fIn = fopen(inFileName, "rb");
	if (fIn == NULL){
		fprintf(stderr, "can not open input file!\n");
		return -1;
	}

	fOutA = fopen(outFileNameA, "wb");
	if (fOutA == NULL){
		fprintf(stderr, "can not open output file for original function to downsample!\n");
		return -1;
	}

	flog = fopen("log.txt", "wb");
	if (flog == NULL){
		fprintf(stderr, "can not open log file!\n");
		return -1;
	}

#if 0
	fseek(fIn, 0L, SEEK_END);
	inFileSize = (int)ftell(fIn);

	if (inFileSize <= 0){
		fprintf(stderr, "Error: input file size error!\n");
		return -1;
	}
	fseek(fIn, 0L, SEEK_SET);
#endif

#if defined(COMPARETWOFUNCS)
	if (compareTwoFuns){
		fOutB = fopen(outFileNameB, "wb");
		if (fOutB == NULL){
			fprintf(stderr, "can not open output file for new function to downsample!\n");
			return -1;
		}
	}
#endif


	//generate log file
	fprintf(flog, "-----------------------------------------------------------\n");
	fprintf(flog, "Input Info:\n");
	fprintf(flog, "		filename: %s\n",inFileName);
	fprintf(flog, "		width: %d,	height: %d\n", SrcPix.width, SrcPix.height);
	fprintf(flog, "Output Info:\n");
	fprintf(flog, "		filenameA: %s\n", outFileNameA);
	fprintf(flog, "		filenameB: %s\n", outFileNameB);
	fprintf(flog, "		width: %d,	height: %d\n", DstPix.width, DstPix.height);
	fprintf(flog, "down sample ratio is 1:%d\n", ratio);
	fprintf(flog, "-----------------------------------------------------------\n");
	fprintf(flog, "\n\n\n");

	printf("downsample start!\n");

	index = getIndex[(int)ratio];

	while (1){
		//read input data
		size = fread(SrcPix.pPixel[0], 1, (size_t)inFrameSizeY, fIn);
		if (size <= 0){
			break;
		}

		size = fread(SrcPix.pPixel[1], 1, (size_t)inFrameSizeU, fIn);
		if (size <= 0){
			break;
		}

		size = fread(SrcPix.pPixel[2], 1, (size_t)inFrameSizeV, fIn);
		if (size <= 0){
			break;
		}

		//downsample using general function
		iStartTime_g = WelsTime();
		downsample_funs_c[index].pfGeneralRatioLuma(DstPix_a->pPixel[0], DstPix_a->width,
			DstPix_a->width, DstPix_a->height,
			SrcPix.pPixel[0], SrcPix.width, SrcPix.width, SrcPix.height);

		downsample_funs_c[index].pfGeneralRatioChroma(DstPix_a->pPixel[1], DstPix_a->width >> 1,
			DstPix_a->width >> 1, DstPix_a->height >> 1,
			SrcPix.pPixel[1], SrcPix.width >> 1, SrcPix.width >> 1, SrcPix.height >> 1);

		downsample_funs_c[index].pfGeneralRatioChroma(DstPix_a->pPixel[2], DstPix_a->width >> 1,
			DstPix_a->width >> 1, DstPix_a->height >> 1,
			SrcPix.pPixel[2], SrcPix.width >> 1, SrcPix.width >> 1, SrcPix.height >> 1);
		iTotalTime_g += WelsTime() - iStartTime_g;

#if  defined(COMPARETWOFUNCS)
		//downsample using specific function
		if (compareTwoFuns){
			iStartTime_s = WelsTime();
			downsample_funs_c[index].pfSpecificFunc(DstPix_b->pPixel[0], DstPix_b->width,
				SrcPix.pPixel[0], SrcPix.width, SrcPix.width, SrcPix.height);

			downsample_funs_c[index].pfSpecificFunc(DstPix_b->pPixel[1], DstPix_b->width >> 1,
				SrcPix.pPixel[1], SrcPix.width >> 1, SrcPix.width >> 1, SrcPix.height >> 1);

			downsample_funs_c[index].pfSpecificFunc(DstPix_b->pPixel[2], DstPix_b->width >> 1,
				SrcPix.pPixel[2], SrcPix.width >> 1, SrcPix.width >> 1, SrcPix.height >> 1);
			iTotalTime_s += WelsTime() - iStartTime_s;

			compareOutput(flog, DstPix_a->pPixel[0], DstPix_b->pPixel[0], DstPix_a->width, DstPix_a->height, frameNum, "Y");
			compareOutput(flog, DstPix_a->pPixel[1], DstPix_b->pPixel[1], DstPix_a->width >> 1, DstPix_a->height >> 1, frameNum, "U");
			compareOutput(flog, DstPix_a->pPixel[2], DstPix_b->pPixel[2], DstPix_a->width >> 1, DstPix_a->height >> 1, frameNum, "V");
		}

#endif
		//saving frame
		printf("\tsaving frame %d\r", frameNum++);
		fwrite(DstPix_a->pPixel[0], 1, (size_t)outFrameSizeY, fOutA);
		fwrite(DstPix_a->pPixel[1], 1, (size_t)outFrameSizeU, fOutA);
		fwrite(DstPix_a->pPixel[2], 1, (size_t)outFrameSizeV, fOutA);
#if defined(COMPARETWOFUNCS)
		if (compareTwoFuns){
			fwrite(DstPix_b->pPixel[0], 1, (size_t)outFrameSizeY, fOutB);
			fwrite(DstPix_b->pPixel[1], 1, (size_t)outFrameSizeU, fOutB);
			fwrite(DstPix_b->pPixel[2], 1, (size_t)outFrameSizeV, fOutB);
		}
#endif

	}

	free(SrcPix.pPixel[0]);
	free(SrcPix.pPixel[1]);
	free(SrcPix.pPixel[2]);

	free(DstPix_a->pPixel[0]);
	free(DstPix_a->pPixel[1]);
	free(DstPix_a->pPixel[2]);
	free(DstPix_b->pPixel[0]);
	free(DstPix_b->pPixel[1]);
	free(DstPix_b->pPixel[2]);

	//free(outFileNameA);
	//free(outFileNameB);

	fclose(fIn);
	fclose(fOutA);
	fclose(flog);
#if defined(COMPARETWOFUNCS)
	if (compareTwoFuns)
		fclose(fOutB);
#endif
	

	printf("\ndown sample end\n");

	// time cost
	iTotalTime_g = iTotalTime_g/1e6;
	iTotalTime_s = iTotalTime_s/1e6;
	printf("-------------------- Time cost  ----------------------\n");
	printf("General Function \n\t Total time:	%fsec => %fms/frame\n",iTotalTime_g, 1000 * iTotalTime_g/frameNum );
	printf("Specific Function \n\t Total time:	%fsec => %fms/frame\n",iTotalTime_s, 1000 * iTotalTime_s/frameNum );
	printf("------------------------------------------------------\n");

	return 0;
}

