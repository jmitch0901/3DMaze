#include "View3DMaze.h"
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cstdlib>
#include <fstream>

using namespace std;

View3DMaze::View3DMaze(){
	mazeController = NULL;
}

View3DMaze::~View3DMaze(){

}

void View3DMaze::resize(int w, int h){
	WINDOW_WIDTH=w;
	WINDOW_HEIGHT=h;
	aspectRatio = w/(float)h;

	if(mazeController!=NULL)
		mazeController->onAspectRatioChanged(aspectRatio);

	while(!proj.empty()){
		proj.pop();
	}

	proj.push(glm::ortho(-200.0f,200.0f,-200.0f*WINDOW_HEIGHT/WINDOW_WIDTH,200.0f*WINDOW_HEIGHT/WINDOW_WIDTH,0.1f,10000.0f));
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

void View3DMaze::onMousePressed(const int mouseX, const int mouseY){

}

void View3DMaze::onMouseMoved(const int mouseX, const int mouseY){

}

void View3DMaze::draw(){

	glUseProgram(programID);
	glBindVertexArray(vao);

	while (!modelView.empty())
        modelView.pop();

	glBindVertexArray(0);
	glUseProgram(0);
}

void View3DMaze::getOpenGLVersion(int *major,int *minor)
{
    const char *verstr = (const char *)glGetString(GL_VERSION);
    if ((verstr == NULL) || (sscanf_s(verstr,"%d.%d",major,minor)!=2))
    {
        *major = *minor = 0;
    }
}

void View3DMaze::getGLSLVersion(int *major,int *minor)
{
    int gl_major,gl_minor;

    getOpenGLVersion(&gl_major,&gl_minor);
    *major = *minor = 0;

    if (gl_major==1)
    {
        /* GL v1.x can only provide GLSL v1.00 as an extension */
        const char *extstr = (const char *)glGetString(GL_EXTENSIONS);
        if ((extstr!=NULL) && (strstr(extstr,"GL_ARB_shading_language_100")!=NULL))
        {
            *major = 1;
            *minor = 0;
        }
    }
    else if (gl_major>=2)
    {
        /* GL v2.0 and greater must parse the version string */
        const char *verstr = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
        if ((verstr==NULL) || (sscanf_s(verstr,"%d.%d",major,minor) !=2))
        {
            *major = 0;
            *minor = 0;
        }
    }
}















