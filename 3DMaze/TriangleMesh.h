#ifndef _TRIANGLE_MESH_H_
#define _TRIANGLE_MESH_H_

#include <glm/glm.hpp>
#include <vector>
using namespace std;
#include "Material.h"


class VertexAttribs
{
public:
	glm::vec4 position;
	glm::vec4 normal;
	glm::vec3 texcoords;
};


class TriangleMesh
{
	friend class OBJImporter;
	friend class OBJExporter;

public:
	TriangleMesh(void)
	{

	}
	~TriangleMesh(void)
	{}

	inline glm::vec4 getMinimumBounds()
	{
		return minBounds;
	}

	inline glm::vec4 getMaximumBounds()
	{
		return maxBounds;
	}

	inline vector<VertexAttribs>& getVertexData()
	{
		return vertex_data;
	}



	inline vector<unsigned int>& getTriangles()
	{
		return triangles;
	}

	inline vector<unsigned int>& getTriangleTextureIndices()
	{
		return triangle_texture_indices;
	}

	inline vector<unsigned int>& getTriangleNormalIndices()
	{
		return triangle_normal_indices;
	}

	inline void initVertexData(vector<VertexAttribs>& p)
	{
		vertex_data = p;
		computeBoundingBox();
	}


	inline void initTriangleTextureIndices(vector<unsigned int>& triangles)
	{
		this->triangle_texture_indices = triangles;
	}

	inline void initTriangleNormalIndices(vector<unsigned int>& triangles)
	{
		this->triangle_normal_indices = triangles;
	}

	inline void initTriangles(vector<unsigned int>& triangles)
	{
		this->triangles = triangles;
	}


	inline void computeBoundingBox()
	{
		unsigned int i,j;

		if (vertex_data.size()<=0)
			return;

		minBounds = maxBounds = vertex_data[0].position;
		for (i=0;i<vertex_data.size();i++)
		{
			for (j=0;j<3;j++)
			{
				if (vertex_data[i].position[j]<minBounds[j])
				{
					minBounds[j] = vertex_data[i].position[j];
				}

				if (vertex_data[i].position[j]>maxBounds[j])
				{
					maxBounds[j] = vertex_data[i].position[j];
				}
			}
		}
	}

	void computeNormals()
	{
		unsigned int i,k;

		for (i=0;i<vertex_data.size();i++)
		{
			vertex_data[i].normal = glm::vec4(0,0,0,0.0);
		}

		for (i=0;i<triangles.size();i+=3)
		{
			glm::vec4 norm;

			//compute the normal of this triangle
			int v[3];

			for (k=0;k<3;k++)
			{
				v[k] = triangles[i+k];
			}

			//the newell's method to calculate normal
			
			for (k=0;k<3;k++)
			{
				norm[0] += (vertex_data[v[k]].position[1]-vertex_data[v[(k+1)%3]].position[1])*(vertex_data[v[k]].position[2]+vertex_data[v[(k+1)%3]].position[2]);
				norm[1] += (vertex_data[v[k]].position[2]-vertex_data[v[(k+1)%3]].position[2])*(vertex_data[v[k]].position[0]+vertex_data[v[(k+1)%3]].position[0]);
				norm[2] += (vertex_data[v[k]].position[0]-vertex_data[v[(k+1)%3]].position[0])*(vertex_data[v[k]].position[1]+vertex_data[v[(k+1)%3]].position[1]);
			}

			if (glm::length(norm)>0)
				norm = glm::normalize(norm);

			for (k=0;k<3;k++)
			{
				vertex_data[v[k]].normal = vertex_data[v[k]].normal + norm;
			}
		}

		for (i=0;i<vertex_data.size();i++)
		{
			if (glm::length(vertex_data[i].normal)>0)
				vertex_data[i].normal = glm::normalize(vertex_data[i].normal);
		}
	}
	
protected:
	vector<VertexAttribs> vertex_data;
	vector<unsigned int> triangles, triangle_texture_indices,triangle_normal_indices;

	glm::vec4 minBounds,maxBounds; //bounding box


};

#endif