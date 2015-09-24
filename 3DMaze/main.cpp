/*
Assignment 2 - 3D Maze
Jonathan Mitchell IT 356
*/
#include <SFML/Graphics.hpp>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include "View3DMaze.h"
#include <iostream>
#include <string>
#include <sstream>
#include "Maze.h"
using namespace std;


sf::Font font;
sf::Clock sfclock;

int frames;
double frame_rate;
int mouseX, mouseY;
bool mouseIsPressed = false;

View3DMaze v;
sf::RenderWindow* renderWindow;
Maze* pMaze;

void resize(int w,int h);
void display(sf::RenderWindow *window);
void processEvent(sf::Event event,sf::RenderWindow& window);
void drawText(sf::RenderWindow& window,string text,int x,int y);
void initialize();

//Start Here
int main(int argc, char *argv[]){
	
	frames = 0;
	frame_rate = 0;
    // Request a 32-bits depth buffer when creating the window
    sf::ContextSettings contextSettings;
    contextSettings.depthBits = 32;
	contextSettings.majorVersion = 4;
	contextSettings.minorVersion = 0;
	
    // Create the main window
    sf::RenderWindow window(sf::VideoMode(800, 600), "Jon Mitchell A2 - 3D Maze", sf::Style::Default, contextSettings);
	renderWindow = &window;
	resize(800,600);
	

 	window.setActive();   

    glewExperimental = GL_TRUE;

    //initialize glew which initializes all the OpenGL extensions.
    if (glewInit()!=GLEW_OK)
    {
        cerr << "Unable to initialize GLEW...exiting" << endl;
        return EXIT_FAILURE;
    }


	Maze maze("maze-10x10.txt");
	pMaze=&maze;
	initialize();



	//Game Loop
	 while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
			processEvent(event,window);

        }

		if (window.isOpen())
			display(&window);
    }

    return EXIT_SUCCESS;
}

/*
 * This will be the function that processes any events
*/

void processEvent(sf::Event event,sf::RenderWindow& window)
{
	// Close window : exit
	if (event.type == sf::Event::Closed)
		window.close();

	// Escape key : exit
	if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape))
		window.close();

	// Adjust the viewport when the window is resized
	if (event.type == sf::Event::Resized)
		resize(event.size.width, event.size.height);


	if(event.mouseButton.button == sf::Mouse::Left && event.type == sf::Event::MouseButtonPressed){
		cout<<"Mouse Pressed "<<endl;
		mouseIsPressed=true;
		mouseX = sf::Mouse::getPosition(window).x;
		mouseY = window.getSize().y -  sf::Mouse::getPosition(window).y;
		v.onMousePressed(mouseX, mouseY);
	}

	if(event.mouseButton.button == sf::Mouse::Left && event.type == sf::Event::MouseButtonReleased){
		cout<<"Mouse Released"<<endl;
		mouseIsPressed=false;

	}

	

	if(mouseIsPressed && event.type == sf::Event::MouseMoved){
		mouseX = sf::Mouse::getPosition(window).x;
		mouseY = window.getSize().y - sf::Mouse::getPosition(window).y;

		//v.onMouseMoved(mouseX,sf::Mouse::getPosition(window).y);
		v.onMouseMoved(mouseX,mouseY);
	}
}

void drawText(sf::RenderWindow *window,string text,int x,int y)
{
	// Create some text to draw on top of our OpenGL object
  
    sf::Text textobj(text, font);

	textobj.setCharacterSize(18);
    textobj.setColor(sf::Color(255, 0, 0, 255));
    textobj.setPosition((float)x,(float)y);
	

	window->pushGLStates();
	window->resetGLStates();
	window->draw(textobj);
    window->popGLStates();
}

void display(sf::RenderWindow *window)
{
	if (frames==0)
		sfclock.restart();

	window->pushGLStates();
	window->resetGLStates();
	
	window->popGLStates();

	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); 
	glEnable(GL_DEPTH_TEST);
	v.draw(); 

	if (frames>500)
	{
		sf::Time t = sfclock.getElapsedTime();
		frame_rate = frames/t.asSeconds();
		frames = 0;
	}
	else
	{
		frames++;
	}
	stringstream str;

	str << "Frame rate " << frame_rate;

	//drawText(window,str.str(),window->getSize().x-200,50);

	window->display();

}


void resize(int w,int h)
{
    v.resize(w,h);
    glViewport(0,0,w,h);
}

void initialize()
{
    int major,minor;
    v.getOpenGLVersion(&major,&minor);

    cout <<"Opengl version supported : "<<major<<"."<<minor<<endl;
    v.getGLSLVersion(&major,&minor);
    cout << "GLSL version supported : "<<major<<"."<<minor << endl;

	v.initialize(pMaze);

	//if (!font.loadFromFile("resources/sansation.ttf"))
		//return;

	
}