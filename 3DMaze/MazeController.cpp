#include "MazeController.h"

MazeController::MazeController(Maze& maze, float aspectRatio){
	modelMaze = &maze;
	this->aspectRatio = aspectRatio;
}

VertexAttribs * MazeController::getArrayStart(){
	return &vertices[0];
}

GLuint * MazeController::getIndicesStart(){
	return &indices[0];
}

int MazeController::getBufferByteCount() const{
	return sizeof(VertexAttribs) * vertices.size();
}

int MazeController::getIndicesByteCount() const{
	return sizeof(GLuint)*indices.size();
}

int MazeController::getIndicesSize() const{
	return indices.size();
}

void MazeController::onAspectRatioChanged(const float aspectRatio){
	this->aspectRatio = aspectRatio;
}

MazeController::~MazeController(){}









