#include "utilities.h"

#include <fstream>

#include <string>
#include <vector>

#include <iostream>

std::string loadStringFromFile(const char* fileName) {
	std::string text = "";

	std::ifstream file(fileName);
	text.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	return text;
}

GLuint buildShaderFromFile(const GLenum type, const char* fileName) {
	std::string source = loadStringFromFile(fileName);

	GLuint shader = glCreateShader(type);
	const GLchar* shaderSource = source.c_str();
	glShaderSource(shader, 1, (const GLchar**)&shaderSource, NULL);
	glCompileShader(shader);

	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<char> errorLog(maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		for (auto c : errorLog) {
			std::cout << c;
		}
		std::cout << std::endl;

		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

GLuint buildShader(const GLenum type, const char* source) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, 0);
	glCompileShader(shader);

	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<char> errorLog(maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		for (auto c : errorLog) {
			std::cout << c;
		}
		std::cout << std::endl;

		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

GLuint buildProgram(const GLuint vertexShader, const GLuint fragmentShader) {
	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	return program;
}

GLuint buildProgram(const GLuint vertexShader, const GLuint geometryShader, const GLuint fragmentShader) {
	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, geometryShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	int isLinked;
	glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
	if (isLinked == GL_FALSE) std::cout << "Program link failed" << std::endl;

	return program;
}

GLuint build3DTexture(const GLenum iformat, const GLenum format, const int width, const int height, const int depth, GLenum type, GLvoid* data, GLenum wrap, GLenum filter) {
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, texture);

	glTexImage3D(GL_TEXTURE_3D, 0, iformat, width, height, depth, 0, format, type, data);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrap);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrap);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, filter);

	return texture;
}

GLuint build2DTexture(const GLenum const iformat, const GLenum format, const GLsizei width, const GLsizei height, const GLenum type, const GLvoid* data, const GLenum wrap, const GLenum filter) {
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, iformat, width, height, 0, format, type, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

	return texture;
}

void hsvToRGB(float h, float s, float v, float& r, float& g, float& b) {
	h = fmod(h, 1.0);

	float c = v * s;

	float hDash = h * 6;

	float x = c * (1 - abs(fmod(hDash, 2.0) - 1));

	int mod = floor(hDash);

	float arrayR[6] = { c, x, 0, 0, x, c };
	float arrayG[6] = { x, c, c, x, 0, 0 };
	float arrayB[6] = { 0, 0, x, c, c, x };

	r = arrayR[mod];
	g = arrayG[mod];
	b = arrayB[mod];

	float m = v - c;

	r += m;
	g += m;
	b += m;
};

void makePerspectiveMatrix (float* matrix, float fov, float aspect, float near, float far) {
	float f = tan(0.5 * (3.14159265 - fov)),
		range = near - far;

	matrix[0] = f / aspect;
	matrix[1] = 0;
	matrix[2] = 0;
	matrix[3] = 0;
	matrix[4] = 0;
	matrix[5] = f;
	matrix[6] = 0;
	matrix[7] = 0;
	matrix[8] = 0;
	matrix[9] = 0;
	matrix[10] = far / range;
	matrix[11] = -1;
	matrix[12] = 0;
	matrix[13] = 0;
	matrix[14] = (near * far) / range;
	matrix[15] = 0.0;
};

float* createEmptyArray(int size) {
	float* emptyData = new float[size];
	for (int i = 0; i < size; ++i) {
		emptyData[i] = 0.0;
	}
	return emptyData;
}