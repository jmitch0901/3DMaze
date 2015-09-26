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
	showWireFrame = false;

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

	proj.push(glm::ortho(-200.0f,200.0f,-200.0f*WINDOW_HEIGHT/WINDOW_WIDTH,200.0f*WINDOW_HEIGHT/WINDOW_WIDTH,-1000.0f,1000.0f));
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

	if(mouseX < lastX)
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

void View3DMaze::setShowWireFrame(bool showWireFrame){
	this->showWireFrame = showWireFrame;
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

	int floorX = 200;
	int floorY = 5;
	int floorZ = 200;

	//Sets just the floor
	o = new Object();
	OBJImporter::importFile(tm,string("models/box"),false);
	o->init(tm);
	o->setColor(0,0,0);
	o->setTransform(glm::scale(glm::mat4(1.0),glm::vec3(floorX,floorY,floorZ)));
	objectsList.push_back(o);


	createWalls(tm,floorX,floorY,floorZ);
	
	glUseProgram(0);
	
}

void View3DMaze::createWalls(TriangleMesh &tm,int floorX, int floorY, int floorZ){

	Object *o;
	//TriangleMesh tm;


	//The stack so we can reference a previous transformed box.
	

	const int ROW_COUNT = maze->getRowCount();
	const int COLUMN_COUNT = maze->getColumnCount();

	float cellWallX = -1.0f * (floorX)/(float)COLUMN_COUNT;
	float cellWallY = (float)floorY;
	float cellWallZ = floorZ/(float)ROW_COUNT;

	const glm::mat4 scaleTransform 
		= glm::scale(glm::mat4(1.0f),glm::vec3(floorY,floorY,cellWallZ));



	stack<glm::mat4> wallTranslateStack;

	//For the ROWS
	wallTranslateStack.push(glm::mat4(1.0f));

	//For COLUMNS
	wallTranslateStack.push(glm::translate(glm::mat4(1.0f),glm::vec3(floorX/2.0f,floorY,floorZ/2.0f)));

	for(int i = 0; i < ROW_COUNT; i++){


		//wallTranslateStack.push(wallTranslateStack.top() * glm::translate(glm::mat4(1.0f),glm::vec3(cellWallX,0,0)));


		for(int j = 0; j < COLUMN_COUNT; j++){

			const int CELL_CODE = maze->getCellLogicAsInteger(j,i);

			if(j==0){
				wallTranslateStack.push(wallTranslateStack.top());
			} else {

				wallTranslateStack.top() *= glm::translate(glm::mat4(1.0f),glm::vec3(cellWallX,0,0));
			}
			
			//Left Wall
			if((CELL_CODE&8)==8){
				o = new Object();
				o->init(tm);
				o->setColor(0,0,1);
				o->setTransform(glm::translate(glm::mat4(1.0f),glm::vec3(-0.5f * cellWallY,0,-2*cellWallY)) * wallTranslateStack.top() * scaleTransform);
				objectsList.push_back(o);
			}

		

			//Top Wall
			if((CELL_CODE&4)==4){
				o = new Object();
				o->init(tm);
				o->setColor(0,1,1);
				o->setTransform(glm::translate(glm::mat4(1.0f),glm::vec3(-2*cellWallY,0,-0.5f * cellWallY)) * glm::rotate(wallTranslateStack.top(),glm::radians(90.0f),glm::vec3(0.0f,1.0f,0.0f)) * scaleTransform);
				objectsList.push_back(o);
				
			}

			if(j == COLUMN_COUNT - 1){
				//Right Wall?
				if((CELL_CODE&2)==2){
					o = new Object();
				o->init(tm);
				o->setColor(0,1,0);
				wallTranslateStack.top() *= glm::translate(glm::mat4(1.0f),glm::vec3(cellWallX,0,0));
				//o->setTransform(glm::translate(glm::mat4(1.0f),glm::vec3() * wallTranslateStack.top() * ))
				o->setTransform(glm::translate(glm::mat4(1.0f),glm::vec3(0.5f * cellWallY,0,-2*cellWallY)) *  wallTranslateStack.top() * scaleTransform);
				//o->setTransform(glm::translate(glm::mat4(1.0f),glm::vec3(-2*cellWallY,0,-0.5f * cellWallY)) * glm::rotate(wallTranslateStack.top(),glm::radians(90.0f),glm::vec3(0.0f,1.0f,0.0f)) * scaleTransform);
				objectsList.push_back(o);

				}



				//Bottom Wall?


			}

			

		}

		wallTranslateStack.pop();
		wallTranslateStack.push(wallTranslateStack.top() * glm::translate(glm::mat4(1.0f),glm::vec3(0,0,-cellWallZ)));


	}


	/*
		If I want to hardcode walls in, follow code below!
	
	//Left Wall */
	/*o = new Object();
	OBJImporter::importFile(tm,string("models/box"),false);
	o->init(tm);
	o->setColor(0,0,1);
	o->setTransform(glm::translate(glm::mat4(1.0f),glm::vec3(floorX/2.0f,floorY,0)) * glm::scale(glm::mat4(1.0f),glm::vec3(floorY,floorY,floorX)));//glm::translate
	objectsList.push_back(o);


	//Right Wall
	o = new Object();
	OBJImporter::importFile(tm,string("models/box"),false);
	o->init(tm);
	o->setColor(0,0,1);
	o->setTransform(glm::translate(glm::mat4(1.0f),glm::vec3(-floorX/2.0f,floorY,0)) * glm::scale(glm::mat4(1.0f),glm::vec3(floorY,floorY,floorX)));//glm::translate
	objectsList.push_back(o);*/
	
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
		glm::vec3(0,10,-50),
		glm::vec3(0,0,0),
		glm::vec3(0,1,0));

	modelView.top() *= mazeTransform;
    glUniformMatrix4fv(projectionLocation,1,GL_FALSE,glm::value_ptr(proj.top()));

	if(showWireFrame){
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	}

	for(int i = 0 ; i < objectsList.size(); i++){
		glm::mat4 transform = objectsList[i]->getTransform();
        glm::vec4 color = objectsList[i]->getColor();

		//The total transformation is whatever was passed to it, with its own transformation
		glUniformMatrix4fv(modelViewLocation,1,GL_FALSE,glm::value_ptr(modelView.top() * transform));
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















