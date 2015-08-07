//
//  downsample.h
//  
//
//  Created by Guangwei Wang on 6/27/15.
//
//

#ifndef ASM_DEV_WS_downsample_h
#define ASM_DEV_WS_downsample_h

#include <stdint.h>

typedef struct {
	uint8_t*     pPixel[3];
	int          iStride[3];
	int width;
	int height;
} sPixMap;

typedef void (SpecificDownsampleFunc)(uint8_t* pDst, const int32_t kiDstStride,
	uint8_t* pSrc, const int32_t kiSrcStride,
	const int32_t kiSrcWidth, const int32_t kiSrcHeight);

typedef void (GeneralDownsampleFunc)(uint8_t* pDst, const int32_t kiDstStride, const int32_t kiDstWidth,
	const int32_t kiDstHeight,
	uint8_t* pSrc, const int32_t kiSrcStride, const int32_t kiSrcWidth, const int32_t kiSrcHeight);

typedef SpecificDownsampleFunc*    PSpecificDownsampleFunc;
typedef GeneralDownsampleFunc*     PGeneralDownsampleFunc;

SpecificDownsampleFunc   DyadicBilinearDownsampler_c;
SpecificDownsampleFunc BilinearDownsamplerQuarter_c;
SpecificDownsampleFunc BilinearDownsamplerThird_c;
GeneralDownsampleFunc GeneralBilinearFastDownsampler_c;
GeneralDownsampleFunc GeneralBilinearAccurateDownsampler_c;

typedef struct {
	// align_index: 0 = x32; 1 = x16; 2 = x8; 3 = common case left;
	//PHalveDownsampleFunc          pfHalfAverage[4];
	PSpecificDownsampleFunc       pfSpecificFunc;
	PGeneralDownsampleFunc        pfGeneralRatioLuma;
	PGeneralDownsampleFunc        pfGeneralRatioChroma;
} SDownsampleFuncs;

#endif
