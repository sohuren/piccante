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

#ifndef PIC_GL_IMAGE_RAW_HPP
#define PIC_GL_IMAGE_RAW_HPP

#include "image.hpp"

#include "gl.hpp"
#include "util/gl/fbo.hpp"
#include "util/gl/formats.hpp"
#include "util/gl/timings.hpp"
#include "util/gl/buffer_ops.hpp"

namespace pic {

enum IMAGESTORE {IMG_GPU_CPU, IMG_CPU_GPU, IMG_CPU, IMG_GPU, IMG_NULL};

/**
 * @brief The ImageGL class
 */
class ImageGL: public Image
{
protected:
    GLuint		texture;
    GLenum		target;
    IMAGESTORE	mode;	        //TODO: check if the mode is always correctly updated
    bool		notOwnedGL;     //do we own the OpenGL texture??    
    Fbo			*tmpFbo;

    /**
     * @brief Destroy
     */
    void	Destroy();

public:
    std::vector<ImageGL *> stack;

    /**
     * @brief ImageGL
     */
    ImageGL();

    ~ImageGL();

    /**
     * @brief ImageGL
     * @param tex
     * @param target
     */
    ImageGL(GLuint tex, GLenum target);

    /**
     * @brief ImageGL
     * @param img
     * @param transferOwnership
     */
    ImageGL(Image *img, bool transferOwnership);

    /**
     * @brief ImageGL
     * @param img
     * @param mipmap
     * @param target
     */
    ImageGL(Image *img, bool mipmap, GLenum target);

    /**
     * @brief ImageGL
     * @param nameFile
     */
    ImageGL(std::string nameFile): Image(nameFile)
    {
        notOwnedGL = false;
        mode = IMG_CPU;
        texture = 0;
        target = 0;
        tmpFbo = NULL;
    }

    /**
     * @brief Image
     * @param frames
     * @param width
     * @param height
     * @param channels
     * @param data
     */
    ImageGL(int frames, int width, int height, int channels, float *data) : Image (frames, width, height, channels, data)
    {
        notOwnedGL = false;
        mode = IMG_CPU;
        texture = 0;
        target = 0;
        tmpFbo = NULL;
    }

    /**
     * @brief ImageGL
     * @param frames
     * @param width
     * @param height
     * @param channels
     * @param mode
     */
    ImageGL(int frames, int width, int height, int channels, IMAGESTORE mode, GLenum target);

    /**
     * @brief AllocateSimilarOneGL
     * @return
     */
    ImageGL *AllocateSimilarOneGL();

    /**
     * @brief CloneGL
     * @return
     */
    ImageGL *CloneGL();

    /**
     * @brief AssignGL
     * @param r
     * @param g
     * @param b
     * @param a
     */
    void AssignGL(float r, float g, float b, float a);

    /**
     * @brief generate
     * @param mipmap
     * @param target
     */
    GLuint generateTextureGL(bool mipmap, GLenum target);

    /**
     * @brief generateTexture2DGL
     * @param mipmap
     * @return
     */
    GLuint generateTexture2DGL(bool mipmap);

    /**
     * @brief generateTexture2DU32GL
     * @return
     */
    GLuint	generateTexture2DU32GL();

    /**
     * @brief generateTexture3DGL
     * @return
     */
    GLuint generateTexture3DGL();

    /**
     * @brief generateTextureCubeMapGL
     * @return
     */
    GLuint generateTextureCubeMapGL();

    /**
     * @brief generateTexture2DArrayGL
     * @return
     */
    GLuint generateTexture2DArrayGL();

    /**
     * @brief loadSliceIntoTexture
     * @param i
     */
    void loadSliceIntoTexture(int i);

    /**
     * @brief loadAllSlicesIntoTex
     */
    void loadAllSlicesIntoTex();

    /**
     * @brief loadFromMemory
     * @param mipmap
     */
    void loadFromMemory(bool mipmap);

    /**
     * @brief loadToMemory
     */
    void loadToMemory();

    /**
     * @brief readFromBindedFBO
     */
    void readFromBindedFBO();

    /**
     * @brief readFromFBO
     * @param fbo
     */
    void readFromFBO(Fbo *fbo);

    /**
     * @brief readFromFBO
     * @param fbo
     * @param format
     */
    void readFromFBO(Fbo *fbo, GLenum format);

    /**
     * @brief getTexture
     * @return
     */
    GLuint getTexture()
    {
        return texture;
    }

    /**
     * @brief bindTexture
     */
    void bindTexture();

    /**
     * @brief unBindTexture
     */
    void unBindTexture();

    /**
     * @brief updateModeGPU
     */
    void updateModeGPU()
    {
        if(mode == IMG_NULL) {
            mode = IMG_GPU;
        }

        if(mode == IMG_CPU) {
            mode = IMG_CPU_GPU;
        }
    }

    /**
     * @brief updateModeCPU
     */
    void updateModeCPU()
    {
        if(mode == IMG_NULL) {
            mode = IMG_CPU;
        }

        if(mode == IMG_GPU) {
            mode = IMG_CPU_GPU;
        }
    }

    /**
     * @brief SetTexture
     * @param texture
     */
    void SetTexture(GLuint texture)
    {
        //TODO: UNSAFE!
        this->texture = texture;
    }

    /**
     * @brief getTarget
     * @return
     */
    GLenum getTarget()
    {
        return target;
    }

    /**
     * @brief GenerateMask creates an opengl mask (a texture) from a buffer of bool values.
     * @param width
     * @param height
     * @param buffer
     * @param tex
     * @param tmpBuffer
     * @param mipmap
     * @return
     */
    static GLuint GenerateMask(int width, int height, bool *buffer = NULL,
                               GLuint tex = 0, unsigned char *tmpBuffer = NULL, bool mipmap = false)
    {
        bool bGen = (tex == 0);

        if(bGen) {
            glGenTextures(1, &tex);
        }

        glBindTexture(GL_TEXTURE_2D, tex);

        if(bGen) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        unsigned char *data = NULL;

        if(buffer != NULL) {
            int n = width * height;

            if(tmpBuffer != NULL) {
                data = tmpBuffer;
            } else {
                data = new unsigned char[n * 3];
            }

            #pragma omp parallel for

            for(int i = 0; i < n; i++) {
                data[i] = buffer[i] ? 255 : 0;
            }
        }

        if(bGen) {
            if(mipmap) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            }

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }

        //Note: GL_LUMINANCE is deprecated since OpenGL 3.1
        #ifndef PIC_DISABLE_OPENGL_NON_CORE
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8 , width, height, 0, GL_LUMINANCE,
                     GL_UNSIGNED_BYTE, data);
        #endif

        #ifdef PIC_DISABLE_OPENGL_NON_CORE
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED,
                         GL_UNSIGNED_BYTE, data);
        #endif

        if(mipmap && bGen) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        glBindTexture(GL_TEXTURE_2D, 0);

        if(data != NULL && tmpBuffer == NULL) {
            delete[] data;
        }

        return tex;
    }


    /**
     * @brief operator +=
     * @param a
     */
    void operator +=(ImageGL &a)
    {
        if(SimilarType(&a)) {
            BufferOpsGL *ops = BufferOpsGL::getInstance();
            ops->list[0]->Process(getTexture(), a.getTexture(), getTexture(), width, height);
        } else {
            if((nPixels() == a.nPixels()) && (a.channels == 1)) {

            }
        }

    }

    /**
     * @brief operator *=
     * @param a
     */
    void operator *=(ImageGL &a)
    {
        if(SimilarType(&a)) {
            BufferOpsGL *ops = BufferOpsGL::getInstance();
            ops->list[1]->Process(getTexture(), a.getTexture(), getTexture(), width, height);
        } else {
            if((nPixels() == a.nPixels()) && (a.channels == 1)) {

            }
        }
    }

    /**
     * @brief operator *=
     * @param a
     */
    void operator *=(const float &a)
    {
        BufferOpsGL *ops = BufferOpsGL::getInstance();

        ops->list[5]->Update(a);
        ops->list[5]->Process(getTexture(), 0, getTexture(), width, height);
    }

    /**
     * @brief operator -=
     * @param a
     */
    void operator -=(ImageGL &a)
    {
        if(SimilarType(&a)) {
            BufferOpsGL *ops = BufferOpsGL::getInstance();
            ops->list[2]->Process(getTexture(), a.getTexture(), getTexture(), width, height);
        } else {
            if((nPixels() == a.nPixels()) && (a.channels == 1)) {

            }
        }
    }

    /**
     * @brief operator /=
     * @param a
     */
    void operator /=(ImageGL &a)
    {
        if(SimilarType(&a)) {
            BufferOpsGL *ops = BufferOpsGL::getInstance();
            ops->list[3]->Process(getTexture(), a.getTexture(), getTexture(), width, height);
        } else {
            if((nPixels() == a.nPixels()) && (a.channels == 1)) {

            }
        }
    }

    /**
     * @brief operator /=
     * @param a
     */
    void operator /=(const float &a)
    {
        BufferOpsGL *ops = BufferOpsGL::getInstance();

        ops->list[6]->Update(a);
        ops->list[6]->Process(getTexture(), 0, getTexture(), width, height);
    }

    /**
    * @brief operator -=
    * @param a
    */
   void operator -=(const float &a)
   {
       BufferOpsGL *ops = BufferOpsGL::getInstance();

       ops->list[7]->Update(a);
       ops->list[7]->Process(getTexture(), 0, getTexture(), width, height);
   }
};

ImageGL::ImageGL() : Image()
{
    notOwnedGL = false;
    texture = 0;
    target = 0;
    mode = IMG_NULL;
    tmpFbo = NULL;
}

ImageGL::ImageGL(GLuint tex, GLuint target) : Image()
{
    notOwnedGL = true;

    tmpFbo = NULL;

    mode = IMG_GPU;

    this->target = target;
    texture = tex;

    GLint internalFormat;

    switch(target) {
    case GL_TEXTURE_2D: {
        glBindTexture(GL_TEXTURE_2D, texture);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT,
                                 &internalFormat);

        channels = getChannelsFromInternalFormatGL(internalFormat);

        frames = 1;

        glBindTexture(GL_TEXTURE_2D, 0);
    }
    break;

    case GL_TEXTURE_CUBE_MAP: {
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP, 0, GL_TEXTURE_WIDTH, &width);
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP, 0, GL_TEXTURE_HEIGHT, &height);
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP, 0, GL_TEXTURE_INTERNAL_FORMAT,
                                 &internalFormat);

        channels = getChannelsFromInternalFormatGL(internalFormat);

        frames = 6;

        glBindTexture(GL_TEXTURE_2D, 0);
    }
    break;

    case GL_TEXTURE_2D_ARRAY: {
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
        glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_WIDTH, &width);
        glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_HEIGHT, &height);
        glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_DEPTH, &frames);
        glGetTexLevelParameteriv(GL_TEXTURE_2D_ARRAY, 0, GL_TEXTURE_INTERNAL_FORMAT,
                                 &internalFormat);

        channels = getChannelsFromInternalFormatGL(internalFormat);
        
        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }
    break;

    case GL_TEXTURE_3D: {
        glBindTexture(GL_TEXTURE_3D, texture);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &width);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &height);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &frames);
        glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_INTERNAL_FORMAT,
                                 &internalFormat);

        channels = getChannelsFromInternalFormatGL(internalFormat);

        glBindTexture(GL_TEXTURE_3D, 0);
    }
    break;
    }

    AllocateAux();
}

ImageGL::ImageGL(Image *img, bool mipmap, GLenum target): Image()
{
    notOwnedGL = false;
    notOwned = true;

    tmpFbo = NULL;

    width    = img->width;
    height   = img->height;
    frames   = img->frames;
    channels = img->channels;
    data     = img->data;

    CalculateStrides();

    texture = 0;

    generateTextureGL(mipmap, target);

    mode = IMG_CPU_GPU;
}

ImageGL::ImageGL(Image *img, bool transferOwnership = false) : Image()
{

    if(transferOwnership) {
        notOwned = false;
        img->ChangeOwnership(true);
    } else {
        notOwned = true;
    }

    notOwnedGL = false;

    tmpFbo = NULL;

    width    = img->width;
    height   = img->height;
    frames   = img->frames;
    channels = img->channels;
    data     = img->data;

    CalculateStrides();

    texture = 0;

    mode = IMG_CPU;
}

ImageGL::ImageGL(int frames, int width, int height, int channels,
                       IMAGESTORE mode, GLenum target) : Image()
{
    notOwnedGL = false;
    tmpFbo = NULL;

    this->mode = mode;

    if(this->mode == IMG_GPU_CPU) {
        this->mode = IMG_CPU_GPU;
    }

    switch(this->mode) {
    case IMG_CPU_GPU: {
        Allocate(width, height, channels, frames);

        generateTextureGL(false, target);
    }
    break;

    case IMG_CPU: {
        Allocate(width, height, channels, frames);
    }
    break;

    case IMG_GPU: {
        this->width = width;
        this->height = height;
        this->frames = frames;
        this->depth = frames;
        this->channels = channels;

        AllocateAux();

        generateTextureGL(false, target);
    }
    break;
    }
}

ImageGL::~ImageGL()
{
    Destroy();
}

GLuint ImageGL::generateTextureGL(bool mipmap, GLenum target)
{
    this->target  = target;

    switch(target) {
        case GL_TEXTURE_2D:
        {
            generateTexture2DGL(mipmap);
        } break;

        case GL_TEXTURE_3D:
        {
            if(frames > 1) {
                generateTexture3DGL();
            } else {
                generateTexture2DGL(mipmap);
                this->target = GL_TEXTURE_2D;
            }
        } break;

        case GL_TEXTURE_2D_ARRAY: {
            generateTexture2DArrayGL();
        } break;

        case GL_TEXTURE_CUBE_MAP: {
            if(frames > 5) {
                generateTextureCubeMapGL();
            } else {
                if(frames > 1) {
                    generateTexture2DArrayGL();
                    this->target = GL_TEXTURE_2D_ARRAY;
                } else {
                    generateTexture2DGL(mipmap);
                    this->target = GL_TEXTURE_2D;
                }
            }
        } break;
    }

    return texture;
}

ImageGL *ImageGL::CloneGL()
{
    //TODO: to improve CloneGL
    Image *tmp = this->Clone();
    return new ImageGL(tmp, false, target);
}

void ImageGL::Destroy()
{
    if(notOwnedGL) {
        return;
    }

    if(texture != 0) {
        glDeleteTextures(1, &texture);
        texture = 0;
        target = 0;
    }
}

ImageGL *ImageGL::AllocateSimilarOneGL()
{
#ifdef PIC_DEBUG
    printf("%d %d %d %d %d\n", frames, width, height, channels, mode);
#endif

    ImageGL *ret = new ImageGL(frames, width, height, channels, mode, target);
    return ret;
}

void ImageGL::AssignGL(float r = 0.0f, float g = 0.0f, float b = 0.0f,
                          float a = 1.0f)
{
    if(tmpFbo == NULL) {
        tmpFbo = new Fbo();
        tmpFbo->create(width, height, 1, false, texture);
    }

    glClearColor(r, g, b, a);

    //Rendering
    tmpFbo->bind();
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    glClear(GL_COLOR_BUFFER_BIT);

    //Fbo
    tmpFbo->unbind();
}

GLuint ImageGL::generateTexture2DGL(bool mipmap = false)
{
    if(width <1 || height < 1 || channels < 1) {
        return 0;
    }

    updateModeGPU();

    target = GL_TEXTURE_2D;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if(mipmap) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    int mode, modeInternalFormat;
    getModesGL(channels, mode, modeInternalFormat);
    glTexImage2D(GL_TEXTURE_2D, 0, modeInternalFormat, width, height, 0,
                 mode, GL_FLOAT, data);

    if(mipmap) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

GLuint	ImageGL::generateTextureCubeMapGL()
{
    if(width <1 || height < 1 || channels < 1 || frames < 6) {
        return 0;
    }

    updateModeGPU();

    target = GL_TEXTURE_CUBE_MAP;

    int mode, modeInternalFormat;
    getModesGL(channels, mode, modeInternalFormat);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //Order Pos,Neg X,Y,Z
    for(int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, modeInternalFormat, width,
                     height, 0, mode, GL_FLOAT, &data[tstride * i]);
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return texture;
}

GLuint ImageGL::generateTexture2DU32GL()
{
    if(width <1 || height < 1 || channels < 1) {
        return 0;
    }

    updateModeGPU();

    target = GL_TEXTURE_2D;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    int *buffer = new int[width * height * channels];

    for(int i = 0; i < (width * height * channels); i++) {
        buffer[i] = int(lround(data[i]));
    }

    int mode, modeInternalFormat;
    getModesIntegerGL(channels, mode, modeInternalFormat);

    glTexImage2D(GL_TEXTURE_2D, 0, modeInternalFormat, width, height, 0,
                 mode, GL_INT, buffer);

    glBindTexture(GL_TEXTURE_2D, 0);

    delete[] buffer;

    return texture;
}

GLuint ImageGL::generateTexture3DGL()
{
    if(width <1 || height < 1 || channels < 1 || frames < 1) {
        return 0;
    }

    updateModeGPU();

    target = GL_TEXTURE_3D;

    int mode, modeInternalFormat;
    getModesGL(channels, mode, modeInternalFormat);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage3D(GL_TEXTURE_3D, 0, modeInternalFormat, width, height, frames, 0,
                 mode, GL_FLOAT, data);

    glBindTexture(GL_TEXTURE_3D, 0);

//	for(int i=0;i<frames;i++)
//		glTexSubImage3D(GL_TEXTURE_3D,0,0,0,i,width,height,1,mode,GL_FLOAT,&data[i*tstride]);

    return texture;
}

GLuint ImageGL::generateTexture2DArrayGL()
{
    if(width <1 || height < 1 || channels < 1 || frames < 1) {
        return 0;
    }

    updateModeGPU();

    target = GL_TEXTURE_2D_ARRAY;

    int mode, modeInternalFormat;
    getModesGL(channels, mode, modeInternalFormat);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, modeInternalFormat, width, height, frames,
                 0, mode, GL_FLOAT, data);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    return texture;
}

void ImageGL::loadFromMemory(bool mipmap = false)
{
    int mode, modeInternalFormat;
    getModesGL(channels, mode, modeInternalFormat);

    glBindTexture(target, texture);

    switch(target) {
        case GL_TEXTURE_2D: {
            glTexImage2D(target, 0, modeInternalFormat, width, height, 0,
                         mode, GL_FLOAT, data);

        } break;

        case GL_TEXTURE_3D: {
            glTexImage3D(GL_TEXTURE_3D, 0, modeInternalFormat, width, height, frames, 0,
                         mode, GL_FLOAT, data);
        } break;
    }

    glBindTexture(target, 0);
}

void ImageGL::loadToMemory()
{
    if(texture == 0) {
        #ifdef PIC_DEBUG
            printf("This texture can not be trasferred from GPU memory\n");
        #endif
        return;
    }

    if(data == NULL) {
        #ifdef PIC_DEBUG
            printf("RAM memory allocated: %d %d %d %d\n", width, height, channels, frames);
        #endif

        Allocate(width, height, channels, frames);
        this->mode = IMG_CPU_GPU;
    }

    int mode, modeInternalFormat;
    getModesGL(channels, mode, modeInternalFormat);

    bindTexture();

    glGetTexImage(target, 0, mode, GL_FLOAT, data);

    unBindTexture();
}

void ImageGL::loadSliceIntoTexture(int i)
{
    int mode, modeInternalFormat;
    getModesGL(channels, mode, modeInternalFormat);

    glBindTexture(target, texture);
    i = i % frames;
    glTexSubImage3D(target, 0, 0, 0, i, width, height, 1, mode, GL_FLOAT,
                    &data[i * tstride]);

    glBindTexture(target, 0);
}

void ImageGL::loadAllSlicesIntoTex()
{
    for(int i = 0; i < frames; i++) {
        loadSliceIntoTexture(i);
    }
}

void ImageGL::readFromFBO(Fbo *fbo, GLenum format)
{
    //TO DO: check data
    bool bCheck =	(fbo->width  != width) ||
                    (fbo->height != height);

    if(data == NULL || bCheck) {
        Allocate(fbo->width, fbo->height, 4, 1);
    }

    //ReadPixels from the FBO
    fbo->bind();
    glReadPixels(0, 0, width, height, format, GL_FLOAT, data);
    fbo->unbind();

    /*	glBindTexture(GL_TEXTURE_2D, fbo->tex);
    	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, data);
    	glBindTexture(GL_TEXTURE_2D, 0);*/
}

void ImageGL::readFromFBO(Fbo *fbo)
{
    if(mode == IMG_NULL) {
        mode = IMG_CPU;
    }

    readFromFBO(fbo, GL_RGBA);
}

void ImageGL::readFromBindedFBO()
{

    int mode, modeInternalFormat;
    getModesGL(channels, mode, modeInternalFormat);

    if(mode == 0x0) {
        #ifdef PIC_DEBUG
            printf("void ImageGL::readFromBindedFBO(): error unknown format!");
        #endif
        return;
    }

    //TODO: check width height and data (mode and modeInternalFormat)

    glReadPixels(0, 0, width, height, mode, GL_FLOAT, data);
    FlipV();
}

void ImageGL::bindTexture()
{
    glBindTexture(target, texture);
}

void ImageGL::unBindTexture()
{
    glBindTexture(target, 0);
}

} // end namespace pic

#endif /* PIC_GL_IMAGE_RAW_HPP */

