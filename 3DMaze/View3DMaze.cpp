#include "View3DMaze.h"
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cstdlib>
#include <fstream>
using namespace std;

View3DMaze::View3DMaze(){

}

View3DMaze::~View3DMaze(){}

void View3DMaze::resize(int w, int h){
	WINDOW_WIDTH=w;
	WINDOW_HEIGHT=h;
	aspectRatio = w/(float)h;
	mazeController->onAspectRatioChanged(aspectRatio);
}

GLuint View3DMaze::linkShadersToGPU(ShaderInfo* shaders){
	ifstream file;
	GLuint shaderProgram;
    GLint linked;

	shaderProgram=glCreateProgram();

	while(shaders->type != GL_NONE){

		file.open(shaders->filename.c_str());
		GLint compiled;

		if(!file.is_open())
			return false;

		string source,line;

		getline(file,line);
		while(!file.eof()){
			source = source+"\n"+line;
			getline(file,line);
		}
		file.close();

		const char *codev = source.c_str();

		shaders->shader = glCreateShader(shaders->type);
		glShaderSource(shaders->shader,1,&codev,NULL);
		glCompileShader(shaders->shader);
		glGetShaderiv(shaders->shader,GL_COMPILE_STATUS,&compiled);

		if(!compiled){
			printShaderInfoToLog(shaders->shader);
            for (ShaderInfo *processed = shaders;processed->type!=GL_NONE;processed++)
            {
                glDeleteShader(processed->shader);
                processed->shader = 0;
            }
            return 0;

		}

		glAttachShader( shaderProgram, shaders->shader );
        shaders++;
	}

	glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram,GL_LINK_STATUS,&linked);

    if (!linked)
    {
        printShaderInfoToLog(shaders->shader);
        for (ShaderInfo *processed = shaders;processed->type!=GL_NONE;processed++)
        {
            glDeleteShader(processed->shader);
            processed->shader = 0;
        }
        return 0;
    }

	cout<<"GL ERROR FOR SHADERS: "<<glGetError()<<endl;

	return shaderProgram;

}

void View3DMaze::printShaderInfoToLog(GLuint shader){
	int infologLen = 0;
    int charsWritten = 0;
    GLubyte *infoLog;

    glGetShaderiv(shader,GL_INFO_LOG_LENGTH,&infologLen);
    //	printOpenGLError();
    if (infologLen>0)
    {
        infoLog = (GLubyte *)malloc(infologLen);
        if (infoLog != NULL)
        {
            glGetShaderInfoLog(shader,infologLen,&charsWritten,(char *)infoLog);
            printf("InfoLog: %s\n\n",infoLog);
            free(infoLog);
        }

    }
}

void View3DMaze::setMazeController(MazeController *mazeController){
	this->mazeController = mazeController;

	ShaderInfo shaders[] =
    {
        {GL_VERTEX_SHADER,"default.vert"},
        {GL_FRAGMENT_SHADER,"default.frag"},
        {GL_NONE,""}
    };

	programID = linkShadersToGPU(shaders);

	glUseProgram(programID);

	projectionLocation = glGetUniformLocation(programID,"projection");
    modelViewLocation = glGetUniformLocation(programID,"modelview");

    vPositionLocation = glGetAttribLocation(programID,"vPosition");
    vColorLocation = glGetAttribLocation(programID,"vColor");

	glGenVertexArrays(1,&vao);
	glBindVertexArray(vao);
	glGenBuffers(NumBuffers,&vbo[0]);

	glBindBuffer(GL_ARRAY_BUFFER,vbo[ArrayBuffer]);
	glBufferData(GL_ARRAY_BUFFER,mazeController->getBufferByteCount(),mazeController->getArrayStart(),GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbo[IndexBuffer]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,mazeController->getIndicesByteCount(),mazeController->getIndicesStart(),GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER,vbo[ArrayBuffer]);
	glVertexAttribPointer(vPositionLocation,4,GL_FLOAT,GL_FALSE,sizeof(VertexAttribs),BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vPositionLocation);
	glVertexAttribPointer(vColorLocation,3,GL_FLOAT,GL_FALSE,sizeof(VertexAttribs),BUFFER_OFFSET(4*sizeof(GLfloat)));
    glEnableVertexAttribArray(vColorLocation);

	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void View3DMaze::reload(){

}

void View3DMaze::draw(){

}














