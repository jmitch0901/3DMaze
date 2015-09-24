#ifndef _MY_MAZE_CONTROLLER_
#define _MY_MAZE_CONTROLLER_

#include "Maze.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>


class VertexAttribs{
	friend class MazeController;
private:
	float position[4];
	float colors[3];
};

class MazeController{

private: 
	Maze *modelMaze;

	vector<VertexAttribs> vertices;
	vector<GLuint> indices;

	float aspectRatio;

public:
	MazeController(Maze& maze,float aspectRatio);
	~MazeController();


	VertexAttribs *getArrayStart();
	GLuint* getIndicesStart();

	int getBufferByteCount() const;
	int getIndicesByteCount() const;
	int getIndicesSize() const;

	void onAspectRatioChanged(const float aspectRatio);


};
#endif