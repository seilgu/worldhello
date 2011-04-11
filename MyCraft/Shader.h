
#ifndef _SHADER_H_
#define _SHADER_H_

#ifndef APPLE
#include <Windows.h>
#include "glew.h"
#else
#include <GL/glew.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")
#ifndef APPLE
#include "wglew.h"
#endif

#include "common.h"

class Shader {
private:
	class shader_program {
	private:
		char *FileRead(char *filename) {
			FILE *fp = 0;
			fopen_s(&fp, filename, "rt");
			if (fp == 0) {
				MessageBox(0, "CAN'T OPEN FILE", "HAA", 0);
				return 0;
			}

			int count = 0;
			fseek(fp, 0, SEEK_END);
			count = ftell(fp);
			rewind(fp);

			char *content = 0;
			if (count > 0) {
				content = new char[count+1];
				if (content == 0) {
					MessageBox(0, "char *alloc failed", "haha", 0);
					fclose(fp);
					return 0;
				}
				count = fread(content, sizeof(char), count, fp);
				content[count] = '\0';
			}

			fclose(fp);

			return content;
		}

	public:
		GLuint program;
		GLuint shaders[3];

		shader_program() {
			program = 0;
			for (int i=0; i<3; i++) {
				shaders[i] = 0;
			}
		}

		~shader_program() {
			DeleteProgram();
		}

		void CreateProgram(char *vFilename, char *gFilename, char *fFilename) {
			if (vFilename == 0 && gFilename == 0 && fFilename == 0) {
				program = 0;
				return;
			}

			program = glCreateProgram();
			

			if (vFilename != 0) { // vertex shader
				char *code = FileRead(vFilename);
				
				if (code != 0) {
					
					shaders[0] = glCreateShader(GL_VERTEX_SHADER);
					
					glShaderSource(shaders[0], 1, (const char **)&code, 0);

					glCompileShader(shaders[0]);
					delete[] code;
				} else {
					MessageBox(0, "shader loading failed", "haha", 0);
					return;
				}
			}

			if (gFilename != 0) { // geometry shader
				char *code = FileRead(gFilename);
				if (code != 0) {
					shaders[1] = glCreateShader(GL_GEOMETRY_SHADER_EXT);
					glShaderSource(shaders[1], 1, (const char **)&code, 0);
					glCompileShader(shaders[1]);
					delete[] code;
				} else {
					MessageBox(0, "shader loading failed", "haha", 0);
					return;
				}
			}

			if (fFilename != 0) { // fragment shader
				char *code = FileRead(fFilename);
				if (code != 0) {
					shaders[2] = glCreateShader(GL_FRAGMENT_SHADER);
					glShaderSource(shaders[2], 1, (const char **)&code, 0);
					glCompileShader(shaders[2]);
					delete[] code;
				} else {
					MessageBox(0, "shader loading failed", "haha", 0);
					return;
				}
			}

			if (vFilename != 0)
				glAttachShader(program, shaders[0]);
			if (gFilename != 0)
				glAttachShader(program, shaders[1]);
			if (fFilename != 0)
				glAttachShader(program, shaders[2]);

			glLinkProgram(program);

			int param;
			glGetProgramiv(program, GL_LINK_STATUS, &param);
			if (param != GL_TRUE) {
				int infologLength = 0;
				int charsWritten  = 0;
				char *infoLog;

				glGetProgramiv(program, GL_INFO_LOG_LENGTH,&infologLength);

				if (infologLength > 0)
				{
					infoLog = (char *)malloc(infologLength);
					glGetProgramInfoLog(program, infologLength, &charsWritten, infoLog);
					MessageBox(0, infoLog, "Shader Loading Error", 0);
					free(infoLog);
				}
			}
		}

		void DeleteProgram() {
			if (program == 0)
				return;

			for (int i=0; i<3; i++) {
				if (shaders[i] != 0) {
					glDetachShader(program, shaders[i]);
					glDeleteShader(shaders[i]);
					shaders[i] = 0;
				}
			}
			
			glDeleteProgram(program);
			program = 0;
		}
	};

public :
	static const int NUL = 0;
	static const int BLUE = 1;

	shader_program programs[2];

	Shader () {
		programs[NUL].CreateProgram(0, 0, 0);
		programs[BLUE].CreateProgram("test.vert", 0, "test.frag");
	}

	~Shader() {
		programs[NUL].DeleteProgram();
		programs[BLUE].DeleteProgram();
	}

	void BuildAllPrograms();
	void UseProgram(int id);

private:
	void BuildBlueProgram();
	void BuildShadowProgram();

};

#endif
