#ifndef OBJECT_H_
#define OBJECT_H_


/*
 *This class is the base class for all the 3D meshes that we will draw
 *
 *Individual objects will actually create the vertex attributes and set up the VBOs and VAO.
 *
 *This class just provides them with a VAO object, and provides the drawing code for all objects
 */

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GL/gl.h>
#include <vector>
#include <string>
using namespace std;

#include "Material.h"
#include "TriangleMesh.h"

#define BUFFER_OFFSET(offset) ((void *)(offset))

class Object
{
protected:
	/*
		*the different kinds of vertex buffer objects (VBOs). We need two: one for vertex data and one for
		*the triangle indices
		*/
	enum Buffer_IDs {VertexBuffer,IndexBuffer,NumBuffers};
	/*
		*As corresponding to the VertexAttribs structure above and the shader input variables, we have
		*three attributes per vertex. The values of the attributes must match the layout specified in the
		*shader.
		*/
	enum Attrib_IDs {vPosition=0,vNormal=1,vTexCoord=2};
protected:
	//the VAO corresponding to this object. Consider this as a proxy for rendering this object.
	GLuint VAO;
	//the VBO IDS required for this object
	GLuint buffers[NumBuffers];
	TriangleMesh mesh;

	Material material;
	//the transformation associated with this object
	glm::mat4 transform;

	//a unique "name" for this object
	string name;


public:
	Object(string name="")
	{
		//set the name
		setName(name);			
	}

	void init(TriangleMesh& mesh)
	{
		//create the VAO ID for this object
		glGenVertexArrays(1,&VAO);
		//bind the VAO
		glBindVertexArray(VAO);
		//create as many VBO IDs as you need, in this case 2
		glGenBuffers(NumBuffers,buffers);
			
		//default material
		material.setAmbient(1.0f,0.6f,0.6f);
		material.setDiffuse(1.0f,0.6f,0.6f);
		material.setSpecular(0.2f,0.1f,0.1f);
		material.setShininess(1);

		this->mesh = mesh;



		this->mesh.computeBoundingBox();


		/*
			*Bind the VAO as the current VAO, so that all subsequent commands affect it
			*/
		glBindVertexArray(VAO);

		/*
			*Allocate the VBO for vertex data and send it to the GPU
			*/
		glBindBuffer(GL_ARRAY_BUFFER,buffers[VertexBuffer]);
		glBufferData(GL_ARRAY_BUFFER,sizeof(VertexAttribs)*this->mesh.getVertexData().size(),&this->mesh.getVertexData()[0],GL_STATIC_DRAW);

		/*
			*Allocate the VBO for triangle indices and send it to GPU
			*/
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[IndexBuffer]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*this->mesh.getTriangles().size(), &this->mesh.getTriangles()[0], GL_STATIC_DRAW);


		/*
			*Specify all the vertex attribute pointers, i.e. tell OpenGL how to organize data according to attributes rather than vertices
			*/

		//first enable the correct VBO, since the "current" is the VBO for triangle indices
		glBindBuffer(GL_ARRAY_BUFFER,buffers[VertexBuffer]);

		//VertexData starts with position, so starting byte is 0
		glVertexAttribPointer(vPosition,4,GL_FLOAT,GL_FALSE,sizeof(VertexAttribs),BUFFER_OFFSET(0));
		//In VertexData, normal follows the position (4 floats), so start reading normals from 4*sizeof(float)
		glVertexAttribPointer(vNormal,4,GL_FLOAT,GL_FALSE,sizeof(VertexAttribs),BUFFER_OFFSET(sizeof(glm::vec4)));
		//In VertexData, texture coordinates follow the position and normal (8 floats), so start reading texture coordinates from 8*sizeof(float)
	//    glVertexAttribPointer(vTexCoord,2,GL_FLOAT,GL_FALSE,sizeof(VertexAttribs),BUFFER_OFFSET(8*sizeof(float)));

		//enable the vertex attribute arrays
		glEnableVertexAttribArray(vPosition);
		glEnableVertexAttribArray(vNormal);
	//    glEnableVertexAttribArray(vTexCoord);

		/*
			*Unbind the VAO to prevent accidental change to all the settings
			*so at this point, this VAO has two VBOs and two enabled VertexAttribPointers.
			* It is going to remember all of that!
			*/
		glBindBuffer(GL_ARRAY_BUFFER,0);
		glBindVertexArray(0);
	}

	/*
		*The destructor is virtual so that any extra memory allocated by subclasses can be freed
		*in their respective destructors
		*/
	virtual ~Object(void)
	{
		if (VAO!=0)
		{
			//give back the VBO IDs to OpenGL, so that they can be reused
			glDeleteBuffers(NumBuffers,buffers);
			//give back the VAO ID to OpenGL, so that it can be reused
			glDeleteVertexArrays(1,&VAO);
		}
	}

	virtual void draw()
	{

		//draw the object

		//1. bind its VAO
		glBindVertexArray(VAO);

		//2. execute the "superpower" command
		glDrawElements(GL_TRIANGLES,mesh.getTriangles().size(),GL_UNSIGNED_INT,BUFFER_OFFSET(0));

		glBindVertexArray(0);
	}

	/*
		*Set the transformation of this object externally
		*/
	void setTransform(glm::mat4& obj)
	{
		transform = obj;
	}

	/*
		* Get the transformation of this object
		*/
	glm::mat4 getTransform()
	{
		return transform;
	}

	/*
		*Set the name of this object
		*/
	void setName(const string& name)
	{
		this->name = name;
	}

	void setColor(float r,float g,float b)
	{
		Material m;

		m.setAmbient(r,g,b);
		m.setDiffuse(r,g,b);
		m.setSpecular(r,g,b);
		material = m;
	}

	glm::vec4 getColor()
	{
		return material.getAmbient();
	}

	/*
		*Set the material of each vertex in this object
		*/
	virtual void setMaterial(Material& mat)
	{
		material = mat;
	}

	/*
		* gets the material
		*/
	Material getMaterial()
	{
		return material;
	}

	glm::vec3 getWorldCenter()
	{
		glm::vec4 p(0.0f,0.0f,0.0f,1.0f);

		p = transform * p;

		return glm::vec3(p[0]/p[3],p[1]/p[3],p[2]/p[3]);
	}

	glm::vec4 getMinimumWorldBounds()
	{
		glm::vec4 p = mesh.getMinimumBounds();
		p = transform * p;
		return p;
	}

	glm::vec4 getMaximumWorldBounds()
	{
		glm::vec4 p = mesh.getMaximumBounds();
		p = transform * p;
		return p;
	}

};
#endif //OBJECT_H_
