//
//  main.m
//  OpenGL-Texture003-公转自转
//
//  Created by lxj on 2018/12/15.
//  Copyright © 2018 lxj. All rights reserved.
//

#include "GLTriangleBatch.h"
#include "GLBatch.h"
#include "GLMatrixStack.h"
#include "GLShaderManager.h"
#include "StopWatch.h"
#include "GLFrustum.h"
#include "GLFrame.h"
#include "GLGeometryTransform.h"

#ifdef __APPLE__
#include <glut/glut.h>
#else
#include <GL/glut.h>
#endif

GLShaderManager shaderManager;
GLFrame cameraFrame;
GLFrustum viewFrustum;
GLMatrixStack modelViewMatri;
GLMatrixStack projectionMatri;

GLGeometryTransform transformLine;

GLuint textures[3];

GLTriangleBatch bigBatch;
GLTriangleBatch smallBatch;
GLBatch floorBatch;

//**4、添加附加随机球
#define NUM_SPHERES 50
GLFrame spheres[NUM_SPHERES];

bool loadTga(const char *name,GLenum minFilter,GLenum magFilter,GLenum mode){
    
    GLbyte *pBytes;
    GLint width, heiht, componets;
    GLenum format;
    
    pBytes = gltReadTGABits(name, &width, &heiht, &componets, &format);
    
    if (pBytes == NULL) {
        return false;
    }
    
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);
    
    //3、精密包装像素数据
    //参数1：GL_UNPACK_ALIGNMENT,指定OpenGL如何从数据缓存区中解包图像数据
    //参数2:针对GL_UNPACK_ALIGNMENT 设置的值
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    glTexImage2D(GL_TEXTURE_2D, 0, componets, width, heiht, 0, format, GL_UNSIGNED_BYTE, pBytes);
    
    
    //只有minFilter 等于以下四种模式，才可以生成Mip贴图
    //GL_NEAREST_MIPMAP_NEAREST具有非常好的性能，并且闪烁现象非常弱
    //GL_LINEAR_MIPMAP_NEAREST常常用于对游戏进行加速，它使用了高质量的线性过滤器
    //GL_LINEAR_MIPMAP_LINEAR 和GL_NEAREST_MIPMAP_LINEAR 过滤器在Mip层之间执行了一些额外的插值，以消除他们之间的过滤痕迹。
    //GL_LINEAR_MIPMAP_LINEAR 三线性Mip贴图。纹理过滤的黄金准则，具有最高的精度。
    if(minFilter == GL_LINEAR_MIPMAP_LINEAR ||
       minFilter == GL_LINEAR_MIPMAP_NEAREST ||
       minFilter == GL_NEAREST_MIPMAP_LINEAR ||
       minFilter == GL_NEAREST_MIPMAP_NEAREST)
        //加载Mip,纹理生成所有的Mip层
        //参数：GL_TEXTURE_1D、GL_TEXTURE_2D、GL_TEXTURE_3D
        glGenerateMipmap(GL_TEXTURE_2D);
    
    free(pBytes);
    
    return true;
}


void setup(void){
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    shaderManager.InitializeStockShaders();
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    
    //往地板floorBatch批处理中添加顶点数据
    GLfloat texSize = 10.0f;
    floorBatch.Begin(GL_TRIANGLE_FAN, 4,1);
    floorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    floorBatch.Vertex3f(-20.f, -0.41f, 20.0f);
    
    floorBatch.MultiTexCoord2f(0, texSize, 0.0f);
    floorBatch.Vertex3f(20.0f, -0.41f, -20.f);
    
    floorBatch.MultiTexCoord2f(0, texSize, texSize);
    floorBatch.Vertex3f(-20.0f, -0.41f, -20.0f);
    
    floorBatch.MultiTexCoord2f(0, 0.0f, texSize);
    floorBatch.Vertex3f(20.0f, -0.41f, 20.0f);
    floorBatch.End();
    
    gltMakeSphere(bigBatch, 0.40f, 30.0f, 30.0f);
    gltMakeSphere(smallBatch, 0.150f, 30.0f, 30.0f);

    
    glGenTextures(3, textures);
    
    //地板
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    loadTga("Marble.tga",GL_LINEAR_MIPMAP_LINEAR,GL_LINEAR,GL_REPEAT);
    
    //大球
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    loadTga("test.tga",GL_LINEAR_MIPMAP_LINEAR,GL_LINEAR,GL_CLAMP_TO_EDGE);
    
    //小球
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    loadTga("moonlike.tga",GL_LINEAR_MIPMAP_LINEAR,GL_LINEAR,GL_CLAMP_TO_EDGE);

    
    for (int i = 0; i < NUM_SPHERES; i++) {
        
       GLfloat x = float((random()%400-200)*0.2f);
       GLfloat y = float((random()%400-200)*0.2f);
        GLfloat z = float((random()%400-200)*0.2f);

        spheres[i].SetOrigin(x, y, z);
    }
    
//    cameraFrame.MoveForward(-10);/
}


void changeSize(int w, int h){
    glViewport(0, 0, w, h);
    
    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 100.0f);
    projectionMatri.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    transformLine.SetMatrixStacks(modelViewMatri, projectionMatri);
    modelViewMatri.LoadIdentity();
}

void specailKey(int key, int x, int y){
    
    if(key == GLUT_KEY_UP)
        cameraFrame.MoveForward(1.f);
    
    if(key == GLUT_KEY_DOWN)
        cameraFrame.MoveForward(-1.0f);
    
    if(key == GLUT_KEY_LEFT)
        cameraFrame.RotateWorld(m3dDegToRad(5), 0, 1.0f, 0.0f);
    
    if(key == GLUT_KEY_RIGHT)
        cameraFrame.RotateWorld(m3dDegToRad(-5), 0, 1.0f, 0.0f);
    
    glutPostRedisplay();
}
static GLfloat vFloorColor[] = { 1.0f, 1.0f, 0.0f, 0.75f};
static GLfloat withte[] = { 1.0f, 1.0f, 1.0f, 1.f};
static GLfloat vLightPos[] = { 1.0f, 1.50f, 0.0f, 1.0f };

void jingxiang(GLfloat yrot){
    
    //**4、添加光源
    //光源位置的全局坐标存储在vLightPos变量中，其中包含了光源位置x坐标、y坐标、z坐标和w坐标。我们必须保留w坐标为1.0。因为无法用一个3分量去乘以4*4矩阵
    M3DVector4f    vLightTransformed;
    
    //定义一个4*4的相机矩阵
    M3DMatrix44f mCamera;
    //从modelViewMatrix获取矩阵堆栈顶部的值
    modelViewMatri.GetMatrix(mCamera);
    //将照相机矩阵mCamera 与 光源矩阵vLightPos 相乘获得vLigthTransformed矩阵
    m3dTransformVector4(vLightTransformed, vLightPos, mCamera);
    
    modelViewMatri.PushMatrix();
    
    modelViewMatri.Translatev(vLightPos);
    
    //月亮
    shaderManager.UseStockShader(GLT_SHADER_FLAT,transformLine.GetModelViewProjectionMatrix(),withte);
    smallBatch.Draw();
    modelViewMatri.PopMatrix();
    
    //随机小球
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    for (int i = 0; i <NUM_SPHERES; i++) {
        
        modelViewMatri.PushMatrix();
        modelViewMatri.MultMatrix(spheres[i]);
        
        shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                     modelViewMatri.GetMatrix(),
                                     transformLine.GetProjectionMatrix(),
                                     vLightTransformed,
                                     withte,
                                     0);
        smallBatch.Draw();
        modelViewMatri.PopMatrix();
    }
    
    
    //大球
    //modelViewMatrix 顶部矩阵沿着z轴移动2.5单位
    modelViewMatri.Translate(0.0f, 0.05f, -2.5f);
    modelViewMatri.PushMatrix();
    modelViewMatri.Rotate(yrot, 0.0f, 1.0f, 0.0f);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                 modelViewMatri.GetMatrix(),
                                 transformLine.GetProjectionMatrix(),
                                 vLightTransformed,
                                 withte,
                                 0);
    bigBatch.Draw();
    
    modelViewMatri.PopMatrix();
    
    //小球
    modelViewMatri.Rotate(yrot*-2.0f, 0.0, 1.0f, 0.0f);
    modelViewMatri.Translate(0.8f, 0.0f, 0.0f);

    glBindTexture(GL_TEXTURE_2D, textures[2]);
    
    
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                 modelViewMatri.GetMatrix(),
                                 transformLine.GetProjectionMatrix(),
                                 vLightTransformed,
                                 withte,
                                 0);
    
    smallBatch.Draw();
}


void renderScene(void){
    
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    static CStopWatch rotTimer;
    GLfloat yrot = rotTimer.GetElapsedSeconds() * 60;
    
    modelViewMatri.PushMatrix();
    
    //观察者矩阵
    M3DMatrix44f mcamera;
    cameraFrame.GetCameraMatrix(mcamera);
    modelViewMatri.MultMatrix(mcamera);
    
    
    //镜像部分
    modelViewMatri.PushMatrix();
    
    modelViewMatri.Scale(0.95f, -1.0f, 1.0f);
    modelViewMatri.Translate(0.0f, 0.8f, 0.0f);
        glFrontFace(GL_CW);
    jingxiang(yrot);
      glFrontFace(GL_CCW);
    modelViewMatri.PopMatrix();
    
    
    //开启混合功能
    glEnable(GL_BLEND);
    
    glBindTexture(GL_TEXTURE_2D, textures[0]);

    
    /*glBlendFunc 颜色混合方程式
     参数1：目标颜色
     参数2：源颜色
     */
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
        shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE,
                                     transformLine.GetModelViewProjectionMatrix(),
                                     vFloorColor,0);
    floorBatch.Draw();
    
    //取消混合
    glDisable(GL_BLEND);
    
    
    jingxiang(yrot);

    
    
    modelViewMatri.PopMatrix();
    
    glutSwapBuffers();
    glutPostRedisplay();
}


int main(int argc, char * argv[]) {
    
    gltSetWorkingDirectory(argv[0]);
    
    glutInit(&argc, argv);
    
    glutInitWindowSize(800, 600);
    glutCreateWindow("公转自转");
    
    glutDisplayFunc(renderScene);
    glutSpecialFunc(specailKey);
    glutReshapeFunc(changeSize);
    
    GLenum err = glewInit();
    if(err != GLEW_OK){
        return 1;
    }
    
    setup();
    glutMainLoop();
    
    glDeleteTextures(3, textures);
    
    return 0;
}
