/*

PICCANTE
The hottest HDR imaging library!
http://vcg.isti.cnr.it/piccante

Copyright (C) 2014
Visual Computing Laboratory - ISTI CNR
http://vcg.isti.cnr.it
First author: Francesco Banterle

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

*/

#ifndef PIC_FILTERING_FILTER_DISPARITY_HPP
#define PIC_FILTERING_FILTER_DISPARITY_HPP

#include "../filtering/filter.hpp"

#include "../features_matching/patch_comp.hpp"

namespace pic {

/**
 * @brief The FilterDisparity class
 */
class FilterDisparity: public Filter
{
protected:

    int maxDisparity, halfMaxDisparity, patchSize;
    PatchComp *pc;

    /**
     * @brief ProcessBBox
     * @param dst
     * @param src
     * @param box
     */
    void ProcessBBox(Image *dst, ImageVec src, BBox *box)
    {

        if(pc == NULL) {
            return;
        }

        for(int j = box->y0; j < box->y1; j++) {

            for(int i = box->x0; i < box->x1; i++) {

                float *prevL = (*dst)(i - 1, j    );
                float *prevU = (*dst)(i    , j - 1);

                int xB = -1;
                float dB = FLT_MAX;

                int minX = MAX(i - halfMaxDisparity, 0);
                int maxX = MIN(i + halfMaxDisparity, src[1]->width);

                for(int x = minX; x < maxX; x++) {
                     pc->improveStereo(i, j, x, prevL, prevU, maxDisparity, xB, dB);
                }

                float *out = (*dst)(i, j);

                out[0] = float(xB - i);
                out[1] = dB;
            }
        }
    }

    /**
     * @brief SetupAux
     * @param imgIn
     * @param imgOut
     * @return
     */
    Image *SetupAux(ImageVec imgIn, Image *imgOut)
    {
        if(imgIn.size() == 2) {
            pc = new PatchComp(imgIn[0], imgIn[1], patchSize, 0.1f, 1.0f);
        } else {
            return NULL;
        }

        if(imgOut == NULL) {
            imgOut = new Image(1, imgIn[0]->width, imgIn[0]->height, 2);
        } else {
            if((imgIn[0]->width  != imgOut->width)  ||
               (imgIn[0]->height != imgOut->height) ||
               (imgOut->channels != 2)) {
                imgOut = new Image(1, imgIn[0]->width, imgIn[0]->height, 2);
            }
        }

        *imgOut = -1.0f;

        return imgOut;
    }

public:

    /**
     * @brief FilterDisparity
     * @param type
     */
    FilterDisparity(int maxDisparity, int patchSize) : Filter()
    {
        init(maxDisparity, patchSize);
    }

    ~FilterDisparity()
    {
        if(pc != NULL) {
            delete pc;
        }
    }

    /**
     * @brief init
     * @param maxDisparity
     */
    void init(int maxDisparity, int patchSize)
    {
        this->maxDisparity = maxDisparity;
        this->halfMaxDisparity = maxDisparity >> 1;
        this->patchSize = patchSize;
        pc = NULL;
    }

    /**
     * @brief OutputSize
     * @param imgIn
     * @param width
     * @param height
     * @param channels
     * @param frames
     */
    void OutputSize(Image *imgIn, int &width, int &height, int &channels, int &frames)
    {
        width       = imgIn->width;
        height      = imgIn->height;
        channels    = 2;
        frames      = imgIn->frames;
    }
};

} // end namespace pic

#endif /* PIC_FILTERING_FILTER_DISPARITY_HPP */

