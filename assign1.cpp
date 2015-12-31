/*
  CSCI 480 Computer Graphics
  Assignment 1: Height Fields
  C++ starter code
*/

#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include <pic.h>

int g_iMenuId;

int g_vMousePos[2] = {0, 0};
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

// these bool variables are used as switches (used for keyboard number keys)
bool isSmooth = false;  // whether smoothing the points, lines and polygons
bool isColorFrom2ndImage = false; // whether color the model with color values taken from another image
bool isTextureMapping = false; // whether texture map the model with another image
bool isScreenShot = false; // whether start to save screenshots

// Saving screenshot count variable
unsigned int filenameCount = 255; // start number
unsigned int filenameCountEndNum = 300; // end number

static GLuint texture;  // texture mapping

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;
//typedef enum { POINTS, LINES, TRIANGLES } RENDERSTATE;

//RENDERSTATE g_RenderState = POINTS;
CONTROLSTATE g_ControlState = ROTATE;

/* state of the world */
float g_vLandRotate[3] = {0.0, 0.0, 0.0};
float g_vLandTranslate[3] = {0.0, 0.0, 0.0};
float g_vLandScale[3] = {1.0, 1.0, 1.0};

/* see <your pic directory>/pic.h for type Pic */
Pic * g_pHeightData;
Pic * secondImageData;  // 2nd image data

// Texture Mapping initial function
GLuint textureData() {
    if (!secondImageData) return -1; // make sure it's safe
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_DECAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_DECAL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    gluBuild2DMipmaps(GL_TEXTURE_2D, secondImageData->bpp, secondImageData->nx, secondImageData->ny, GL_RGB, GL_UNSIGNED_BYTE, secondImageData->pix);
    return texture;
}

/* Write a screenshot to the specified filename */
void saveScreenshot (char *filename)
{
  int i, j;
  Pic *in = NULL;

  if (filename == NULL)
    return;

  /* Allocate a picture buffer */
  in = pic_alloc(640, 480, 3, NULL);

  printf("File to save to: %s\n", filename);

  for (i=479; i>=0; i--) {
    glReadPixels(0, 479-i, 640, 1, GL_RGB, GL_UNSIGNED_BYTE,
                 &in->pix[i*in->nx*in->bpp]);
  }

  if (jpeg_write(filename, in))
    printf("File saved Successfully\n");
  else
    printf("Error in Saving\n");

  pic_free(in);
}

void myinit()
{
  /* setup gl view here */
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glEnable(GL_DEPTH_TEST);
  glShadeModel(GL_SMOOTH);
  glDepthFunc(GL_LESS);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
//  glHint(GL_POINT_SMOOTH, GL_NICEST);
//  glHint(GL_LINE_SMOOTH, GL_NICEST);
//  glHint(GL_POLYGON_SMOOTH, GL_NICEST);
  texture = textureData();  // Initiate texture mapping
}

// Transformation matice: Transform the model with mouse and special keys
void objectTransform() {
    glRotatef(g_vLandRotate[0], 1.0, 0.0, 0.0);
    glRotatef(g_vLandRotate[1], 0.0, 1.0, 0.0);
    glRotatef(g_vLandRotate[2], 0.0, 0.0, 1.0);
    glTranslatef(g_vLandTranslate[0], g_vLandTranslate[1], g_vLandTranslate[2]);
    glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]);
}

// Render as a perspective view
void reshape(int width, int height) {
    if (height == 0) height = 1;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40, (GLfloat)width / (GLfloat)height, 0.001, 100);
    glMatrixMode(GL_MODELVIEW);
    
}

// Calculate the z value of each pixel from RGB value.
// Both bpp = 1 and bpp = 3 cases are considered
unsigned char getHeight(int j, int i) {
    if (g_pHeightData->bpp == 1) {
        return PIC_PIXEL(g_pHeightData, j, i, 0);
    }
    if (g_pHeightData->bpp == 3 || g_pHeightData->bpp == 4) {
        return (unsigned char)(((float)PIC_PIXEL(g_pHeightData, j, i, 0) + (float)PIC_PIXEL(g_pHeightData, j, i, 1) + (float)PIC_PIXEL(g_pHeightData, j, i, 2)) / 3.0);
    }
}

void display() {
  /* draw 1x1 cube about origin */
  /* replace this code with your height field implementation */
  /* you may also want to precede it with your 
     rotation/translation/scaling */
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0, 0, -2.3);
    objectTransform();
    
    if (isTextureMapping) glColor3f(1.0f, 1.0f, 1.0f); // make it brighter for texture mapping
    for (int i = 0; i < g_pHeightData->ny - 1; i++) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j < g_pHeightData->nx; j++) {
            // I use triangle strip here, so each time one line and the line below should be computed at the same iteration
            
            unsigned char height = getHeight(j, i);
            if (!isTextureMapping) {  // if it's not texture mapping, just compute the color
                if (isColorFrom2ndImage) {  // if '5' is pressed: color values are taken from another image
                    if (secondImageData->bpp == 1) {
                        glColor3f((float)PIC_PIXEL(secondImageData, j, i, 0) / 255.0, (float)PIC_PIXEL(secondImageData, j, i, 0) / 255.0, (float)PIC_PIXEL(secondImageData, j, i, 0) / 255.0);
                    } else {
                        glColor3f((float)PIC_PIXEL(secondImageData, j, i, 0) / 255.0, (float)PIC_PIXEL(secondImageData, j, i, 1) / 255.0, (float)PIC_PIXEL(secondImageData, j, i, 2) / 255.0);
                    }
                } else {
                    glColor3f((float)height / 255, 0.5, 0.8);
                }
                glVertex3f(-0.5 + (float)j / g_pHeightData->nx, 0.5 - (float)i / g_pHeightData->ny, (float)height / 255.0 - 0.5);
            } else { // if it's texture mapping, no need to compute the color, just use the texture.
                glTexCoord2d((float)j / g_pHeightData->nx, (float)i / g_pHeightData->ny);
                glVertex3f(-0.5 + (float)j / g_pHeightData->nx, 0.5 - (float)i / g_pHeightData->ny, (float)height / 255.0 - 0.5);
            }
            
            height = getHeight(j, i + 1);
            if (!isTextureMapping) { // if it's not texture mapping, just compute the color
                if (isColorFrom2ndImage) { // if '5' is pressed: color values are taken from another image
                    if (secondImageData->bpp == 1) {
                        glColor3f((float)PIC_PIXEL(secondImageData, j, i + 1, 0) / 255.0, (float)PIC_PIXEL(secondImageData, j, i + 1, 0) / 255.0, (float)PIC_PIXEL(secondImageData, j, i + 1, 0) / 255.0);
                    } else {
                        glColor3f((float)PIC_PIXEL(secondImageData, j, i + 1, 0) / 255.0, (float)PIC_PIXEL(secondImageData, j, i + 1, 1) / 255.0, (float)PIC_PIXEL(secondImageData, j, i + 1, 2) / 255.0);
                    }
                } else {
                    glColor3f((float)height / 255, 0.5, 0.8);
                }
                glVertex3f(-0.5 + (float)j / g_pHeightData->nx, 0.5 - (float)(i + 1) / g_pHeightData->ny, (float)height / 255.0 - 0.5);
            } else { // if it's texture mapping, no need to compute the color, just use the texture.
                glTexCoord2d((float)j / g_pHeightData->nx, (float)(i + 1) / g_pHeightData->ny);
                glVertex3f(-0.5 + (float)j / g_pHeightData->nx, 0.5 - (float)(i + 1) / g_pHeightData->ny, (float)height / 255.0 - 0.5);
            }
        }
        glEnd();
     }
    
    // saving screenshot
    if (isScreenShot && filenameCount < filenameCountEndNum) {
        char fileName[100];
        sprintf(fileName, "duoduoyu.anim.%04d.jpg", filenameCount);
        saveScreenshot(fileName);
        filenameCount++;
    }
    
  glutSwapBuffers();
}

void menufunc(int value)
{
  switch (value)
  {
    case 0:
      exit(0);
      break;
  }
}

void doIdle()
{
  /* do some stuff... */

  /* make the screen update */
  glutPostRedisplay();
}

/* converts mouse drags into information about 
rotation/translation/scaling */
void mousedrag(int x, int y)
{
  int vMouseDelta[2] = {x-g_vMousePos[0], y-g_vMousePos[1]};
  
  switch (g_ControlState)
  {
    case TRANSLATE:  
      if (g_iLeftMouseButton)
      {
        g_vLandTranslate[0] += vMouseDelta[0]*0.01;
        g_vLandTranslate[1] -= vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandTranslate[2] += vMouseDelta[1]*0.01;
      }
      break;
    case ROTATE:
      if (g_iLeftMouseButton)
      {
        g_vLandRotate[0] += vMouseDelta[1];
        g_vLandRotate[1] += vMouseDelta[0];
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandRotate[2] += vMouseDelta[1];
      }
      break;
    case SCALE:
      if (g_iLeftMouseButton)
      {
        g_vLandScale[0] *= 1.0+vMouseDelta[0]*0.01;
        g_vLandScale[1] *= 1.0-vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandScale[2] *= 1.0-vMouseDelta[1]*0.01;
      }
      break;
  }
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mouseidle(int x, int y)
{
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mousebutton(int button, int state, int x, int y)
{

  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      g_iLeftMouseButton = (state==GLUT_DOWN);
          print("left button");
      break;
    case GLUT_MIDDLE_BUTTON:
      g_iMiddleMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_RIGHT_BUTTON:
      g_iRightMouseButton = (state==GLUT_DOWN);
      break;
  }
 
  switch(glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      g_ControlState = TRANSLATE;
      break;
    case GLUT_ACTIVE_SHIFT:
      g_ControlState = SCALE;
      break;
    default:
      g_ControlState = ROTATE;
      break;
  }

  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void keyboardFunctions(unsigned char key, int x, int y) {
    switch (key) {
        case '1':  // render as points
            //g_RenderState = POINTS;
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            break;
        case '2':  // render as lines
            //g_RenderState = LINES;
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
        case '3':  // render as solid triangles
            //g_RenderState = TRIANGLES;
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        case '4':  // smooth the results
            if (!isSmooth) {
                glEnable(GL_POINT_SMOOTH);
                glEnable(GL_LINE_SMOOTH);
                glEnable(GL_POLYGON_SMOOTH);
            } else {
                glDisable(GL_POINT_SMOOTH);
                glDisable(GL_LINE_SMOOTH);
                glDisable(GL_POLYGON_SMOOTH);
            }
            isSmooth = !isSmooth;
            break;
        case '5':  // color values from another iamge
            if (secondImageData) {
                if (!isColorFrom2ndImage) isColorFrom2ndImage = true;
                else isColorFrom2ndImage = false;
            }
            break;
        case '6': // texture mapping
            if (secondImageData) {
                if (isTextureMapping) {
                    isTextureMapping = false;
                    glDisable(GL_TEXTURE_2D);
                } else {
                    isTextureMapping = true;
                    glEnable(GL_TEXTURE_2D);
                }
            }
            break;
        case 'a':  // start/stop saving screenshots
            if(isScreenShot) isScreenShot = false;
            else isScreenShot = true;
            break;
        default:
            break;
    }
}

int main (int argc, char ** argv)
{
  if (argc<2) {
    printf ("usage: %s heightfield.jpg\n", argv[0]);
    //exit(1);
  }

  g_pHeightData = jpeg_read(argv[1], NULL);
  if (!g_pHeightData)
  {
    printf ("error reading %s.\n", argv[1]);
    exit(1);
  }
  
  if (argc >= 2) {
    secondImageData = jpeg_read(argv[2], NULL);
  }
    
  glutInit(&argc,argv);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
  glutInitWindowSize(640, 480);
  glutInitWindowPosition(0, 0);
  glutCreateWindow("Height Field");
  
  glutReshapeFunc(reshape);
  /* tells glut to use a particular display function to redraw */
  glutDisplayFunc(display);
  
  /* allow the user to quit using the right mouse button menu */
  g_iMenuId = glutCreateMenu(menufunc);
  glutSetMenu(g_iMenuId);
  glutAddMenuEntry("Quit",0);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
  
  /* replace with any animate code */
  glutIdleFunc(doIdle);

  /* callback for mouse drags */
  glutMotionFunc(mousedrag);
  /* callback for idle mouse movement */
  glutPassiveMotionFunc(mouseidle);
  /* callback for mouse button changes */
  glutMouseFunc(mousebutton);
  glutKeyboardFunc(keyboardFunctions);
  /* do initialization */
  myinit();

  glutMainLoop();
  return(0);
}
