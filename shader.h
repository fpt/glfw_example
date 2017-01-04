#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>

//#include <OpenGL/gl3.h>

// Include GLEW. Always include it before gl.h and glfw.h, since it's a bit magic.
#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;


GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);

void DeleteShader(GLuint ProgramID);
