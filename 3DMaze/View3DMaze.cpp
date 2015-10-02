#include "View3DMaze.h"
#include <GL/glew.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include "OBJImporter.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <time.h>
/*
	Jonathan Mitchell
*/
using namespace std;

View3DMaze::View3DMaze(){
	mazeTransform = glm::mat4(1.0f);
	showWireFrame = false;
	mazeRotatedWeight = 0.0f;
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


	float x = 1.0f;
	float y = 1.0f;
	float z = 0.0f;
	float theta = 0.04f;


	//Determines which mouse direction you're dragging is dominant.
	if(abs(mouseX - lastX) > abs(mouseY - lastY)){
		x = 0.0f;
	} else {
		y = 0.0f;
	}

	if(mouseX < lastX)
		y*=-1;
	if(mouseY < lastY)
		x*=-1;

	mazeTransform = glm::rotate(glm::mat4(1.0f),theta,glm::vec3(x,y,z)) * mazeTransform;

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

	colToRowRatio = maze->getColumnCount()/(float)maze->getRowCount();

	float floorX = 150.0f*colToRowRatio;
	float floorY = 5.0f;
	float floorZ = 150.0f/colToRowRatio;

	//Sets just the floor
	o = new Object();
	OBJImporter::importFile(tm,string("models/box"),false);
	o->init(tm);
	o->setColor(0,0,0);
	o->setTransform(glm::scale(glm::mat4(1.0),glm::vec3(floorX,floorY,floorZ)));
	objectsList.push_back(o);

	createWallsAndFindHoles(tm,floorX,floorY,floorZ);
	if(mazeIndicesWithHoles.empty()){
		cout<<"There are no holes in this maze! Cannot create mesh object!"<<endl;
	} else {
		placeMartiniGlass(tm,floorX,floorY,floorZ);
	}

	glUseProgram(0);
	
}

void View3DMaze::createWallsAndFindHoles(TriangleMesh &tm,float floorX, float floorY, float floorZ){

	Object *o;

	//The stack so we can reference a previous transformed box.
	

	const int ROW_COUNT = maze->getRowCount();
	const int COLUMN_COUNT = maze->getColumnCount();

	float cellWallX = -1.0f* (floorX)/(float)COLUMN_COUNT;
	float cellWallY = (float)floorY;
	float cellWallZ = floorZ/(float)ROW_COUNT;

	float cellWallThickness = -1.0f * cellWallX/10.0f;

	//We initialize assuming that the maze is square, or there are more rows than columns. The math will work either way.
	glm::mat4 xScaleTransform 
		= glm::scale(glm::mat4(1.0f),glm::vec3(cellWallThickness,floorY,cellWallZ*colToRowRatio));

	glm::mat4 zScaleTransform 
		= glm::scale(glm::mat4(1.0f),glm::vec3(cellWallThickness,floorY,cellWallZ));

	float xTranslateFixer = colToRowRatio;
	float zTranslateFixer = 1.0f;

	//If there are more columns than rows, that we need to reverse the scaling transforms...
	if(COLUMN_COUNT>ROW_COUNT){
		xScaleTransform 
			= glm::scale(glm::mat4(1.0f),glm::vec3(cellWallThickness,floorY,cellWallZ*colToRowRatio));

		zScaleTransform 
			= glm::scale(glm::mat4(1.0f),glm::vec3(cellWallThickness,floorY,cellWallZ));

	}


	stack<glm::mat4> wallTranslateStack;

	//For the ROWS
	wallTranslateStack.push(glm::mat4(1.0f));

	//For COLUMNS
	wallTranslateStack.push(glm::translate(glm::mat4(1.0f),glm::vec3(floorX/2.0f,floorY,floorZ/2.0f)));

	for(int i = 0; i < ROW_COUNT; i++){

		for(int j = 0; j < COLUMN_COUNT; j++){

			const int CELL_CODE = maze->getCellLogicAsInteger(j,i);


			//Used later to determine where to place the martini glass
			if(CELL_CODE == 0){
				vector<int> v;
				v.push_back(j);
				v.push_back(i);
				
				mazeIndicesWithHoles.push_back(v);			
			}

			if(j==0){
				wallTranslateStack.push(wallTranslateStack.top());
			} else {
				wallTranslateStack.top() *= glm::translate(glm::mat4(1.0f),glm::vec3(cellWallX,0,0));
			}
			
			//Left Wall
			if((CELL_CODE&8)==8){
				o = new Object();
				o->init(tm);
				o->setColor(0,1,0);
				o->setTransform(glm::translate(glm::mat4(1.0f),glm::vec3(-0.5f * cellWallThickness,0,-5.0f * cellWallThickness * (1.0f/colToRowRatio))) * wallTranslateStack.top() * zScaleTransform);
				//o->setTransform(wallTranslateStack.top() * scaleTransform);
				objectsList.push_back(o);
			}

		

			//Top Wall
			if((CELL_CODE&4)==4){
				o = new Object();
				o->init(tm);
				o->setColor(0,1,0);
				//o->setTransform(glm::translate(glm::mat4(1.0f),glm::vec3(-2*cellWallY,0,-0.5f * cellWallY)) * glm::rotate(wallTranslateStack.top(),glm::radians(90.0f),glm::vec3(0.0f,1.0f,0.0f)) * scaleTransform);
				o->setTransform(glm::translate(glm::mat4(1.0f),glm::vec3(-5.0f*cellWallThickness,0,-0.5f * cellWallThickness)) * glm::rotate(wallTranslateStack.top(),glm::radians(90.0f),glm::vec3(0.0f,1.0f,0.0f)) * xScaleTransform);
				objectsList.push_back(o);
				
			}

			//Bottom Wall?
			if(i == ROW_COUNT - 1){
				//cout<<"CELL CODE: "<<CELL_CODE<<endl;
				if((CELL_CODE&1)==1){			
					o = new Object();
					o->init(tm);
					o->setColor(0,1,0);
					o->setTransform(glm::translate(glm::mat4(1.0f),glm::vec3(-5.0*cellWallThickness,0,(-cellWallZ)+(0.5f * cellWallThickness))) * glm::rotate(wallTranslateStack.top(),glm::radians(90.0f),glm::vec3(0.0f,1.0f,0.0f)) * xScaleTransform);
					objectsList.push_back(o);
				}
			}


			//Right Wall?
			if(j == COLUMN_COUNT - 1){			
				if((CELL_CODE&2)==2){
					o = new Object();
					o->init(tm);
					o->setColor(0,1,0);
					wallTranslateStack.top() *= glm::translate(glm::mat4(1.0f),glm::vec3(cellWallX,0,0));
					o->setTransform(glm::translate(glm::mat4(1.0f),glm::vec3(0.5f * cellWallThickness,0,-5.0*cellWallThickness*(1.0f/colToRowRatio))) *  wallTranslateStack.top() * zScaleTransform);
					objectsList.push_back(o);

				}
			}
		}

		wallTranslateStack.pop();
		wallTranslateStack.push(wallTranslateStack.top() * glm::translate(glm::mat4(1.0f),glm::vec3(0,0,-cellWallZ)));
	}
}

void View3DMaze::placeMartiniGlass(TriangleMesh &tm, float floorX, float floorY, float floorZ){

	
	srand ( time(NULL) ); //initialize the random seed

	const int ROW_COUNT = maze->getRowCount();
	const int COLUMN_COUNT = maze->getColumnCount();

	float cellWallX = -1.0f * (floorX)/(float)COLUMN_COUNT;
	float cellWallY = (float)floorY;
	float cellWallZ = floorZ/(float)ROW_COUNT;

	float cellWallThickness = -1.0f * cellWallX/10.0f;

	int holeIndex = std::rand() % mazeIndicesWithHoles.size();

	int columnNumber = mazeIndicesWithHoles[holeIndex][0];
	int rowNumber = mazeIndicesWithHoles[holeIndex][1];

	cout<<"Hole List SIZE: "<<mazeIndicesWithHoles.size()<<endl;
	cout<<"HOLE INDEX: "<<holeIndex<<endl;
	cout<<"(COLUMN,ROW): "<<columnNumber<<", "<<rowNumber<<endl;

	float xTranslateFixer = colToRowRatio;
	float zTranslateFixer = 1.0f;

	
	glm::mat4 glassTransform =  
		glm::translate(glm::mat4(1.0f),glm::vec3(columnNumber*cellWallX + (cellWallX * 0.5f),0,-rowNumber*cellWallZ - (0.5f*cellWallZ))) 
		* glm::translate(glm::mat4(1.0f),glm::vec3(floorX/2.0f,cellWallY*0.5f,floorZ/2.0f))
		*glm::scale(glm::mat4(1.0f),glm::vec3(cellWallZ*3,cellWallZ*3,cellWallZ*3));
		;


	Object* o = new Object();
	OBJImporter::importFile(tm,string("models/martini_glass"),false);
	o->init(tm);
	o->setColor(1,0,0);
	o->setTransform(glassTransform);
	objectsList.push_back(o);






}

void View3DMaze::draw(){

	glUseProgram(programID);
	while (!modelView.empty())
        modelView.pop();


	modelView.push(glm::mat4(1.0));
    modelView.top() *= glm::lookAt(
		glm::vec3(0,10,-50),
		glm::vec3(0,0,0),
		glm::vec3(0,1,0));

	
    glUniformMatrix4fv(projectionLocation,1,GL_FALSE,glm::value_ptr(proj.top()));

	if(showWireFrame){
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	}

	modelView.top() *= mazeTransform;

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















