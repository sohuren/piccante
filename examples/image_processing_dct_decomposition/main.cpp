/*

PICCANTE Examples
The hottest examples of Piccante:
http://vcg.isti.cnr.it/piccante

Copyright (C) 2014
Visual Computing Laboratory - ISTI CNR
http://vcg.isti.cnr.it
First author: Francesco Banterle

This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3.0 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    See the GNU Lesser General Public License
    ( http://www.gnu.org/licenses/lgpl-3.0.html ) for more details.
*/

//This means that we disable Eigen; some functionalities cannot be used.
//For example, estimating the camera response function
#define PIC_DISABLE_EIGEN

//This means that OpenGL acceleration layer is disabled
#define PIC_DISABLE_OPENGL

#include "piccante.hpp"

int main(int argc, char *argv[])
{
    std::string img_str;

    if(argc == 2) {
        img_str = argv[1];
    } else {
        img_str = "../data/input/singapore.png";
    }

    printf("Reading an LDR file...");

    pic::Image img;
    img.Read(img_str);

    printf("Ok\n");

    printf("Is it valid? ");
    if(img.isValid()) {
        printf("OK\n");

        printf("DCT transform...");
        pic::Image *img_dct = pic::FilterDCT2D::Transform(&img, NULL, 8);
        printf(" Ok\n");

        printf("Removing small coefficients...");
        for(int i = 0; i < img_dct->size(); i++) {
            if(fabsf(img_dct->data[i]) < 0.025f) {
                img_dct->data[i] = 0.0f;
            }
        }
        printf(" Ok\n");

        pic::Image *imgOut = pic::FilterDCT2D::Inverse(img_dct, NULL, 8);

        printf("Writing the file to disk...");
        bool bWritten = imgOut->Write("../data/output/ip_simple_dct.png");

        if(bWritten) {
            printf(" Ok\n");
        } else {
            printf("Writing had some issues!\n");
        }
    } else {
        printf("No, the file is not valid!\n");
    }

    return 0;
}
