/******************************************************************************
 *
 * Project:  GDAL
 * Purpose:  Implementation of a set of GDALDerivedPixelFunc(s) to be used
 *           with source raster band of virtual GDAL datasets.
 * Author:   Antonio Valentino <a_valentino@users.sf.net>
 *
 ******************************************************************************
 * Copyright (c) 2008-2011 Antonio Valentino
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *****************************************************************************/

#include <math.h>
#include <gdal.h>


CPLErr RealPixelFunc(void **papoSources, int nSources, void *pData,
                     int nXSize, int nYSize,
                     GDALDataType eSrcType, GDALDataType eBufType,
                     int nPixelSpace, int nLineSpace)
{
    int iLine, nPixelSpaceSrc, nLineSpaceSrc;

    /* ---- Init ---- */
    if (nSources != 1) return CE_Failure;

    nPixelSpaceSrc = GDALGetDataTypeSize( eSrcType ) / 8;
    nLineSpaceSrc = nPixelSpaceSrc * nXSize;

    /* ---- Set pixels ---- */
    for( iLine = 0; iLine < nYSize; ++iLine ) {
        GDALCopyWords(((GByte *)papoSources[0]) + nLineSpaceSrc * iLine,
                      eSrcType, nPixelSpaceSrc,
                      ((GByte *)pData) + nLineSpace * iLine,
                      eBufType, nPixelSpace, nXSize);
    }

    /* ---- Return success ---- */
    return CE_None;
} /* RealPixelFunc */


CPLErr ImagPixelFunc(void **papoSources, int nSources, void *pData,
                     int nXSize, int nYSize,
                     GDALDataType eSrcType, GDALDataType eBufType,
                     int nPixelSpace, int nLineSpace)
{
    int iLine;

    /* ---- Init ---- */
    if (nSources != 1) return CE_Failure;

    if (GDALDataTypeIsComplex( eSrcType ))
    {
        int nPixelSpaceSrc = GDALGetDataTypeSize( eSrcType ) / 8;
        int nLineSpaceSrc = nPixelSpaceSrc * nXSize;

        void* pImag = ((GByte *)papoSources[0])
                    + GDALGetDataTypeSize( eSrcType ) / 8 / 2;

        /* ---- Set pixels ---- */
        for( iLine = 0; iLine < nYSize; ++iLine ) {
            GDALCopyWords(((GByte *)pImag) + nLineSpaceSrc * iLine,
                          eSrcType, nPixelSpaceSrc,
                          ((GByte *)pData) + nLineSpace * iLine,
                          eBufType, nPixelSpace, nXSize);
        }
    } else {
        double dfImag = 0;

        /* ---- Set pixels ---- */
        for( iLine = 0; iLine < nYSize; ++iLine ) {
            /* always copy from the same location */
            GDALCopyWords(&dfImag, eSrcType, 0,
                          ((GByte *)pData) + nLineSpace * iLine,
                          eBufType, nPixelSpace, nXSize);
        }
    }

    /* ---- Return success ---- */
    return CE_None;
} /* ImagPixelFunc */


CPLErr ModulePixelFunc(void **papoSources, int nSources, void *pData,
                       int nXSize, int nYSize,
                       GDALDataType eSrcType, GDALDataType eBufType,
                       int nPixelSpace, int nLineSpace)
{
    int ii, iLine, iCol;
    double dfPixVal;

    /* ---- Init ---- */
    if (nSources != 1) return CE_Failure;

    if (GDALDataTypeIsComplex( eSrcType ))
    {
        double dfReal, dfImag;
        void *pReal = papoSources[0];
        void *pImag = ((GByte *)papoSources[0])
                    + GDALGetDataTypeSize( eSrcType ) / 8 / 2;

        /* ---- Set pixels ---- */
        for( iLine = 0, ii = 0; iLine < nYSize; ++iLine ) {
            for( iCol = 0; iCol < nXSize; ++iCol, ++ii ) {

                /* Source raster pixels may be obtained with SRCVAL macro */
                dfReal = SRCVAL(pReal, eSrcType, ii);
                dfImag = SRCVAL(pImag, eSrcType, ii);

                dfPixVal = sqrt( dfReal * dfReal + dfImag * dfImag );

                GDALCopyWords(&dfPixVal, GDT_Float64, 0,
                              ((GByte *)pData) + nLineSpace * iLine +
                              iCol * nPixelSpace, eBufType, nPixelSpace, 1);
            }
        }
    } else {
        /* ---- Set pixels ---- */
        for( iLine = 0, ii = 0; iLine < nYSize; ++iLine ) {
            for( iCol = 0; iCol < nXSize; ++iCol, ++ii ) {

                /* Source raster pixels may be obtained with SRCVAL macro */
                dfPixVal = abs(SRCVAL(papoSources[0], eSrcType, ii));

                GDALCopyWords(&dfPixVal, GDT_Float64, 0,
                              ((GByte *)pData) + nLineSpace * iLine +
                              iCol * nPixelSpace, eBufType, nPixelSpace, 1);
            }
        }
    }

    /* ---- Return success ---- */
    return CE_None;
} /* ModulePixelFunc */


CPLErr PhasePixelFunc(void **papoSources, int nSources, void *pData,
                      int nXSize, int nYSize,
                      GDALDataType eSrcType, GDALDataType eBufType,
                      int nPixelSpace, int nLineSpace)
{
    int ii, iLine, iCol;
    double dfPixVal, dfReal;

    /* ---- Init ---- */
    if (nSources != 1) return CE_Failure;

    if (GDALDataTypeIsComplex( eSrcType ))
    {
        double dfImag;
        void *pReal = papoSources[0];
        void *pImag = ((GByte *)papoSources[0])
                    + GDALGetDataTypeSize( eSrcType ) / 8 / 2;

        /* ---- Set pixels ---- */
        for( iLine = 0, ii= 0; iLine < nYSize; ++iLine ) {
            for( iCol = 0; iCol < nXSize; ++iCol, ++ii ) {

                /* Source raster pixels may be obtained with SRCVAL macro */
                dfReal = SRCVAL(pReal, eSrcType, ii);
                dfImag = SRCVAL(pImag, eSrcType, ii);

                dfPixVal = atan2(dfImag, dfReal);

                GDALCopyWords(&dfPixVal, GDT_Float64, 0,
                          ((GByte *)pData) + nLineSpace * iLine +
                          iCol * nPixelSpace, eBufType, nPixelSpace, 1);
            }
        }
    } else {
        /* ---- Set pixels ---- */
        /*
        for( iLine = 0; iLine < nYSize; ++iLine ) {
            / * always copy from the same location * /
            GDALCopyWords(&dfImag, eSrcType, 0,
                          ((GByte *)pData) + nLineSpace * iLine,
                          eBufType, nPixelSpace, nXSize);
        }
        */
        /* ---- Set pixels ---- */
        double pi = atan2(0, -1);
        for( iLine = 0, ii= 0; iLine < nYSize; ++iLine ) {
            for( iCol = 0; iCol < nXSize; ++iCol, ++ii ) {

                void *pReal = papoSources[0];

                /* Source raster pixels may be obtained with SRCVAL macro */
                dfReal = SRCVAL(pReal, eSrcType, ii);
                dfPixVal = (dfReal < 0) ? pi : 0;

                GDALCopyWords(&dfPixVal, GDT_Float64, dfPixVal,
                          ((GByte *)pData) + nLineSpace * iLine +
                          iCol * nPixelSpace, eBufType, nPixelSpace, 1);
            }
        }
    }

    /* ---- Return success ---- */
    return CE_None;
} /* PhasePixelFunc */


CPLErr ConjPixelFunc(void **papoSources, int nSources, void *pData,
                     int nXSize, int nYSize,
                     GDALDataType eSrcType, GDALDataType eBufType,
                     int nPixelSpace, int nLineSpace)
{
    /* ---- Init ---- */
    if (nSources != 1) return CE_Failure;

    if (GDALDataTypeIsComplex( eSrcType ) && GDALDataTypeIsComplex( eBufType ))
    {
        int iLine, iCol, ii;
        double adfPixVal[2];
        int nOffset = GDALGetDataTypeSize( eSrcType ) / 8 / 2;
        void *pReal = papoSources[0];
        void *pImag = ((GByte *)papoSources[0]) + nOffset;

        /* ---- Set pixels ---- */
        for( iLine = 0, ii= 0; iLine < nYSize; ++iLine ) {
            for( iCol = 0; iCol < nXSize; ++iCol, ++ii ) {

                /* Source raster pixels may be obtained with SRCVAL macro */
                adfPixVal[0] = +SRCVAL(pReal, eSrcType, ii); /* re */
                adfPixVal[1] = -SRCVAL(pImag, eSrcType, ii); /* im */

                GDALCopyWords(adfPixVal, GDT_CFloat64, 0,
                              ((GByte *)pData) + nLineSpace * iLine +
                              iCol * nPixelSpace, eBufType, nPixelSpace, 1);
            }
        }
    } else {
        /* no complex data type */
        return RealPixelFunc(papoSources, nSources, pData, nXSize, nYSize,
                             eSrcType, eBufType, nPixelSpace, nLineSpace);
    }

    /* ---- Return success ---- */
    return CE_None;
} /* ConjPixelFunc */


CPLErr SumPixelFunc(void **papoSources, int nSources, void *pData,
                    int nXSize, int nYSize,
                    GDALDataType eSrcType, GDALDataType eBufType,
                    int nPixelSpace, int nLineSpace)
{
    int iLine, iCol, ii, iSrc;

    /* ---- Init ---- */
    if (nSources < 2) return CE_Failure;

    /* ---- Set pixels ---- */
    if (GDALDataTypeIsComplex( eSrcType ))
    {
        double adfSum[2];
        void *pReal, *pImag;
        int nOffset = GDALGetDataTypeSize( eSrcType ) / 8 / 2;

        /* ---- Set pixels ---- */
        for( iLine = 0, ii= 0; iLine < nYSize; ++iLine ) {
            for( iCol = 0; iCol < nXSize; ++iCol, ++ii ) {
                adfSum[0] = 0;
                adfSum[1] = 0;

                for( iSrc = 0; iSrc < nSources; ++iSrc ) {
                    pReal = papoSources[iSrc];
                    pImag = ((GByte *)pReal) + nOffset;

                    /* Source raster pixels may be obtained with SRCVAL macro */
                    adfSum[0] += SRCVAL(pReal, eSrcType, ii);
                    adfSum[1] += SRCVAL(pImag, eSrcType, ii);
                }

                GDALCopyWords(adfSum, GDT_CFloat64, 0,
                              ((GByte *)pData) + nLineSpace * iLine +
                              iCol * nPixelSpace, eBufType, nPixelSpace, 1);
            }
        }
    } else {
        /* non complex */
        double dfSum;

        /* ---- Set pixels ---- */
        for( iLine = 0, ii= 0; iLine < nYSize; ++iLine ) {
            for( iCol = 0; iCol < nXSize; ++iCol, ++ii ) {
                dfSum = 0;

                for( iSrc = 0; iSrc < nSources; ++iSrc ) {
                    /* Source raster pixels may be obtained with SRCVAL macro */
                    dfSum += SRCVAL(papoSources[iSrc], eSrcType, ii);
                }

                GDALCopyWords(&dfSum, GDT_Float64, 0,
                              ((GByte *)pData) + nLineSpace * iLine +
                              iCol * nPixelSpace, eBufType, nPixelSpace, 1);
            }
        }
    }

    /* ---- Return success ---- */
    return CE_None;
} /* SumPixelFunc */


CPLErr DiffPixelFunc(void **papoSources, int nSources, void *pData,
                     int nXSize, int nYSize,
                     GDALDataType eSrcType, GDALDataType eBufType,
                     int nPixelSpace, int nLineSpace)
{
    int ii, iLine, iCol;

    /* ---- Init ---- */
    if (nSources != 2) return CE_Failure;

    if (GDALDataTypeIsComplex( eSrcType ))
    {

        double adfPixVal[2];
        int nOffset = GDALGetDataTypeSize( eSrcType ) / 8 / 2;
        void *pReal0 = papoSources[0];
        void *pImag0 = ((GByte *)papoSources[0]) + nOffset;
        void *pReal1 = papoSources[1];
        void *pImag1 = ((GByte *)papoSources[1]) + nOffset;

        /* ---- Set pixels ---- */
        for( iLine = 0, ii= 0; iLine < nYSize; ++iLine ) {
            for( iCol = 0; iCol < nXSize; ++iCol, ++ii ) {

                /* Source raster pixels may be obtained with SRCVAL macro */
                adfPixVal[0] = SRCVAL(pReal0, eSrcType, ii)
                             - SRCVAL(pReal1, eSrcType, ii);
                adfPixVal[1] = SRCVAL(pImag0, eSrcType, ii)
                             - SRCVAL(pImag1, eSrcType, ii);

                GDALCopyWords(adfPixVal, GDT_CFloat64, 0,
                              ((GByte *)pData) + nLineSpace * iLine +
                              iCol * nPixelSpace, eBufType, nPixelSpace, 1);
            }
        }
    } else {
        /* non complex */
        double dfPixVal;

        /* ---- Set pixels ---- */
        for( iLine = 0, ii= 0; iLine < nYSize; ++iLine ) {
            for( iCol = 0; iCol < nXSize; ++iCol, ++ii ) {

                /* Source raster pixels may be obtained with SRCVAL macro */
                dfPixVal = SRCVAL(papoSources[0], eSrcType, ii)
                         - SRCVAL(papoSources[1], eSrcType, ii);

                GDALCopyWords(&dfPixVal, GDT_Float64, 0,
                              ((GByte *)pData) + nLineSpace * iLine +
                              iCol * nPixelSpace, eBufType, nPixelSpace, 1);
            }
        }
    }

    /* ---- Return success ---- */
    return CE_None;
} /* DiffPixelFunc */


CPLErr MulPixelFunc(void **papoSources, int nSources, void *pData,
                    int nXSize, int nYSize,
                    GDALDataType eSrcType, GDALDataType eBufType,
                    int nPixelSpace, int nLineSpace)
{
    int iLine, iCol, ii, iSrc;

    /* ---- Init ---- */
    if (nSources < 2) return CE_Failure;

    /* ---- Set pixels ---- */
    if (GDALDataTypeIsComplex( eSrcType ))
    {
        double adfPixVal[2], dfOldR, dfOldI, dfNewR, dfNewI;
        void *pReal, *pImag;
        int nOffset = GDALGetDataTypeSize( eSrcType ) / 8 / 2;

        /* ---- Set pixels ---- */
        for( iLine = 0, ii= 0; iLine < nYSize; ++iLine ) {
            for( iCol = 0; iCol < nXSize; ++iCol, ++ii ) {
                adfPixVal[0] = 1.;
                adfPixVal[1] = 0.;

                for( iSrc = 0; iSrc < nSources; ++iSrc ) {
                    pReal = papoSources[iSrc];
                    pImag = ((GByte *)pReal) + nOffset;

                    dfOldR = adfPixVal[0];
                    dfOldI = adfPixVal[1];

                    /* Source raster pixels may be obtained with SRCVAL macro */
                    dfNewR = SRCVAL(pReal, eSrcType, ii);
                    dfNewI = SRCVAL(pImag, eSrcType, ii);

                    adfPixVal[0] = dfOldR * dfNewR - dfOldI * dfNewI;
                    adfPixVal[1] = dfOldR * dfNewI + dfOldI * dfNewR;
                }

                GDALCopyWords(adfPixVal, GDT_CFloat64, 0,
                              ((GByte *)pData) + nLineSpace * iLine +
                              iCol * nPixelSpace, eBufType, nPixelSpace, 1);
            }
        }
    } else {
        /* non complex */
        double dfPixVal;

        /* ---- Set pixels ---- */
        for( iLine = 0, ii= 0; iLine < nYSize; ++iLine ) {
            for( iCol = 0; iCol < nXSize; ++iCol, ++ii ) {
                dfPixVal = 1;

                for( iSrc = 0; iSrc < nSources; ++iSrc ) {
                    /* Source raster pixels may be obtained with SRCVAL macro */
                    dfPixVal *= SRCVAL(papoSources[iSrc], eSrcType, ii);
                }

                GDALCopyWords(&dfPixVal, GDT_Float64, 0,
                              ((GByte *)pData) + nLineSpace * iLine +
                              iCol * nPixelSpace, eBufType, nPixelSpace, 1);
            }
        }
    }

    /* ---- Return success ---- */
    return CE_None;
} /* MulPixelFunc */


CPLErr CMulPixelFunc(void **papoSources, int nSources, void *pData,
                     int nXSize, int nYSize,
                     GDALDataType eSrcType, GDALDataType eBufType,
                     int nPixelSpace, int nLineSpace)
{
    int ii, iLine, iCol;

    /* ---- Init ---- */
    if (nSources != 2) return CE_Failure;

    /* ---- Set pixels ---- */
    if (GDALDataTypeIsComplex( eSrcType ))
    {
        double adfPixVal[2], dfReal0, dfImag0, dfReal1, dfImag1;

        int nOffset = GDALGetDataTypeSize( eSrcType ) / 8 / 2;
        void *pReal0 = papoSources[0];
        void *pImag0 = ((GByte *)papoSources[0]) + nOffset;
        void *pReal1 = papoSources[1];
        void *pImag1 = ((GByte *)papoSources[1]) + nOffset;

        for( iLine = 0, ii= 0; iLine < nYSize; ++iLine ) {
            for( iCol = 0; iCol < nXSize; ++iCol, ++ii ) {

                /* Source raster pixels may be obtained with SRCVAL macro */
                dfReal0 = SRCVAL(pReal0, eSrcType, ii);
                dfReal1 = SRCVAL(pReal1, eSrcType, ii);
                dfImag0 = SRCVAL(pImag0, eSrcType, ii);
                dfImag1 = SRCVAL(pImag1, eSrcType, ii);
                adfPixVal[0]  = dfReal0 * dfReal1 + dfImag0 * dfImag1;
                adfPixVal[1]  = dfReal1 * dfImag0 - dfReal0 * dfImag1;

                GDALCopyWords(adfPixVal, GDT_CFloat64, 0,
                              ((GByte *)pData) + nLineSpace * iLine +
                              iCol * nPixelSpace, eBufType, nPixelSpace, 1);
            }
        }
    } else {
        /* non complex */
        double adfPixVal[2] = {0, 0};

        for( iLine = 0, ii= 0; iLine < nYSize; ++iLine ) {
            for( iCol = 0; iCol < nXSize; ++iCol, ++ii ) {

                /* Source raster pixels may be obtained with SRCVAL macro */
                adfPixVal[0] = SRCVAL(papoSources[0], eSrcType, ii)
                             * SRCVAL(papoSources[1], eSrcType, ii);

                GDALCopyWords(adfPixVal, GDT_CFloat64, 0,
                              ((GByte *)pData) + nLineSpace * iLine +
                              iCol * nPixelSpace, eBufType, nPixelSpace, 1);
            }
        }
    }

    /* ---- Return success ---- */
    return CE_None;
} /* CMulPixelFunc */


CPLErr InvPixelFunc(void **papoSources, int nSources, void *pData,
                    int nXSize, int nYSize,
                    GDALDataType eSrcType, GDALDataType eBufType,
                    int nPixelSpace, int nLineSpace)
{
    int iLine, iCol, ii;

    /* ---- Init ---- */
    if (nSources != 1) return CE_Failure;

    /* ---- Set pixels ---- */
    if (GDALDataTypeIsComplex( eSrcType ))
    {
        double adfPixVal[2], dfReal, dfImag, dfAux;

        int nOffset = GDALGetDataTypeSize( eSrcType ) / 8 / 2;
        void *pReal = papoSources[0];
        void *pImag = ((GByte *)papoSources[0]) + nOffset;

        for( iLine = 0, ii= 0; iLine < nYSize; ++iLine ) {
            for( iCol = 0; iCol < nXSize; ++iCol, ++ii ) {

                /* Source raster pixels may be obtained with SRCVAL macro */
                dfReal = SRCVAL(pReal, eSrcType, ii);
                dfImag = SRCVAL(pImag, eSrcType, ii);
                dfAux = dfReal * dfReal + dfImag * dfImag;
                adfPixVal[0]  = +dfReal / dfAux;
                adfPixVal[1]  = -dfImag / dfAux;

                GDALCopyWords(adfPixVal, GDT_CFloat64, 0,
                              ((GByte *)pData) + nLineSpace * iLine +
                              iCol * nPixelSpace, eBufType, nPixelSpace, 1);
            }
        }
    } else {
        /* non complex */
        double dfPixVal;

        /* ---- Set pixels ---- */
        for( iLine = 0, ii= 0; iLine < nYSize; ++iLine ) {
            for( iCol = 0; iCol < nXSize; ++iCol, ++ii ) {

                /* Source raster pixels may be obtained with SRCVAL macro */
                dfPixVal = 1. / SRCVAL(papoSources[0], eSrcType, ii);

                GDALCopyWords(&dfPixVal, GDT_Float64, 0,
                              ((GByte *)pData) + nLineSpace * iLine +
                              iCol * nPixelSpace, eBufType, nPixelSpace, 1);
            }
        }
    }

    /* ---- Return success ---- */
    return CE_None;
} /* InvPixelFunc */


CPLErr IntensityPixelFunc(void **papoSources, int nSources, void *pData,
                          int nXSize, int nYSize,
                          GDALDataType eSrcType, GDALDataType eBufType,
                          int nPixelSpace, int nLineSpace)
{
    int ii, iLine, iCol;
    double dfPixVal;

    /* ---- Init ---- */
    if (nSources != 1) return CE_Failure;

    if (GDALDataTypeIsComplex( eSrcType ))
    {
        double dfReal, dfImag;
        int nOffset = GDALGetDataTypeSize( eSrcType ) / 8 / 2;
        void *pReal = papoSources[0];
        void *pImag = ((GByte *)papoSources[0]) + nOffset;

        /* ---- Set pixels ---- */
        for( iLine = 0, ii = 0; iLine < nYSize; ++iLine ) {
            for( iCol = 0; iCol < nXSize; ++iCol, ++ii ) {

                /* Source raster pixels may be obtained with SRCVAL macro */
                dfReal = SRCVAL(pReal, eSrcType, ii);
                dfImag = SRCVAL(pImag, eSrcType, ii);

                dfPixVal = dfReal * dfReal + dfImag * dfImag;

                GDALCopyWords(&dfPixVal, GDT_Float64, 0,
                              ((GByte *)pData) + nLineSpace * iLine +
                              iCol * nPixelSpace, eBufType, nPixelSpace, 1);
            }
        }
    } else {
        /* ---- Set pixels ---- */
        for( iLine = 0, ii = 0; iLine < nYSize; ++iLine ) {
            for( iCol = 0; iCol < nXSize; ++iCol, ++ii ) {

                /* Source raster pixels may be obtained with SRCVAL macro */
                dfPixVal = SRCVAL(papoSources[0], eSrcType, ii);
                dfPixVal *= dfPixVal;

                GDALCopyWords(&dfPixVal, GDT_Float64, 0,
                              ((GByte *)pData) + nLineSpace * iLine +
                              iCol * nPixelSpace, eBufType, nPixelSpace, 1);
            }
        }
    }

    /* ---- Return success ---- */
    return CE_None;
} /* IntensityPixelFunc */


CPLErr SqrtPixelFunc(void **papoSources, int nSources, void *pData,
                     int nXSize, int nYSize,
                     GDALDataType eSrcType, GDALDataType eBufType,
                     int nPixelSpace, int nLineSpace)
{
    int iLine, iCol, ii;
    double dfPixVal;

    /* ---- Init ---- */
    if (nSources != 1) return CE_Failure;
    if (GDALDataTypeIsComplex( eSrcType )) return CE_Failure;

    /* ---- Set pixels ---- */
    for( iLine = 0, ii= 0; iLine < nYSize; ++iLine ) {
        for( iCol = 0; iCol < nXSize; ++iCol, ++ii ) {

            /* Source raster pixels may be obtained with SRCVAL macro */;
            dfPixVal = sqrt( SRCVAL(papoSources[0], eSrcType, ii) );

            GDALCopyWords(&dfPixVal, GDT_Float64, 0,
                          ((GByte *)pData) + nLineSpace * iLine +
                          iCol * nPixelSpace, eBufType, nPixelSpace, 1);
        }
    }

    /* ---- Return success ---- */
    return CE_None;
} /* SqrtPixelFunc */


CPLErr Log10PixelFunc(void **papoSources, int nSources, void *pData,
                      int nXSize, int nYSize,
                      GDALDataType eSrcType, GDALDataType eBufType,
                      int nPixelSpace, int nLineSpace)
{
    int ii, iLine, iCol;

    /* ---- Init ---- */
    if (nSources != 1) return CE_Failure;

    if (GDALDataTypeIsComplex( eSrcType ))
    {
        /* complex input datatype */
        double dfReal, dfImag, dfPixVal;
        int nOffset = GDALGetDataTypeSize( eSrcType ) / 8 / 2;
        void *pReal = papoSources[0];
        void *pImag = ((GByte *)papoSources[0]) + nOffset;

        /* ---- Set pixels ---- */
        for( iLine = 0, ii = 0; iLine < nYSize; ++iLine ) {
            for( iCol = 0; iCol < nXSize; ++iCol, ++ii ) {

                /* Source raster pixels may be obtained with SRCVAL macro */
                dfReal = SRCVAL(pReal, eSrcType, ii);
                dfImag = SRCVAL(pImag, eSrcType, ii);

                dfPixVal = log10( dfReal * dfReal + dfImag * dfImag );

                GDALCopyWords(&dfPixVal, GDT_Float64, 0,
                              ((GByte *)pData) + nLineSpace * iLine +
                              iCol * nPixelSpace, eBufType, nPixelSpace, 1);
            }
        }
    } else {
        double dfPixVal;

        /* ---- Set pixels ---- */
        for( iLine = 0, ii = 0; iLine < nYSize; ++iLine ) {
            for( iCol = 0; iCol < nXSize; ++iCol, ++ii ) {

                /* Source raster pixels may be obtained with SRCVAL macro */
                dfPixVal = SRCVAL(papoSources[0], eSrcType, ii);
                dfPixVal = log10( abs( dfPixVal ) );

                GDALCopyWords(&dfPixVal, GDT_Float64, 0,
                              ((GByte *)pData) + nLineSpace * iLine +
                              iCol * nPixelSpace, eBufType, nPixelSpace, 1);
            }
        }
    }

    /* ---- Return success ---- */
    return CE_None;
} /* Log10PixelFunc */


CPLErr PowPixelFuncHelper(void **papoSources, int nSources, void *pData,
                          int nXSize, int nYSize,
                          GDALDataType eSrcType, GDALDataType eBufType,
                          int nPixelSpace, int nLineSpace,
                          double base, double fact)
{
    int iLine, iCol, ii;
    double dfPixVal;

    /* ---- Init ---- */
    if (nSources != 1) return CE_Failure;
    if (GDALDataTypeIsComplex( eSrcType )) return CE_Failure;

    /* ---- Set pixels ---- */
    for( iLine = 0, ii= 0; iLine < nYSize; ++iLine ) {
        for( iCol = 0; iCol < nXSize; ++iCol, ++ii ) {

            /* Source raster pixels may be obtained with SRCVAL macro */
            dfPixVal = SRCVAL(papoSources[0], eSrcType, ii);
            dfPixVal = pow(base, dfPixVal / fact);

            GDALCopyWords(&dfPixVal, GDT_Float64, 0,
                          ((GByte *)pData) + nLineSpace * iLine +
                          iCol * nPixelSpace, eBufType, nPixelSpace, 1);
        }
    }

    /* ---- Return success ---- */
    return CE_None;
} /* PowPixelFuncHelper */

CPLErr dB2AmpPixelFunc(void **papoSources, int nSources, void *pData,
                       int nXSize, int nYSize,
                       GDALDataType eSrcType, GDALDataType eBufType,
                       int nPixelSpace, int nLineSpace)
{
    return PowPixelFuncHelper(papoSources, nSources, pData,
                              nXSize, nYSize, eSrcType, eBufType,
                              nPixelSpace, nLineSpace, 10., 20.);
} /* dB2AmpPixelFunc */


CPLErr dB2PowPixelFunc(void **papoSources, int nSources, void *pData,
                       int nXSize, int nYSize,
                       GDALDataType eSrcType, GDALDataType eBufType,
                       int nPixelSpace, int nLineSpace)
{
    return PowPixelFuncHelper(papoSources, nSources, pData,
                              nXSize, nYSize, eSrcType, eBufType,
                              nPixelSpace, nLineSpace, 10., 10.);
} /* dB2PowPixelFunc */

/************************************************************************/
/*                     Nansat pixelfunctions                            */
/************************************************************************/

CPLErr BetaSigmaToIncidence(void **papoSources, int nSources, void *pData,
        int nXSize, int nYSize,
        GDALDataType eSrcType, GDALDataType eBufType,
        int nPixelSpace, int nLineSpace)
{
	int ii, iLine, iCol;
	double incidence;
	double beta0, sigma0;
		
	/* ---- Init ---- */
	if (nSources != 2) return CE_Failure;
	#define PI 3.14159265;
	
	/* ---- Set pixels ---- */
	for( iLine = 0; iLine < nYSize; iLine++ )
	{
		for( iCol = 0; iCol < nXSize; iCol++ )
		{
			ii = iLine * nXSize + iCol;
			/* Source raster pixels may be obtained with SRCVAL macro */
			beta0 = SRCVAL(papoSources[0], eSrcType, ii);
			sigma0 = SRCVAL(papoSources[1], eSrcType, ii);
			
			if (beta0 != 0) incidence = asin(sigma0/beta0)*180/PI
			else incidence = 0;
			
			GDALCopyWords(&incidence, GDT_Float64, 0,
			              ((GByte *)pData) + nLineSpace * iLine + iCol * nPixelSpace,
			              eBufType, nPixelSpace, 1);
		}
	}
	
	/* ---- Return success ---- */
return CE_None;
}


CPLErr UVToMagnitude(void **papoSources, int nSources, void *pData,
        int nXSize, int nYSize,
        GDALDataType eSrcType, GDALDataType eBufType,
        int nPixelSpace, int nLineSpace)
{
	int ii, iLine, iCol;
	double magnitude;
	double u, v;
		
	/* ---- Init ---- */
	if (nSources != 2) return CE_Failure;
	
	/* ---- Set pixels ---- */
	for( iLine = 0; iLine < nYSize; iLine++ )
	{
		for( iCol = 0; iCol < nXSize; iCol++ )
		{
			ii = iLine * nXSize + iCol;
			/* Source raster pixels may be obtained with SRCVAL macro */
			u = SRCVAL(papoSources[0], eSrcType, ii);
			v = SRCVAL(papoSources[1], eSrcType, ii);
			
			magnitude = sqrt(u*u + v*v);
			
			GDALCopyWords(&magnitude, GDT_Float64, 0,
			              ((GByte *)pData) + nLineSpace * iLine + iCol * nPixelSpace,
			              eBufType, nPixelSpace, 1);
		}
	}
	
	/* ---- Return success ---- */
return CE_None;
}


CPLErr UVToDirectionTo(void **papoSources, int nSources, void *pData,
        int nXSize, int nYSize,
        GDALDataType eSrcType, GDALDataType eBufType,
        int nPixelSpace, int nLineSpace)
{
	int ii, iLine, iCol;
	double winddir_to;
	double u, v;
	#define PI 3.14159265;
	#define offset 180.
		
	/* ---- Init ---- */
	if (nSources != 2) return CE_Failure;
	
	/* ---- Set pixels ---- */
	for( iLine = 0; iLine < nYSize; iLine++ )
	{
		for( iCol = 0; iCol < nXSize; iCol++ )
		{
			ii = iLine * nXSize + iCol;
			/* Source raster pixels may be obtained with SRCVAL macro */
			u = SRCVAL(papoSources[0], eSrcType, ii);
			v = SRCVAL(papoSources[1], eSrcType, ii);
			
			winddir_to = atan2(-u,-v)*180/PI; /* Convention 0-360 degrees */
			winddir_to = winddir_to + 180;    /* Convention 0-360 degrees */
			
			GDALCopyWords(&winddir_to, GDT_Float64, 0,
			              ((GByte *)pData) + nLineSpace * iLine + iCol * nPixelSpace,
			              eBufType, nPixelSpace, 1);
		}
	}
	
	/* ---- Return success ---- */
return CE_None;
}

CPLErr UVToDirectionFrom(void **papoSources, int nSources, void *pData,
        int nXSize, int nYSize,
        GDALDataType eSrcType, GDALDataType eBufType,
        int nPixelSpace, int nLineSpace)
{
	int ii, iLine, iCol;
	double winddir_from;
	double u, v;
	#define PI 3.14159265;
		
	/* ---- Init ---- */
	if (nSources != 2) return CE_Failure;
	
	/* ---- Set pixels ---- */
	for( iLine = 0; iLine < nYSize; iLine++ )
	{
		for( iCol = 0; iCol < nXSize; iCol++ )
		{
			ii = iLine * nXSize + iCol;
			/* Source raster pixels may be obtained with SRCVAL macro */
			u = SRCVAL(papoSources[0], eSrcType, ii);
			v = SRCVAL(papoSources[1], eSrcType, ii);
			
			winddir_from = atan2(u,v)*180/PI; 	/* Convention 0-360 degrees */
			winddir_from = winddir_from + 180;  /* Convention 0-360 degrees */

			GDALCopyWords(&winddir_from, GDT_Float64, 0,
			              ((GByte *)pData) + nLineSpace * iLine + iCol * nPixelSpace,
			              eBufType, nPixelSpace, 1);
		}
	}
	
	/* ---- Return success ---- */
return CE_None;
}


CPLErr Sigma0HHIncidenceToSigma0VV(void **papoSources, int nSources, void *pData,
        int nXSize, int nYSize,
        GDALDataType eSrcType, GDALDataType eBufType,
        int nPixelSpace, int nLineSpace)
{
	int ii, iLine, iCol;
	double sigma0VV, sigma0HH, incidence_angle, factor;
	#define PI 3.14159265;
		
	/* ---- Init ---- */
	if (nSources != 2) return CE_Failure;
	
	/* ---- Set pixels ---- */
	for( iLine = 0; iLine < nYSize; iLine++ )
	{
		for( iCol = 0; iCol < nXSize; iCol++ )
		{
			ii = iLine * nXSize + iCol;
			/* Source raster pixels may be obtained with SRCVAL macro */
			sigma0HH = SRCVAL(papoSources[0], eSrcType, ii);
			incidence_angle = SRCVAL(papoSources[1], eSrcType, ii)*PI;
			incidence_angle = incidence_angle/180;
			
			/* Polarisation ratio from Thompson et al. with alpha=0.6 */
			factor = pow( (1+2* pow(tan(incidence_angle),2)) / (1+0.6*pow(tan(incidence_angle),2)), 2); 
			sigma0VV = sigma0HH*factor;

			GDALCopyWords(&sigma0VV, GDT_Float64, 0,
			              ((GByte *)pData) + nLineSpace * iLine + iCol * nPixelSpace,
			              eBufType, nPixelSpace, 1);
		}
	}
	
	/* ---- Return success ---- */
return CE_None;
}



/************************************************************************/
/*                     GDALRegisterDefaultPixelFunc()                   */
/************************************************************************/

/**
 * This adds a default set of pixel functions to the global list of
 * available pixel functions for derived bands:
 *
 * - "real": extract real part from a single raster band (just a copy if the
 *           input is non-complex)
 * - "imag": extract imaginary part from a single raster band (0 for
 *           non-complex)
 * - "mod": extract module from a single raster band (real or complex)
 * - "phase": extract phase from a single raster band (0 for non-complex)
 * - "conj": computes the complex conjugate of a single raster band (just a
 *           copy if the input is non-complex)
 * - "sum": sum 2 or more raster bands
 * - "diff": computes the difference between 2 raster bands (b1 - b2)
 * - "mul": multilpy 2 or more raster bands
 * - "cmul": multiply the first band for the complex comjugate of the second
 * - "inv": inverse (1./x). Note: no check is performed on zero division
 * - "intensity": computes the intensity Re(x*conj(x)) of a single raster band
 *                (real or complex)
 * - "sqrt": perform the square root of a single raster band (real only)
 * - "log10": compute the logarithm (base 10) of the abs of a single raster
 *            band (real or complex): log10( abs( x ) )
 * - "dB2amp": perform scale conversion from logarithmic to linear
 *             (amplitude) (i.e. 10 ^ ( x / 20 ) ) of a single raster
 *                 band (real only)
 * - "dB2pow": perform scale conversion from logarithmic to linear
 *             (power) (i.e. 10 ^ ( x / 10 ) ) of a single raster
 *             band (real only)
 *
 * @see GDALAddDerivedBandPixelFunc
 *
 * @return CE_None, invalid (NULL) parameters are currently ignored.
 */
CPLErr CPL_STDCALL GDALRegisterDefaultPixelFunc()
{
    GDALAddDerivedBandPixelFunc("real", RealPixelFunc);
    GDALAddDerivedBandPixelFunc("imag", ImagPixelFunc);
    GDALAddDerivedBandPixelFunc("mod", ModulePixelFunc);
    GDALAddDerivedBandPixelFunc("phase", PhasePixelFunc);
    GDALAddDerivedBandPixelFunc("conj", ConjPixelFunc);
    GDALAddDerivedBandPixelFunc("sum", SumPixelFunc);
    GDALAddDerivedBandPixelFunc("diff", DiffPixelFunc);
    GDALAddDerivedBandPixelFunc("mul", MulPixelFunc);
    GDALAddDerivedBandPixelFunc("cmul", CMulPixelFunc);
    GDALAddDerivedBandPixelFunc("inv", InvPixelFunc);
    GDALAddDerivedBandPixelFunc("intensity", IntensityPixelFunc);
    GDALAddDerivedBandPixelFunc("sqrt", SqrtPixelFunc);
    GDALAddDerivedBandPixelFunc("log10", Log10PixelFunc);
    GDALAddDerivedBandPixelFunc("dB2amp", dB2AmpPixelFunc);
    GDALAddDerivedBandPixelFunc("dB2pow", dB2PowPixelFunc);

    GDALAddDerivedBandPixelFunc("BetaSigmaToIncidence", BetaSigmaToIncidence);
    GDALAddDerivedBandPixelFunc("UVToMagnitude", UVToMagnitude);
    GDALAddDerivedBandPixelFunc("UVToDirectionTo", UVToDirectionTo);
    GDALAddDerivedBandPixelFunc("UVToDirectionFrom", UVToDirectionFrom);
    GDALAddDerivedBandPixelFunc("Sigma0HHIncidenceToSigma0VV", Sigma0HHIncidenceToSigma0VV);
    
    return CE_None;
}