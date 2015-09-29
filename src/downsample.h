/*!
 * \copy
 *     Copyright (c)  2011-2013, Cisco Systems
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
 * \file        :  downsample.h
 *
 * \brief       :  downsample class of wels video processor class
 *
 * \date        :  2011/03/33
 *
 * \description :  1. rewrite the package code of downsample class
 *
 *************************************************************************************
 */
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
SpecificDownsampleFunc BilinearDownsamplerOneThird_c;
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
