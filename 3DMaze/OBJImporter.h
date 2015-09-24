#ifndef _OBJIMPORTER_H_
#define _OBJIMPORTER_H_

#include "TriangleMesh.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
using namespace std;
#include <glm/glm.hpp>

class OBJImporter
{
public:
	static bool importFile(TriangleMesh& mesh,string& filename,bool scaleAndCenter=false)
	{
		vector<glm::vec3> vertices;
		vector<glm::vec3> normals;
		vector<glm::vec3> texcoords;
		vector<unsigned int> triangles, triangle_texture_indices, triangle_normal_indices;
		unsigned int i,j;
		
		ifstream file(std::string(filename)+".obj");

		if (!file.is_open())
			return false;

		string line;

		getline(file,line);

		while (!file.eof())
		{
			if ((line.length()<=0) || (line[0] == '#'))
			{
				//line is a comment, ignore
				getline(file,line);
				continue;
			}

			stringstream str;

			str << line;

			string token;
			str >> token;

			if (token =="v")
			{
				float num;
				glm::vec3 v;

				str >> v[0];
				str >> v[1];
				str >> v[2];

				if (str.rdbuf()->in_avail()>0)
				{
					str >> num;
					if (num!=0)
					{
						v[0]/=num;
						v[1]/=num;
						v[2]/=num;
					}
				}

				vertices.push_back(v);
			}
			else if (token == "vt")
			{
				glm::vec3 v;

				str >> v[0];
				str >> v[1];
					
				if (str.str().length()>0)
				{
					str >> v[2];						
				}
				else
				{
					v[2] = 0;
				}

				texcoords.push_back(v);
			}
			else if (token == "n")
			{
				glm::vec3 v;

				str >> v[0];
				str >> v[1];
				str >> v[2];

				v = glm::normalize(v);
				normals.push_back(v);
			}
			else if (token == "f")
			{
				vector<unsigned int> t_triangles,t_tex,t_normal;
				string data;
				
				while (str >> data)
				{
					unsigned int num1,num2,num3;
					int pos1,pos2; //positions of first and second /

					pos1 = data.find('/');
					pos2 = data.find('/',pos1+1);

					stringstream tempstr;

					if (pos1==string::npos)
					{
						//no '/'. 
						//read entire data as one number and store thrice
						tempstr << data;

						tempstr >> num1;

						num3 = num2 = num1;
					}
					else if (pos2==string::npos)
					{
						//only one '/'
						//repeat 1st number into the 3rd

						tempstr << data.substr(0,pos1);
						tempstr >> num1;

						tempstr.str(data.substr(pos1+1));
						tempstr >> num2;

						num3 = num1;
					}
					else
					{
						//two '/'
						//repeat 1st number into the 3rd

						tempstr.str(data.substr(0,pos1));
						tempstr >> num1;

						tempstr.str(data.substr(pos1+1,pos2-pos1));
						tempstr >> num2;

						tempstr.str(data.substr(pos2+1));
						tempstr >> num3;
					}
					t_triangles.push_back(num1-1);
					t_tex.push_back(num2-1);
					t_normal.push_back(num3-1);
				}
				//if face has more than 3 vertices, break down into a triangle fan
				for (i=2;i<t_triangles.size();i++)
				{
					triangles.push_back(t_triangles[0]);
					triangles.push_back(t_triangles[i-1]);
					triangles.push_back(t_triangles[i]);

					triangle_texture_indices.push_back(t_tex[0]);
					triangle_texture_indices.push_back(t_tex[i-1]);
					triangle_texture_indices.push_back(t_tex[i]);

					triangle_normal_indices.push_back(t_normal[0]);
					triangle_normal_indices.push_back(t_normal[i-1]);
					triangle_normal_indices.push_back(t_normal[i]);
				}


			}
			getline(file,line);
			
		}

		if (scaleAndCenter)
		{
			//center about the origin and within a cube of side 1 centered at the origin
			//find the centroid
			glm::vec3 center;

			glm::vec3 minimum,maximum;

			minimum = maximum = center;
			for (i=0;i<vertices.size();i++)
			{
				center = center + vertices[i];
				for (j=0;j<3;j++)
				{
					if (vertices[i][j]<minimum[j])
						minimum[j] = vertices[i][j];

					if (vertices[i][j]>maximum[j])
						maximum[j] = vertices[i][j];
				}
			}

			center = center* (1.0f/vertices.size());

			for (i=0;i<vertices.size();i++)
			{
				vertices[i] = vertices[i] - center;
			}

			minimum = minimum - center;
			maximum = maximum - center;

			int longest;

			glm::vec3 newbounds;

			//the longest side of the bounding box will map to 1
			if ((maximum[0]-minimum[0])>(maximum[1]-minimum[1]))
			{
				if ((maximum[0]-minimum[0])>(maximum[2]-minimum[2]))
				{
					longest = 0;
				}
				else
				{
					longest = 2;
				}
			}
			else
			{
				if ((maximum[1]-minimum[1])>(maximum[2]-minimum[2]))
				{
					longest = 1;
				}
				else
				{
					longest = 2;
				}
			}

			for (i=0;i<3;i++)
			{
				if (fabs(maximum[longest]-minimum[longest])>0.1f)
					newbounds[i] = (maximum[i]-minimum[i])/(maximum[longest]-minimum[longest]);
				else
					newbounds[i] = 0;
			}

			//scale down each other
			for (i=0;i<vertices.size();i++)
			{
				for (j=0;j<3;j++)
				{
					if (fabs(maximum[j]-minimum[j])>0.1f)
						vertices[i][j] = (vertices[i][j]-minimum[j])*newbounds[j]/(maximum[j]-minimum[j]) - 0.5*newbounds[j];
				}
			}
		}



		//convert into VertexAttribs
		vector<VertexAttribs> vertex_data;

		for (unsigned int i=0;i<vertices.size();i++)
		{
			VertexAttribs vat;
			vat.position = glm::vec4(vertices[i],1.0f);
			if (normals.size()>0)
				vat.normal = glm::vec4(normals[i],0.0f);
			else
				vat.normal = glm::vec4(0.0f,0.0f,0.0f,0.0f);

			if (texcoords.size()>0)
				vat.texcoords = texcoords[i];
			else
				vat.texcoords = glm::vec3(0.0f,0.0f,0.0f);

			vertex_data.push_back(vat);
		}
		mesh.initVertexData(vertex_data);
		mesh.initTriangles(triangles);
		mesh.initTriangleTextureIndices(triangle_texture_indices);
		mesh.initTriangleNormalIndices(triangle_normal_indices);

		if (normals.size()<=0)
		{
			mesh.computeNormals();
		}
		return true;
	}
};

#endif