#ifndef RESOURCE_UTILS_H
#define RESOURCE_UTILS_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include "opengl_utils.h"

#include <cstdio>
#include <cstdlib>

using namespace std;
VAO* loadSplineControlPoints(string path)
{
	//TODO: load spline control point data and return VAO
	//You can make use of getVAOFromAttribData in opengl_utils.h
	char buffer1[1024] = {0}, buffer2[1024] = {0};
	FILE *fptr = fopen(path.c_str(), "r");
	fgets(buffer1, 1024, fptr);
	size_t num_of_vertex = atoi(buffer1);
	size_t num_of_attributes = 1; // Position vector

	float* attrib_data = (float*)calloc (num_of_vertex*3, sizeof(float));
	size_t* num_of_floats_per_attribute = (size_t*)calloc (num_of_attributes, sizeof(size_t));
	*(num_of_floats_per_attribute) = 3;
	int data_idx = 0;
	for (int i = 0; i < num_of_vertex; i++)
	{
		bzero (buffer1, 1024*sizeof(char));
		fgets (buffer1, 1024, fptr);
		char *cptr = buffer1, *rptr = buffer2;
		// printf("Vertex %d: ", i);
		for (; *cptr != '\0'; cptr++)
		{
			// printf("%d\n", *cptr);
			if (('0' <= *cptr && *cptr <= '9') || *cptr == '.' || *cptr == '-' || *cptr == 'e')
			{
				*rptr = *cptr;
				rptr++;
			}
			else
			{
				if (rptr != buffer2)
				{
					*(attrib_data + data_idx) = atof(buffer2);
					// printf ("Idx %d - %3.3f, ", data_idx, *(attrib_data + data_idx));
					bzero (buffer2, 1024*sizeof(char));
					rptr = buffer2;
					data_idx++;
				}
				if (*cptr == '\0')
				{
					break;
				}
			}
		}
		// printf ("\n");
	}
	return getVAOFromAttribData (attrib_data, num_of_vertex, num_of_floats_per_attribute, num_of_attributes);
}

VAO* loadBezierSurfaceControlPoints(string path)
{
	//(Optional)TODO: load surface control point data and return VAO.
	//You can make use of getVAOFromAttribData in opengl_utils.h
	//TODO: load spline control point data and return VAO
	//You can make use of getVAOFromAttribData in opengl_utils.h
	char buffer1[1024] = {0}, buffer2[1024] = {0};
	FILE *fptr = fopen(path.c_str(), "r");
	fgets(buffer1, 1024, fptr);
	size_t num_of_vertex = atoi(buffer1)*16;
	size_t num_of_attributes = 1; // Position vector

	float* attrib_data = (float*)calloc (num_of_vertex*3, sizeof(float));
	size_t* num_of_floats_per_attribute = (size_t*)calloc (num_of_attributes, sizeof(size_t));
	*(num_of_floats_per_attribute) = 3;
	int data_idx = 0;
	for (int i = 0; i < num_of_vertex/16; i++)
	{
		fgets (buffer1, 1024, fptr);
		for (int j = 0; j < 16; j++)
		{
			bzero (buffer1, 1024*sizeof(char));
			fgets (buffer1, 1024, fptr);
			char *cptr = buffer1, *rptr = buffer2;
			for (; *cptr != '\0'; cptr++)
			{
				if (('0' <= *cptr && *cptr <= '9') || *cptr == '.' || *cptr == '-' || *cptr == 'e')
				{
					*rptr = *cptr;
					rptr++;
				}
				else
				{
					if (rptr != buffer2)
					{
						*(attrib_data + data_idx) = atof(buffer2);
						bzero (buffer2, 1024*sizeof(char));
						rptr = buffer2;
						data_idx++;
					}
					if (*cptr == '\0')
					{
						break;
					}
				}
			}
		}
	}
	return getVAOFromAttribData (attrib_data, num_of_vertex, num_of_floats_per_attribute, num_of_attributes);
	
}
#endif