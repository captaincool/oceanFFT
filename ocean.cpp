#include <stdlib.h>
#include <iostream>

#include <GL/glew.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "shader-load.h"

// Rotation angles
GLint phi = 0;
GLint theta = 0;

int size = 4;
int zoom = 0;
int height = 0;

// Shader
GLuint program;
bool program_on = true;
GLint time_var, resolution_var;  // handles to the "time" and "resolution" shader variables
float time_count = 0;   // value of the "time" shader variable

// Card buffer IDs
GLuint bufferIds[2];

// Display function - draw a teapot
void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Modeling transformation
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0.0, 0.0 + height, -20.0 + zoom, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
  glTranslatef(0.0, 0.0, -3);
  glRotated(phi,1,0,0);
  glRotated(theta,0,1,0);
  
  // Provide both material and color so shaders can use either
  GLfloat color[4] = {.8,.4,.1,0};
  GLfloat specular[4] = {1,1,1,0};
  glColor4fv(color);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,color);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,specular);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS,20);
  
  // Send the time variable to the shader
  glUniform1f(time_var,time_count);

  glBindBufferARB(GL_ARRAY_BUFFER_ARB, bufferIds[0]);
  glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, bufferIds[1]);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, 0);

  glDrawElements(GL_TRIANGLE_STRIP, size*size*6, GL_UNSIGNED_BYTE, 0);

  glDisableClientState(GL_VERTEX_ARRAY);

  glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
  glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

  //glBegin(GL_POLYGON);
    //glNormal3f(0.0, 1.0, 0.0);
    //for (int i = -64; i < 64; i++) {
        //for (int j = -64; j < 64; j++) {
            //glVertex3f(-1+i, -5.0, -1+j);
            //glVertex3f(-1+i, -5.0, 1+j);
            //glVertex3f(1+i, -5.0, 1+j);
            //glVertex3f(1+i, -5.0, -1+j);
        //}
    //}
  //glEnd();
  
  glutSwapBuffers();
}

void keyboard (unsigned char key, int x, int y)
{
  switch (key) {
  case 'j':
    theta -= 2;
    break;
  case 'l':
    theta += 2;
    break;
  case 'i':
    phi -= 2;
    break;
  case 'k':
    phi += 2;
    break;
  case 's':
    if (program_on)
      glUseProgram(0);
    else
      glUseProgram(program);
    program_on = !program_on;
    break;
  case '+':
    zoom++;
    break;
  case '-':
    zoom--;
    break;
  case 'u':
    height++;
    break;
  case 'o':
    height--;
    break;
  case 'q':
    exit(0);
  default:
    return;
  }
  x = y;
  glutPostRedisplay();
}

// Window resize
void reshape(int w, int h)
{
  // Inform shader program of new resolution
  // by setting "resolution" variable
  glUniform2f(resolution_var,w,h);

  // Set up view and projection
  glViewport(0,0,w,h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0, (GLfloat) w / (GLfloat) h, 0.1, 200.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

// Advance time
void idle(void)
{
  time_count += .02;
  glutPostRedisplay();
}

// Fixed quad map length and width, regardless of square resolution
float sizeX = 40.0f, sizeZ = 40.0f;

// Generates a square vertex map for the water and copies it to the graphics card
void buildWater(int size) {
    int vertSize = size*size*3;
    // number of triangles + 3 for each row (2 for first triangle, 1 for row end marker) - 1 for overcount
    int indSize = (size*size*2) + (3 * size) - 1;

    GLfloat* verticies = new GLfloat[vertSize];
    GLuint* indicies = new GLuint[indSize];
    // Calculate vertex positions from defined size limits
    // This will fit specified square count in defined size
    for (int i = 0; i < vertSize; i += 3) {
        int curVert = i / 3;
        // determine what row we're on and generate square location
        float x = float(curVert % size);
        float z = float(curVert / size);
        // generate vertex within some portion of coordinate boundaries
        verticies[i] = -sizeX / 2 + sizeX*x / float(size - 1);
        verticies[i+1] = -5; // Fixed height, real height controlled by shader
        verticies[i+2] = sizeZ / 2 + sizeZ*z / float(size - 1);
    }

    
    for (int j = 0; j < size; j++) {
        for (int k = 0; k < (size*2) + 2; k++) {
            
        }
    }

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, bufferIds[0]);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, vertSize, verticies, GL_STATIC_DRAW_ARB);

    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, bufferIds[1]);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indSize, indicies, GL_STATIC_DRAW_ARB);

    delete[] verticies;
    delete[] indicies;
}

void init() {
  // Depth test
  glEnable(GL_DEPTH_TEST);
  // Simple Lighting
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_PRIMITIVE_RESTART);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glGenBuffersARB(2, bufferIds);
}

// Main code - all initialization is here
int main(int argc, char** argv)
{
  // Initialize OpenGL
  glutInit(&argc, argv);
  
  // Check args
  char *shaderfile;
  if (argc != 2) {
    std::cerr << "usage: shader-test shadername\n";
    exit(1);
  }
  shaderfile = argv[1];

  // Make window
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutCreateWindow("Shader Tester");

  // Initialize extensions manager and set up shaders
  glewInit();
  program = loadShader(shaderfile);  // defined in the shader-load module
  glUseProgram(program);

  // Uniform shader variables to pass UI information to the shaders
  resolution_var = glGetUniformLocation(program,"resolution");
  time_var = glGetUniformLocation(program,"time");
  init();
  buildWater(size);

  
  // Attach event handlers
  glutKeyboardFunc(keyboard);
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  glutReshapeFunc(reshape);
  glutMainLoop();
  glDeleteBuffersARB(2, bufferIds);
  return 0; 
}