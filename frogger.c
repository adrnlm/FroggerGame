#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#if _WIN32
#   include <Windows.h>
#endif
#if __APPLE__
#   include <OpenGL/gl.h>
#   include <OpenGL/glu.h>
#   include <GLUT/glut.h>
#else
#   include <GL/gl.h>
#   include <GL/glu.h>
#   include <GL/glut.h>
#endif

#define DEG2RAD  M_PI/180.0
#define GRAVITY -0.25
#define MILLI 1000
#define INITIAL_COORD_X 8.5
#define INITIAL_COORD_Y 0
#define INITIAL_COORD_Z 0
#define CAR_LANE_1 3.5
#define CAR_LANE_2 4.5
#define CAR_LANE_3 5.5
#define CAR_LANE_4 6.5
#define LOG_RADIUS 0.25
#define LOG_LENGTH 2


typedef struct { float A; float k; float w; } Sinewave;
typedef struct {float x, y, z;} vec3f;
typedef struct {float x, y, z, row, column;} land_v;

typedef struct {
  vec3f currentCoord;
  float length;
  float height;
  float width;
} car_val;

typedef struct {
  vec3f currentCoord;
  vec3f nextCoord;
  float rotation;
} frog_val;

typedef struct {
  land_v beforeWater;
  land_v leftWaterWall;
  land_v underWater;
  land_v rightWaterWall;
  land_v afterWater;
  land_v leftRoadWall;
  land_v road;
  land_v rightRoadWall;
  land_v afterRoad;
} land_t;

typedef struct {
  vec3f coord;
  Sinewave wave;
  float width;
  float length;
} water_val;

typedef struct {
  vec3f coord;
  float radius;
  float length;
} log_val;

typedef struct {
  // CAMERA ROTATION
  float rotY;
  float rotX;
  float lastX;
  float lastY;
  // MOUSE FUNCTIONS
  bool lmd;
  bool rmd;
  // RENDER FUNCTIONS
  bool lighting;
  bool filled;
  bool axes;
  bool normals;
  bool go;
  // CAMERA
  float camera_x;
  float camera_z;
  float zoom;
  // VARIABLES
  float time;
  int tess;
  int row;
  int column;
  float scale;
  int nCars;
  int nLogs;
} global_val;


global_val global = {
  // CAMERA ROTATION
  0.0, 0.0, 0.0, 0.0,
  // MOUSE FUNCTIONS
  false, false,
  // KEYBOARD VARIABLES
  false, false, true, true, false,
  // CAMERA
  0.0, 0.0, 5,
  // VARIABLES
  0.0,
  8,
  10, 2,
  0.5,
  16,
  2
};

Sinewave sw = { 0.2, (2*M_PI)/2.0, 2.0 };

land_t land = {
  {-6.5, 0, 0, 10, 1.5},
  {0.5, -5, 0, 10, .5},
  {-2.5, -.75, 0, 10, 2.5},
  {-0.5, 0, 0, 10, .5},
  {1.5, 0, 0, 10, 1.5},
  {0.5, -3, 0, 10, .5},
  {5, 0.2, 0, 10, 2},
  {-0.5, 7, 0, 10, .5},
  {8.5, 0, 0, 10, 1.5}
};

water_val water = {
  {-2.5, -.25, 0},
  { 0.2, (2*M_PI)/2.0, 2.0 },
  10, 2.5
};

car_val cars[] = {
  {{CAR_LANE_1, 0.45, -9}, 0.5, 0.4, 0.7}, // Car 1
  {{CAR_LANE_1, 0.45, -6}, 0.5, 0.4, 0.7}, // Car 2
  {{CAR_LANE_1, 0.45, 3}, 0.5, 0.4, 0.7}, // Car 3
  {{CAR_LANE_1, 0.45, 7}, 0.5, 0.4, 0.7}, // Car 4
  {{CAR_LANE_2, 0.45, -8}, 0.5, 0.4, 0.7}, // Car 5
  {{CAR_LANE_2, 0.45, -4}, 0.5, 0.4, 0.7}, // Car 6
  {{CAR_LANE_2, 0.45, 0}, 0.5, 0.4, 0.7}, // Car 7
  {{CAR_LANE_2, 0.45, 5}, 0.5, 0.4, 0.7}, // Car 8
  {{CAR_LANE_3, 0.45, -5}, 0.5, 0.4, 0.7}, // Car 9
  {{CAR_LANE_3, 0.45, -1}, 0.5, 0.4, 0.7}, // Car 10
  {{CAR_LANE_3, 0.45, 6}, 0.5, 0.4, 0.7}, // Car 11
  {{CAR_LANE_3, 0.45, 9}, 0.5, 0.4, 0.7}, // Car 12
  {{CAR_LANE_4, 0.45, -3}, 0.5, 0.4, 0.7},  // Car 13
  {{CAR_LANE_4, 0.45, 1}, 0.5, 0.4, 0.7},  // Car 14
  {{CAR_LANE_4, 0.45, 4}, 0.5, 0.4, 0.7},  // Car 15
  {{CAR_LANE_4, 0.45, 7}, 0.5, 0.4, 0.7}  // Car 16
};

log_val logs[] = {
  {{-2, -0.25, 2}, LOG_RADIUS, LOG_LENGTH},
  {{-4, -0.25, -2}, LOG_RADIUS, LOG_LENGTH}
};

frog_val frog = {
  {INITIAL_COORD_X, INITIAL_COORD_Y, INITIAL_COORD_Y},
  {0, 0, 0},
  0
};


// +++++++++++++++++++++++++++ DRAW FUNCTION ++++++++++++++++++++++++++++++

void drawAxes(float length){
  if (global.axes){
    glPushMatrix();
      glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT);
        glLineWidth(2.5);

        glBegin(GL_LINES);

          GLfloat shiny[] = { 128 }; // SHINY!

          GLfloat red[] = { 1.0, 0.0, 0.0 }; // MATERIAL RGBA
          glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, red);
          glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, red);
          glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, red);
          glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
          glColor3f(1, 0, 0); //red_X
          glVertex3f(0, 0, 0);
          glVertex3f(length, 0.0, 0.0);

          GLfloat green[] = { 0.0, 1.0, 0.0 }; // MATERIAL RGBA
          glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, green);
          glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, green);
          glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, green);
          glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
          glColor3f(0, 1, 0);//green_Y
          glVertex3f(0, 0, 0);
          glVertex3f(0.0, length, 0.0);

          GLfloat blue[] = { 0.0, 0.0, 1.0 }; // MATERIAL RGBA
          glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, blue);
          glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, blue);
          glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, blue);
          glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
          glColor3f(0, 0, 1);//blue_Z
          glVertex3f(0, 0, 0);
          glVertex3f(0.0, 0.0, length);

        glEnd();
      glPopAttrib();
    glPopMatrix();
  }
}

void drawNormal(float x, float y, float z,
  float xx, float yy, float zz, bool curve, Sinewave wave, float t, float s){
  if (global.axes){
    if (curve){

      float dx = 1.0;
      float dxx = 1.0;
      float angle_x = wave.k * x + wave.w * (t/2);
      float angle_xx = wave.k * xx + wave.w * (t/2);
      float dy = wave.k * wave.A * cosf(angle_x);
      float dyy = wave.k * wave.A * cosf(angle_xx);

      float mag_x = sqrtf(dx * dx + dy * dy);
        dx /= mag_x;
        dy /= mag_x;
      float mag_xx = sqrtf(dxx * dxx + dyy * dyy);
        dxx /= mag_xx;
        dyy /= mag_xx;

      glPushAttrib(GL_CURRENT_BIT);
        GLfloat color[] = { 1.0, 1.0, 0.0 }; // MATERIAL RGBA
        GLfloat shiny[] = { 128 }; // SHINY!
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shiny);

        glBegin(GL_LINES);
          glColor3f(1, 1, 0);
          glVertex3f(x, y, z);
          glVertex3f(x+global.scale*-dy, y+global.scale*dx, z);

          glVertex3f(x, y, zz);
          glVertex3f(x+global.scale*-dy, y+global.scale*dx, zz);
        glEnd();
      glPopAttrib();
    }
    else{
      glPushAttrib(GL_CURRENT_BIT);
      GLfloat color[] = { 1.0, 1.0, 0.0 }; // MATERIAL RGBA
      glColor3f(1, 1, 0); //YELLOW
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);
      glBegin(GL_LINES);
      glNormal3f(x, y, z); // Normal
      glVertex3f(x, y, z);
      glVertex3f(x, y+s, z);
      glEnd();
      glPopAttrib();
    }
  }
}

void drawEllipsoid(frog_val frog){

  float ix = frog.currentCoord.x;
  float iy = frog.currentCoord.y;
  float iz = frog.currentCoord.z;


  float uiStacks = global.tess;
  float uiSlices = global.tess;

  // Scale
  float x = 0.2;
  float y = 0.1;
  float z = 0.1;

  glPushMatrix();
    glTranslatef(ix,iy,iz);
    glRotatef(180,0,1,0);
    glRotatef(frog.rotation,0,1,0);
    drawAxes(0.5);

    if (global.filled){
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else{
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    GLfloat color[] = { 1, 0, 1 }; // MATERIAL RGBA
    GLfloat shiny[] = { 128 }; // SHINY!
    glPushAttrib(GL_CURRENT_BIT);
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
      glColor3f(1, 0, 1); // RED
      glBegin(GL_TRIANGLE_STRIP);

        float tStep = (M_PI) / (float)uiSlices;
      	float sStep = (M_PI) / (float)uiStacks;

        for(float t = -M_PI/2; t <= (M_PI/2)+.0001; t += tStep) {
      		for(float s = -M_PI; s <= M_PI+.0001; s += sStep) {
            float x1 = x * cosf(t) * cosf(s);
            float y1 = y * cosf(t) * sinf(s);
            float z1 = z * sinf(t);
            float x2 = x * cosf(t+tStep) * cosf(s);
            float y2 = y * cosf(t+tStep) * sinf(s);
            float z2 = z * sinf(t+tStep);

            glNormal3f(x1, y1, z1);
      			glVertex3f(x1, y1, z1);
            glNormal3f(x2, y2, z2);
      			glVertex3f(x2, y2, z2);
      		}
      	}
      glEnd();
    glPopAttrib();
  glPopMatrix();
}

void drawGrid(land_v grid){

  float ix = grid.x;
  float iy = grid.y;
  float iz = grid.z;
  float row = grid.row;
  float column = grid.column;

  glPushMatrix();
    glTranslatef(ix,iy,iz);
    drawAxes(1.0);


    for (float z = row; z>(-row); z-= 1){
      for (float x = -column; x<column; x+= 1){
        float y=0;

        if (global.filled){
          glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else{
          glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        glPushAttrib(GL_CURRENT_BIT);
        GLfloat color[] = { 0.0, 1.0, 0.0 }; // MATERIAL RGBA
        GLfloat shiny[] = { 128 }; // SHINY!
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
        glColor3f(0,1,0); // RGB-GREEN

        glBegin(GL_TRIANGLES);

        glNormal3f(x, y, z); // Normal
        glVertex3f(x, y, z);
        glNormal3f(x+1, y, z); // Normal
        glVertex3f(x+1, y, z);
        glNormal3f(x+1, y, z-1); // Normal
        glVertex3f(x+1, y, z-1);

        glNormal3f(x, y, z-1); // Normal
        glVertex3f(x, y, z-1);
        glNormal3f(x, y, z); // Normal
        glVertex3f(x, y, z);
        glNormal3f(x+1, y, z-1); // Normal
        glVertex3f(x+1, y, z-1);

        glEnd();
        glPopAttrib();
        // glEnd();

        drawNormal(x, y, z, x, y, z, false, sw, global.time, global.scale);
        drawNormal(x+1, y, z, x, y, z, false, sw, global.time, global.scale);
        drawNormal(x+1, y, z, x, y, z-1, false, sw, global.time, global.scale);
        drawNormal(x, y, z, x, y, z-1, false, sw, global.time, global.scale);
      }
    }
  glPopMatrix();
}

void drawSin(water_val water) {

  float ix = water.coord.x;
  float iy = water.coord.y;
  float iz = water.coord.z;

  float width = water.width;
  float length = water.length;

  float ampere = water.wave.A;
  float wavelength = water.wave.w;
  float k = water.wave.k;

  glPushMatrix();
    glTranslatef(ix,iy,iz);
    drawAxes(1.0);


    float x_segment = 2* length/global.tess;
    float z_segment = 2* width/global.tess;

    for ( float x= -length; x<length; x+=(x_segment)) {
      float y = ampere * sin(k*x+global.time);
      float xx = x + (x_segment);
      float yy = ampere * sin(k*(xx)+global.time);
      for(float z =-width; z<width; z+=(z_segment)){
        float zz = z + (z_segment);

        if (global.filled){
          glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else{
          glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        glPushAttrib(GL_CURRENT_BIT);
          GLfloat color[] = { 0.0, 1.0, 1.0 }; // MATERIAL RGBA
          GLfloat shiny[] = { 128 }; // SHINY!
          glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
          glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);
          glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);
          glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shiny);

          glBegin(GL_TRIANGLES);
            glColor3f(0,1,1);

            glNormal3f(x, y, z);
            glVertex3f(x, y, z);
            glNormal3f(xx, yy, z);
            glVertex3f(xx, yy, z);
            glNormal3f(xx, yy, zz);
            glVertex3f(xx, yy, zz);

            glNormal3f(xx, yy, zz);
            glVertex3f(xx, yy, zz);
            glNormal3f(x, y, z);
            glVertex3f(x, y, z);
            glNormal3f(x, y, zz);
            glVertex3f(x, y, zz);

          glEnd();
        glPopAttrib();

        drawNormal(x, y, z, xx, yy, zz, true, water.wave, global.time, global.scale);

      }
    }
  glPopMatrix();
}

void drawRect(car_val car){

  float length = car.length/2;
  float width = car.width/2;
  float height = car.height/2;

  glPushMatrix();
    glTranslatef(car.currentCoord.x, car.currentCoord.y, car.currentCoord.z);
    drawAxes(0.5);
    glPushAttrib(GL_CURRENT_BIT);
      if (global.filled)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      else
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

      GLfloat color[] = { 1.0, 0.0, 0.0 }; // MATERIAL RGBA
      GLfloat shiny[] = { 128 }; // SHINY!
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shiny);

      glBegin(GL_TRIANGLES);
      glColor3f(1,0,0);
        // Bottom 1
        glNormal3f(-length, -height, -width);
        glVertex3f(-length, -height, -width);
        glNormal3f(length, -height, -width);
        glVertex3f(length, -height, -width);
        glNormal3f(length, -height, width);
        glVertex3f(length, -height, width);
        // Bottom 2
        glNormal3f(-length, -height, -width);
        glVertex3f(-length, -height, -width);
        glNormal3f(length, -height, width);
        glVertex3f(length, -height, width);
        glNormal3f(-length, -height, width);
        glVertex3f(-length, -height, width);

        // Top 1
        glNormal3f(-length, height, width);
        glVertex3f(-length, height, width);
        glNormal3f(length, height, width);
        glVertex3f(length, height, width);
        glNormal3f(length, height, -width);
        glVertex3f(length, height, -width);
        // Top 2
        glNormal3f(-length, height, width);
        glVertex3f(-length, height, width);
        glNormal3f(length, height, -width);
        glVertex3f(length, height, -width);
        glNormal3f(-length, height, -width);
        glVertex3f(-length, height, -width);

        // Right 1
        glNormal3f(length, -height, width);
        glVertex3f(length, -height, width);
        glNormal3f(length, -height, -width);
        glVertex3f(length, -height, -width);
        glNormal3f(length, height, -width);
        glVertex3f(length, height, -width);
        // Right 2
        glNormal3f(length, -height, width);
        glVertex3f(length, -height, width);
        glNormal3f(length, height, -width);
        glVertex3f(length, height, -width);
        glNormal3f(length, height, width);
        glVertex3f(length, height, width);

        // Left 1
        glNormal3f(-length, -height, -width);
        glVertex3f(-length, -height, -width);
        glNormal3f(-length, -height, width);
        glVertex3f(-length, -height, width);
        glNormal3f(-length, height, width);
        glVertex3f(-length, height, width);
        // Left 2
        glNormal3f(-length, -height, -width);
        glVertex3f(-length, -height, -width);
        glNormal3f(-length, height, width);
        glVertex3f(-length, height, width);
        glNormal3f(-length, height, -width);
        glVertex3f(-length, height, -width);

        // Back 1
        glNormal3f(length, -height, -width);
        glVertex3f(length, -height, -width);
        glNormal3f(-length, -height, -width);
        glVertex3f(-length, -height, -width);
        glNormal3f(-length, height, -width);
        glVertex3f(-length, height, -width);
        // Back 2
        glNormal3f(length, -height, -width);
        glVertex3f(length, -height, -width);
        glNormal3f(-length, height, -width);
        glVertex3f(-length, height, -width);
        glNormal3f(length, height, -width);
        glVertex3f(length, height, -width);

        // Front 1
        glNormal3f(-length, -height, width);
        glVertex3f(-length, -height, width);
        glNormal3f(length, -height, width);
        glVertex3f(length, -height, width);
        glNormal3f(length, height, width);
        glVertex3f(length, height, width);
        // Front 2
        glNormal3f(-length, -height, width);
        glVertex3f(-length, -height, width);
        glNormal3f(length, height, width);
        glVertex3f(length, height, width);
        glNormal3f(-length, height, width);
        glVertex3f(-length, height, width);

      glEnd();
    glPopAttrib();
  glPopMatrix();
}

void drawCars(){
  for ( int i=0; i<global.nCars; i++){
    drawRect(cars[i]);
  }
}

void drawCyl(log_val log){
  float radius = log.radius;
  float halfLength = log.length/2;
  int slices = global.tess;
  float pi2 = 2*M_PI;

  glPushMatrix();
    glTranslatef(log.coord.x, log.coord.y, log.coord.z);
    drawAxes(0.5);
    glPushAttrib(GL_CURRENT_BIT);
    if (global.filled)
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    for(int i=0; i<slices; i++) {
      float x = radius * cosf(i*(pi2/slices));
      float y = radius * sinf(i*(pi2/slices));
      float xx = radius * cosf((i+1)*(pi2/slices));
      float yy = radius * sinf((i+1)*(pi2/slices));


      GLfloat color[] = { 0.09, 0.41, 0.59 }; // MATERIAL RGBA BROWN
      GLfloat shiny[] = { 128 }; // SHINY!
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shiny);

      glBegin(GL_TRIANGLES);
      glColor3f(0.09, 0.41, 0.59);
        glNormal3f(0,0,halfLength);
        glVertex3f(0,0,halfLength);
        glNormal3f(x,y,halfLength);
        glVertex3f(x,y,halfLength);
        glNormal3f(xx,yy,halfLength);
        glVertex3f(xx,yy,halfLength);

        glNormal3f(0,0,-halfLength);
        glVertex3f(0,0,-halfLength);
        glNormal3f(x,y,-halfLength);
        glVertex3f(x,y,-halfLength);
        glNormal3f(xx,yy,-halfLength);
        glVertex3f(xx,yy,-halfLength);

        glNormal3f(x,y,halfLength);
        glVertex3f(x,y,halfLength);
        glNormal3f(xx,yy,halfLength);
        glVertex3f(xx,yy,halfLength);
        glNormal3f(xx,yy,-halfLength);
        glVertex3f(xx,yy,-halfLength);

        glNormal3f(x,y,halfLength);
        glVertex3f(x,y,halfLength);
        glNormal3f(xx,yy,-halfLength);
        glVertex3f(xx,yy,-halfLength);
        glNormal3f(x,y,-halfLength);
        glVertex3f(x,y,-halfLength);
      glEnd();
    }
    glPopAttrib();
  glPopMatrix();
}

void drawLogs(){
  for ( int i=0; i<global.nLogs; i++){

    drawCyl(logs[i]);
  }
}
// +++++++++++++++++++++++++++ IDLE FUNCTION ++++++++++++++++++++++++++++++++

void idle(){
  static float lastT = -1.0;
  float t, dt;

  t = glutGet(GLUT_ELAPSED_TIME) / (float)MILLI;

  if (lastT < 0.0){
    lastT = t;
    return;
  }

  dt = t - lastT;
  lastT = t;

  if(dt<=0){
    return;
  }

  glutPostRedisplay();

  global.time = t;

}

// +++++++++++++++++++++++++++ MOUSE FUNCTION ++++++++++++++++++++++++++++++++
void mouse(int button, int state, int x, int y){
  global.lastX = x;
  global.lastY = y;
  if (button == GLUT_LEFT_BUTTON)
    global.lmd = (state == GLUT_DOWN);
  if (button == GLUT_RIGHT_BUTTON)
    global.rmd = (state == GLUT_DOWN);

  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
  {
    printf("Left Mouse Button\n" );
  }
  else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
  {
    printf("Right Mouse Button\n" );
  }

}

void mouseMotion(int x, int y){
  if (global.lmd){
    global.rotY += x - global.lastX;
    global.rotX += y - global.lastY;
  }
  if (global.rmd){
    global.zoom += (y - global.lastY)/100;
  }
  global.lastX = x;
  global.lastY = y;

}

// +++++++++++++++++++++++++++ DISPLAY ++++++++++++++++++++++++++++++++

void init(){
  /* In this program these OpenGL calls only need to be done once,
    but normally they would go elsewhere, e.g. display */

  // glMatrixMode(GL_PROJECTION);
  // glOrtho(-2.0, 2.0, -2.0, 2.0, -2.0, 2.0);
  // glMatrixMode(GL_MODELVIEW);

  if (global.lighting){
    glEnable(GL_LIGHTING);
  }
  else{
    glDisable(GL_LIGHTING);
  }

  if (global.normals){
    glEnable(GL_NORMALIZE);
  }
  else{
    glDisable(GL_NORMALIZE);
  }

  GLfloat mat_specular[] = { 1, 1, 1, 1 }; // RGBA
  GLfloat mat_ambience[] = { 1, 1, 1, 1 }; // RGBA
  GLfloat mat_diffuse[] = { .8, .8, .8, 1 }; // RGBA
  GLfloat mat_shininess[] = { 128 }; // Range from 0 -> 128
  GLfloat light_position[] = { 1, 1, 1, 0 };

  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambience);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

  glEnable(GL_LIGHT0);
}

void display(){

  init();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  /* Put drawing code here */
  // ...

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(75, 1, 0.01, 100);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0, 0, -global.zoom); // CAMERA
  glRotatef(global.rotY, 0, 1, 0);
  glRotatef(global.rotX, 1, 0, 0);
  // glTranslatef(-INITIAL_COORD_X + global.camera_x, -INITIAL_COORD_Y, INITIAL_COORD_Z+global.camera_z);
  glTranslatef(global.camera_x, 0, global.camera_z);

  drawAxes(5);

  // LAND
  glPushMatrix();
    drawGrid(land.beforeWater); // BEFORE WATER
      glPushMatrix();
      glRotatef(-90, 0, 0, 1);
      glScalef(0.75,1,1);
      drawGrid(land.leftWaterWall); // LEFT WATER WALL
      glPopMatrix();
    drawGrid(land.underWater); // UNDER WATER
      glPushMatrix();
      glRotatef(90, 0, 0, 1);
      glScalef(0.75,1,1);
      drawGrid(land.rightWaterWall); // RIGHT WATER WALL
      glPopMatrix();
    drawGrid(land.afterWater); // AFTER WATER BEFORE ROAD
      glPushMatrix();
      glRotatef(90, 0, 0, 1);
      glScalef(0.2,1,1);
      drawGrid(land.leftRoadWall); // LEFT ROAD WALL
      glPopMatrix();
    drawGrid(land.road); // ROAD
      glPushMatrix();
      glRotatef(-90, 0, 0, 1);
      glScalef(0.2,1,1);
      drawGrid(land.rightRoadWall); // RIGHT ROAD WALL
      glPopMatrix();
    drawGrid(land.afterRoad); // AFTER ROAD
  glPopMatrix();

  // WATER
  drawSin(water);

  // FROG
  glPushMatrix();
    glTranslatef(0.0, 0.1, 0.0);
    drawEllipsoid(frog);
  glPopMatrix();

  // CARS
  drawCars();

  // LOGS
  drawLogs();

  glutPostRedisplay();
  /* Always check for errors! */
  int err;
  while ((err = glGetError()) != GL_NO_ERROR)
    printf("display: %s\n", gluErrorString(err));

  glutSwapBuffers();
}

// +++++++++++++++++++++++++++ KEYBOARD FUNCTION ++++++++++++++++++++++++++++++

void SpecialInput(int key, int x, int y){
  switch(key)
  {
    case GLUT_KEY_UP:
      //do something here
      printf("UP\n");

      // if(row < 10)
      //   row += 1;
      global.camera_z += .2;
      break;
    case GLUT_KEY_DOWN:
      //do something here
      printf("DOWN\n");

      // if(row > 1)
      //   row -= 1;
      global.camera_z -= .2;
      break;
    case GLUT_KEY_LEFT:
      //do something here
      printf("LEFT\n");

      // if(column > 1)
      //   column -= 1;
      global.camera_x += .2;
      break;
    case GLUT_KEY_RIGHT:
      //do something here
      printf("RIGHT\n");

      // if(column < 10)
      //   column += 1;
      global.camera_x -= .2;
      break;
  }
  glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y){
  switch (key)
  {
  case 'a':
    printf("key: a\n");
    frog.rotation += 5;
    break;
  case 'd':
    printf("key: d\n");
    frog.rotation -= 5;
    break;
  case 'l':
    printf("key: l\n");
    global.lighting = !global.lighting;
    break;
  case '-':
    printf("key: -\n");
      if(global.tess > 4)
        global.tess = global.tess/2;
    break;
  case '=':
    printf("key: +\n");
      if(global.tess < 32)
        global.tess = global.tess*2;
    break;
  case 'm':
    printf("key: m\n");
    global.filled = !global.filled;
    break;
  case 'n':
    printf("key: n\n");
    global.normals = !global.normals;
    break;
  case 'x':
    printf("key: x\n");
    global.axes = !global.axes;
    break;
  case 'q':
    exit(EXIT_SUCCESS);
    break;
  default:
    break;
  }
}

// +++++++++++++++++++++++++++ MAIN FUNCTION ++++++++++++++++++++++++++++++

int main(int argc, char **argv){
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowPosition(25, 50);
  glutInitWindowSize(700, 700);
  glutCreateWindow("Frogger | Assignment 2");

  // init();

  glutIdleFunc(idle);
  glutDisplayFunc(display);
  glutSpecialFunc(SpecialInput);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(mouseMotion);
  glutMainLoop();

  return EXIT_SUCCESS;
}