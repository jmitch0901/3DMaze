#ifndef _OBJEXPORTER_H_
#define _OBJEXPORTER_H_

#include "TriangleMesh.h"
#include <string>
#include <fstream>
using namespace std;

class OBJExporter
{
public:
	static bool exportFile(TriangleMesh& mesh,string& filename)
	{
		ofstream of((filename + string(".obj")).c_str());

		if (of.is_open())
		{
			int i,j;

			for (i=0;i<mesh.vertex_data.size();i++)
			{
				of << "v " << mesh.vertex_data[i].position[0] << " " << mesh.vertex_data[i].position[1] << " " << mesh.vertex_data[i].position[2] <<  endl;
			}
			
			for (i=0;i<mesh.vertex_data.size();i++)
			{
				of << "vn " << mesh.vertex_data[i].normal[0] << " " << mesh.vertex_data[i].normal[1] << " " << mesh.vertex_data[i].normal[2] << endl;
			}
			
			for (i=0;i<mesh.vertex_data.size();i++)
			{
				of << "vt " << mesh.vertex_data[i].texcoords[0] << " " << mesh.vertex_data[i].texcoords[1] << " " << mesh.vertex_data[i].texcoords[2] << endl;
			}
		

			//triangles
			if ((mesh.triangle_texture_indices.size()>0) || (mesh.triangle_normal_indices.size()>0))
			{
				for (i=0;i<mesh.triangles.size();i+=3)
				{
					of << "f " ;
					for (j=0;j<3;j++)
					{
						of << mesh.triangles[i+j]+1;
					}

					if (mesh.triangle_texture_indices.size()>0)
					{
						for (j=0;j<3;j++)
						{
							of << "/" << mesh.triangle_texture_indices[i+j]+1;
						}
					}
					else
					{
						of << "/";
					}

					if (mesh.triangle_normal_indices.size()>0)
					{
						for (j=0;j<3;j++)
						{
							of << "/" << mesh.triangle_normal_indices[i+j]+1;
						}
					}
					of << endl;
				}				
			}
			else
			{
				for (i=0;i<mesh.triangles.size();i+=3)
				{
					of << "f " ;
					for (j=0;j<3;j++)
					{
						of << mesh.triangles[i+j]+1 << " ";
					}
					of << endl;
				}				
			}
		
			of.close();
		}
		else
		{
			return false;
		}
		return true;
	}
};

#endif