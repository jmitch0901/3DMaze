#ifndef _VIEW_H_
#define _VIEW_H_

#include <SFML/Graphics.hpp>
#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <GL/GL.h>
#include <string>
#include "MazeController.h"
using namespace std;

#include <glm/glm.hpp>


class View3DMaze{
	#define BUFFER_OFFSET(offset) ((void *)(offset))

	typedef struct {
        GLenum type; 
        string  filename; 
        GLuint  shader; 
    } ShaderInfo;

	enum Buffer_IDs {ArrayBuffer,IndexBuffer,NumBuffers};

private:

	MazeController* mazeController;

	GLuint programID;
	GLuint vao;
	GLuint vbo[NumBuffers];

	glm::mat4 proj,modelView;
	GLint projectionLocation,
		modelViewLocation, 
		vPositionLocation, 
		vColorLocation;

	int WINDOW_WIDTH, WINDOW_HEIGHT;

	float aspectRatio;

	void reload();
	

public:
	View3DMaze();
	~View3DMaze();

	void resize(int w, int h);
	void draw();

	void getOpenGLVersion(int *major, int *minor);
	void getGLSLVersion(int *major, int *minor);

	void setMazeController(MazeController *mazeController);
	

protected:
	
	GLuint linkShadersToGPU(ShaderInfo* shaders);
	void printShaderInfoToLog(GLuint shader);
};

#endif