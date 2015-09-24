#ifndef _VIEW_H_
#define _VIEW_H_
#include "MazeController.h"
#include <GL/glew.h>
#include <GL/gl.h>
#include <string>
#include <stack>
#include <glm/glm.hpp>

using namespace std;


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

	stack<glm::mat4> proj, modelView;

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

	void onMousePressed(const int mouseX, const int mouseY);
	void onMouseMoved(const int mouseX, const int mouseY);

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