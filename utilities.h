#include "glew.h"
#include "glfw3.h"

#include <string>

std::string loadStringFromFile(const char* fileName);
GLuint buildShaderFromFile(const GLenum type, const char* fileName);
GLuint buildShader(const GLenum type, const char* source);
GLuint buildProgram(GLuint vertexShader, GLuint fragmentShader);
GLuint buildProgram(const GLuint vertexShader, const GLuint geometryShader, const GLuint fragmentShader);
GLuint build3DTexture(const GLenum iformat, const GLenum format, const GLsizei width, const GLsizei height, const GLsizei depth, const GLenum type, GLvoid* data, GLenum wrap, GLenum filter);
GLuint build2DTexture(const GLenum iformat, const GLenum format, const GLsizei width, const GLsizei height, const GLenum type, const GLvoid* data, const GLenum wrap, const GLenum filter);
float* createEmptyArray(int size);
void hsvToRGB(float h, float s, float v, float& r, float& g, float& b);
void makePerspectiveMatrix(float* matrix, float fov, float aspect, float near, float far);