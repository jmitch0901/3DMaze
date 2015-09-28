#ifndef _VIEW_H_
#define _VIEW_H_
#include <GL/glew.h>
#include <GL/gl.h>
#include <stack>
#include <glm/glm.hpp>
#include "Object.h"
#include "Maze.h"

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
	int WINDOW_WIDTH, WINDOW_HEIGHT;
	float aspectRatio;

	bool showWireFrame;
	int startX, startY, lastX, lastY;

	GLuint programID;

	vector<Object *> objectsList;
	stack<glm::mat4> proj, 
		modelView;

	glm::mat4 mazeTransform;

	GLint projectionLocation,
		modelViewLocation, 
		objectColorLocation;

	Maze *maze;

	vector<vector<int> > mazeIndicesWithHoles;

	void createWallsAndFindHoles(TriangleMesh &tm,float floorX, float floorY, float floorZ);
	void placeMartiniGlass(TriangleMesh &tm,float floorX, float floorY, float floorZ);
	

public:
	View3DMaze();
	~View3DMaze();

	void initialize(Maze* maze);

	void onMousePressed(const int mouseX, const int mouseY);
	void onMouseMoved(const int mouseX, const int mouseY);
	void setShowWireFrame(bool showWireFrame);

	void resize(int w, int h);
	void draw();

	void getOpenGLVersion(int *major, int *minor);
	void getGLSLVersion(int *major, int *minor);

protected:
	
	GLuint linkShadersToGPU(ShaderInfo* shaders);
	void printShaderInfoToLog(GLuint shader);
};

#endif