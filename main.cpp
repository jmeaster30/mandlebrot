#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define _USE_MATH_DEFINES

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <chrono>
#include <string>

#include <GL/glut.h>


int win_width;
int win_height;

int max_iteration = 100;

int** screen;
bool update;

float zoom_factor = 1;
float zoom_offx = 0;
float zoom_offy = 0;

//camera variables
int fov = 45;
int near = 1;
int far = 2000;

float camerax = 0;
float cameray = 0;
float cameraz = 0;

float dx = 0;
float dy = 0;
float dz = 0;

float cameraMoveSpeed = 1.0f;
float cameraZoomSpeed = 2.0f;

float lookx = 0;
float looky = 0;
float lookz = 1;

float upx = 0;
float upy = 1;
float upz = 0;

//timing variables
std::chrono::steady_clock::time_point lastTime;
double ns = 1000000000.0 / 60.0;
double delta = 0;

int mousex = 0;
int mousey = 0;

float mouseGlobalX = 0.0f;
float mouseGlobalY = 0.0f;
float mouseGlobalZ = 0.0f;

void changeSize(int w, int h)
{
  if(h == 0) h = 1;
  float ratio = 1.0 * w / h;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glViewport(0, 0, w, h);
  gluPerspective(fov, ratio, near, far);

  glMatrixMode(GL_MODELVIEW);
}

void cSquare(float r1, float i1, float* r, float* i)
{
  *r = (r1 * r1) - (i1 * i1);
  *i = 2 * r1 * i1;
}

void cAdd(float r1, float i1, float r2, float i2, float* r, float* i)
{
  *r = r1 + r2;
  *i = i1 + i2;
}

int stability(float r, float i)
{
  float x = 0;
  float y = 0;
  float tx = 0;
  float ty = 0;
  int iteration = 0;
  while((x * x + y * y <= 2 * 2) && iteration < max_iteration)
  {
    cSquare(x, y, &tx, &ty);
    cAdd(tx, ty, r, i, &x, &y);
    iteration++;
  }
  return iteration;
}

void renderScene(void)
{
  //update
  if(update)
  {
    for(int x = 0; x < win_width; x++)
    {
      for(int y = 0; y < win_height; y++)
      {
        screen[x][y] = stability(((x * 4) / ((float)win_width * zoom_factor)) - (2 / zoom_factor) - zoom_offx,
                                 ((y * 4) / ((float)win_height * zoom_factor)) - (2 / zoom_factor) + zoom_offy);
      }
    }
    update = false;
  }

  //render
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //reset transformations
  glLoadIdentity();
  //set the camera
  gluLookAt(camerax        , cameray        , cameraz,
            camerax + lookx, cameray + looky, cameraz + lookz,
            upx            , upy            , upz);

  glBegin(GL_QUADS);
  for(int x = 0; x < win_width; x++)
  {
    for(int y = 0; y < win_height; y++)
    {
      float gs = screen[x][y] / (float)max_iteration;
      glColor3f(gs, gs, gs);
      glVertex3f(win_width - x, y, 0);
      glVertex3f(win_width - x, y + 1, 0);
      glVertex3f(win_width - (x + 1), y + 1, 0);
      glVertex3f(win_width - (x + 1), y, 0);
    }
  }
  glEnd();

  glutSwapBuffers();
}

void processMouse(int button, int state, int x, int y)
{
  switch(state)
  {
    case GLUT_DOWN:
      if(button == GLUT_LEFT_BUTTON)
      {
        //left click
      }
      else if(button == 3 || button == 4)
      {
        //std::cout << "Scroll " << ((button == 3) ? "Up" : "Down") << std::endl;
        zoom_factor += ((button == 3) ? 0.3 : - 0.3);
        zoom_offx = ((mouseGlobalX * 4) / ((float)win_width * zoom_factor)) - (2 / zoom_factor);
        zoom_offy = ((mouseGlobalY * 4) / ((float)win_height * zoom_factor)) - (2 / zoom_factor);
        update = true;
      }
      break;
    case GLUT_UP:
      if(button == GLUT_LEFT_BUTTON)
      {
        //left button releases
      }
      else if(button == 3 || button == 4)
      {
        //scroll stopped
      }
      break;
    default:
      break;
  }
}

void processMotion(int x, int y)
{
  mousex = x;
  mousey = y;
  //calculates the mouse global coordinate location
  GLint viewport[4];
  GLdouble modelview[16];
  GLdouble projection[16];
  GLfloat winx, winy;
  GLdouble worx, wory, worz;
  GLdouble worx1, wory1, worz1;

  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  glGetIntegerv(GL_VIEWPORT, viewport);

  winx = (float)x;
  winy = (float)viewport[3] - (float)y - 1;

  gluUnProject(winx, winy, 0.0f, modelview, projection, viewport, &worx, &wory, &worz);
  gluUnProject(winx, winy, 1.0f, modelview, projection, viewport, &worx1, &wory1, &worz1);

  float f = worz / (worz1 - worz);
  mouseGlobalX = worx - f * (worx1 - worx);
  mouseGlobalY = wory - f * (wory1 - wory);
}

int main(int argc, char** argv)
{
  const char* win_title = "Mandlebrot";//default: "Cellular Automata"

  win_width = 640;//default: 640
  win_height = 480;//default: 480

  screen = (int**)malloc(640 * sizeof(int*));
  for(int i = 0; i < 640; i++)
  {
    int* row = (int*)malloc(480 * sizeof(int));
    memset(row, 0, 480 * sizeof(int));
    screen[i] = row;
  }

  //initialize glut and create the Window
  glutInit(&argc, argv);
  glutInitWindowPosition(-1, -1); //window position doesn't matter
  glutInitWindowSize(win_width, win_height);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow(win_title);

  glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

  //register callbacks
  glutDisplayFunc(renderScene);
  glutReshapeFunc(changeSize);
  glutIdleFunc(renderScene); //when there is nothing to be processed call this function

  //far = cell_w * cell_size * 4;
  //set camera
  if(win_height == 0) win_height = 1;
  float ratio = 1.0 * win_width / win_height;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glViewport(0, 0, win_width, win_height);
  gluPerspective(fov, ratio, near, far);

  glMatrixMode(GL_MODELVIEW);

  camerax = win_width / 2.0f;
  cameray = win_height / 2.0f;
  cameraz = -500;

  //keyboard and mouse events
  glutMouseFunc(processMouse);
  glutMotionFunc(processMotion);
  glutPassiveMotionFunc(processMotion);

  glEnable(GL_DEPTH_TEST);

  //enter GLUT event processing cycle
  update = true;
  glutMainLoop();

  return 0;
}
