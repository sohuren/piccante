/*

PICCANTE
The hottest HDR imaging library!
http://piccantelib.net

Copyright (C) 2014
Visual Computing Laboratory - ISTI CNR
http://vcg.isti.cnr.it
First author: Francesco Banterle

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

*/

/**
 * NOTE: if you do not want to use this OpenGL functions loader,
 * please change it with your favorite one. This is just
 * a suggestion for running examples.
*/

#include "../common_code/gl_include.hpp"

#include <QKeyEvent>
#include <QtCore/QCoreApplication>
#include <QtOpenGL/QGLWidget>
#include <QApplication>
#include <QVBoxLayout>
#include <QLabel>

#include "piccante.hpp"

class GLWidget : public QGLWidget
        #ifndef _MSC_VER
        , protected QOpenGLFunctions
        #endif
{
protected:
    pic::QuadGL *quad;
    pic::FilterGLSimpleTMO *tmo;
    pic::FilterGLBilateral2DF *fltBilF;
    pic::FilterGLBilateral2DG *fltBilG;
    pic::FilterGLBilateral2DSP *fltBilSP;
    pic::FilterGLBilateral2DS *fltBilS;
    pic::FilterGLGaussian2D *fltGauss;
    pic::FilterGLAnisotropicDiffusion *fltAD;

    pic::ImageGL *img, *img_flt, *img_flt_tmo;
    pic::TechniqueGL technique;

    int method;

    /**
     * @brief initializeGL sets variables up.
     */
    void initializeGL(){

    #ifndef _MSC_VER
        initializeOpenGLFunctions();
    #endif

    #ifdef _MSC_VER
        if(ogl_LoadFunctions() == ogl_LOAD_FAILED) {
            printf("OpenGL functions are not loaded!\n");
        }
    #endif

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f );

        //read an input image
        img = new pic::ImageGL();
        img->Read("../data/input/yellow_flowers.png", pic::LT_NOR_GAMMA);
        img->generateTextureGL(GL_TEXTURE_2D, GL_FLOAT, false);

        #ifdef PIC_DEBUG
            printf("Image is read: %d\n", bRead);
        #endif

        //create a screen aligned quad
        pic::QuadGL::getTechnique(technique,
                                pic::QuadGL::getVertexProgramV3(),
                                pic::QuadGL::getFragmentProgramForView());

        quad = new pic::QuadGL(true);

        //allocate a new filter for simple tone mapping
        tmo = new pic::FilterGLSimpleTMO();

        float sigma_s = 16.0f;
        float sigma_r = 0.1f;

        fltGauss = new pic::FilterGLGaussian2D(sigma_s);

        //allocate a new bilateral filter
        fltBilG = new pic::FilterGLBilateral2DG(sigma_s, sigma_r);

        //allocate a new bilateral filter
        fltBilSP = new pic::FilterGLBilateral2DSP(sigma_s, sigma_r);

        //allocate a new bilateral filter
        fltBilS = new pic::FilterGLBilateral2DS(sigma_s, sigma_r);

        //allocate a new bilateral filter
        fltBilF = new pic::FilterGLBilateral2DF(sigma_s, sigma_r);

        //allocate a new anisotropic diffusion filter
        fltAD = new pic::FilterGLAnisotropicDiffusion(sigma_s, sigma_r);

        /*
        auto *out = pic::FilterGLSamplingMap::execute(img, NULL, 16.0f);
        out->loadToMemory();
        out->Write("testSampling.png");
        */

        img_flt_tmo = NULL;
        img_flt = NULL;
    }

    /**
     * @brief resizeGL
     * @param w
     * @param h
     */
    void resizeGL( int w, int h ){
        const qreal retinaScale = devicePixelRatio();
        glViewport(0, 0, w * retinaScale, h * retinaScale);
    }

    /**
     * @brief paintGL
     */
    void paintGL(){
        if(parentWidget() != NULL) {
            if(!parentWidget()->isVisible()) {
                return;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        pic::ImageGL *img_out = NULL;
        switch(method)
        {
            case 0:
                //input image
                img_out = img;
                window_ext->setWindowTitle(tr("Filtering Example: Original Image"));

            break;

            case 1:
                //apply the gaussian filter
                img_flt = fltGauss->Process(SingleGL(img), img_flt);
                img_out = img_flt;
                window_ext->setWindowTitle(tr("Filtering Example: Guassian Filter"));

            break;

            case 2:
                //apply the sampling bilateral filter
                img_flt = fltBilF->Process(SingleGL(img), img_flt);
                img_out = img_flt;
                window_ext->setWindowTitle(tr("Filtering Example: Full Bilateral"));

            break;

            case 3:
                //apply the bilateral grid filter
                img_flt = fltBilG->Process(SingleGL(img), img_flt);
                img_out = img_flt;
                window_ext->setWindowTitle(tr("Filtering Example: Bilateral Grid"));

            break;

            case 4:
                //apply the separate bilateral filter
                img_flt = fltBilSP->Process(SingleGL(img), img_flt);
                img_out = img_flt;
                window_ext->setWindowTitle(tr("Filtering Example: Separate Bilateral"));

            break;

            case 5:
                //apply the sampling bilateral filter
                img_flt = fltBilS->Process(SingleGL(img), img_flt);
                img_out = img_flt;
                window_ext->setWindowTitle(tr("Filtering Example: Sub-Sampled Bilateral"));
            break;

            case 6:
                //apply the anisotropic diffusion filter
                img_flt = fltAD->AnisotropicDiffusion(SingleGL(img), img_flt);
                img_out = img_flt;
                window_ext->setWindowTitle(tr("Filtering Example: Anisotropic Diffusion"));

            break;

        default:
            img_out = img;
            break;
        }

        //simple tone mapping: gamma + exposure correction
        img_flt_tmo = tmo->Process(SingleGL(img_out), img_flt_tmo);

        //visualization
        quad->Render(technique, img_flt_tmo->getTexture());
    }

public:

    QWidget *window_ext;

    /**
     * @brief GLWidget
     * @param format
     * @param parent
     */
    GLWidget( const QGLFormat& format, QWidget* parent = 0 ): QGLWidget(format, parent, 0)
    {
        setFixedWidth(800);
        setFixedHeight(533);

        tmo = NULL;
        img_flt = NULL;
        img_flt_tmo = NULL;
        method = 0;
    }

    /**
     * @brief update
     */
    void update()
    {
        method = (method + 1) % 7;
    }
};

class Window : public QWidget
{
protected:
    GLWidget *window_gl;
    QVBoxLayout *layout;
    QLabel *label;

public:

    /**
     * @brief Window
     * @param format
     */
    Window(const QGLFormat &format)
    {
        resize(800, 533 + 64);

        window_gl = new GLWidget(format, this);
        window_gl->window_ext = this;

        layout = new QVBoxLayout();

        layout->addWidget(window_gl);

        label = new QLabel(
        "Pease hit the space bar in order to switch from the original one to the filtered one using different filters.", this);
        label->setAlignment(Qt::AlignHCenter);
        label->setFixedWidth(800);
        label->setFixedHeight(64);

        layout->addWidget(label);

        setLayout(layout);

        setWindowTitle(tr("Filtering Example"));
    }

    ~Window()
    {
        delete window_gl;
        delete layout;
        delete label;
    }

    /**
     * @brief keyPressEvent
     * @param e
     */
    void keyPressEvent( QKeyEvent* e ){
        if(e->type() == QEvent::KeyPress) {
            if(e->key() == Qt::Key_Space) {
                window_gl->update();
                window_gl->updateGL();
            }
        }
    }
};

int main(int argc, char **argv)
{
    QApplication app( argc, argv );

    QGLFormat glFormat;
    glFormat.setVersion( 4, 0 );
    glFormat.setProfile( QGLFormat::CoreProfile );
    glFormat.setSampleBuffers( true );

    //Creating a window with OpenGL 4.0 Core profile
    Window w( glFormat );
    w.show();

    app.installEventFilter(&w);

    return app.exec();
}
