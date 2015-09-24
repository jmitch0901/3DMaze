#include "View3DMaze.h"
#include <GL/glew.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include "OBJImporter.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
/*
	Jonathan Mitchell
*/
using namespace std;

View3DMaze::View3DMaze(){
	mazeTransform = glm::mat4(1.0f);

}

View3DMaze::~View3DMaze(){
	 for (int i=0;i<objectsList.size();i++)
    {
        delete objectsList[i];
    }

    objectsList.clear();
}

void View3DMaze::resize(int w, int h){
	WINDOW_WIDTH=w;
	WINDOW_HEIGHT=h;
	aspectRatio = w/(float)h;

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

void View3DMaze::initialize(Maze* maze){
	this->maze = maze;

	ShaderInfo shaders[] =
    {
        {GL_VERTEX_SHADER,"triangles.vert"},
        {GL_FRAGMENT_SHADER,"triangles.frag"},
        {GL_NONE,""}
    };

	programID = linkShadersToGPU(shaders);

	glUseProgram(programID);

	projectionLocation = glGetUniformLocation(programID,"projection");
    modelViewLocation = glGetUniformLocation(programID,"modelview");
	objectColorLocation = glGetAttribLocation(programID,"vColor");


	Object *o;
	TriangleMesh tm;

	o = new Object();
	OBJImporter::importFile(tm,string("models/box"),false);
	o->init(tm);
	o->setColor(0,0,0);
	o->setTransform(glm::translate(glm::mat4(1.0),glm::vec3(0,50.0f,0)) * glm::scale(glm::mat4(1.0),glm::vec3(150,5,150)));
	objectsList.push_back(o);
	
	glUseProgram(0);
	
}


void View3DMaze::onMousePressed(const int mouseX, const int mouseY){
	lastX = startX = mouseX;
	lastY = startY = mouseY;
}

void View3DMaze::onMouseMoved(const int mouseX, const int mouseY){

	bool isStartXBig = false;
	bool isStartYBig = false;

	int bigX = (isStartXBig = (startX>=mouseX)) ? startX : mouseX;
	int bigY = (isStartYBig = (startY>=mouseY)) ? startY : mouseY;

	int smallX = isStartXBig ? mouseX : startX;
	int smallY = isStartYBig ? mouseY : startY;

	float x = smallY/(float)bigY;
	float y = smallX/(float)bigX;
	float z = 0.0f;

	float theta = 0.04f;

	if(mouseX > lastX)
		y*=-1.0f;
	if(mouseY > lastY)
		x*=-1.0f;


	//Determines which mouse direction is dominant.
	if(abs(mouseX - startX) > abs(mouseY - startY)){
		x = 0.0f;
	} else {
		y = 0.0f;
	}

	//cout<<"start x: "<<startX<<", mouseX: "<<mouseX<<endl;

	/*float x,y;
	float z = 1.0f;
	float theta = 1.0f;

	x = mouseX / startX;
	y = mouseY / startY;*/

	//Check to see whether or not you are dragging the mouse back

	mazeTransform = glm::rotate(
		mazeTransform,
		theta,
		glm::vec3(x,y,z)
		);

	lastX = mouseX;
	lastY = mouseY;

}

void View3DMaze::draw(){

	glUseProgram(programID);
	while (!modelView.empty())
        modelView.pop();

	glEnable(GL_LINE_SMOOTH);// or GL_POLYGON_SMOOTH 
	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); ;
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST); //GL_FASTEST,GL_DONT_CARE 

	modelView.push(glm::mat4(1.0));
    modelView.top() *= glm::lookAt(
		glm::vec3(0,10,50),
		glm::vec3(0,0,0),
		glm::vec3(0,1,0));

	modelView.top() *= mazeTransform;
    glUniformMatrix4fv(projectionLocation,1,GL_FALSE,glm::value_ptr(proj.top()));

	//glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	for(int i = 0 ; i < objectsList.size(); i++){
		modelView.top() *= objectsList[i]->getTransform() * glm::translate(glm::mat4(1.0f),glm::vec3(0,-10.0f,0));
        glm::vec4 color = objectsList[i]->getColor();

		//The total transformation is whatever was passed to it, with its own transformation
        glUniformMatrix4fv(modelViewLocation,1,GL_FALSE,glm::value_ptr(modelView.top()));
        //set the color for all vertices to be drawn for this object
        glVertexAttrib3fv(objectColorLocation,glm::value_ptr(color));

		objectsList[i]->draw();
	}
	glFinish();
    modelView.pop();


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















