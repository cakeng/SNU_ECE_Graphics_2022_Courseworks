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
	char buffer[1024] = {0};
	FILE *fptr = fopen(path.c_str(), "r");
	fgets(buffer, 1024, fptr);
	size_t num_of_vertex = atoi(buffer);
	size_t num_of_attributes = 1; // Position vector

	float* attrib_data = (float*)calloc (num_of_vertex*3, sizeof(float));
	size_t* num_of_floats_per_attribute = (size_t*)calloc  (num_of_attributes, sizeof(size_t));
	*(num_of_floats_per_attribute) = 3;
	int data_idx = 0;
	for (int i = 0; i < num_of_vertex; i++)
	{
		bzero (buffer, 1024*sizeof(char));
		fgets(buffer, 1024, fptr);
		char* rptr = buffer;
		for (char* cptr = buffer; 1; cptr++)
		{
			// printf ("%d\n", *cptr);
			if (*cptr == ' ' || *cptr == '\n' || *cptr == '\0')
			{
				*cptr = '\0';
				*(attrib_data + data_idx) = atof(rptr);
				// printf ("Idx %d: Read %3.3f\n", data_idx, *(attrib_data + data_idx));
				data_idx++;
				rptr = cptr + 1;
				if (*rptr ==  '\0')
				{
					break;
				}
			}
		}
	}
	return getVAOFromAttribData (attrib_data, num_of_vertex, num_of_floats_per_attribute, num_of_attributes);
}

VAO* loadBezierSurfaceControlPoints(string path)
{
	//(Optional)TODO: load surface control point data and return VAO.
	//You can make use of getVAOFromAttribData in opengl_utils.h
	//TODO: load spline control point data and return VAO
	//You can make use of getVAOFromAttribData in opengl_utils.h
	char buffer[1024] = {0};
	FILE *fptr = fopen(path.c_str(), "r");
	fgets(buffer, 1024, fptr);
	size_t num_of_vertex = atoi(buffer);
	size_t num_of_attributes = 1; // Position vector

	float* attrib_data = (float*)calloc (num_of_vertex*3, sizeof(float));
	size_t* num_of_floats_per_attribute = (size_t*)calloc  (num_of_attributes, sizeof(size_t));
	*(num_of_floats_per_attribute) = 3;
	int data_idx = 0;
	for (int i = 0; i < num_of_vertex; i++)
	{
		bzero (buffer, 1024*sizeof(char));
		fgets(buffer, 1024, fptr);
		char* rptr = buffer;
		for (char* cptr = buffer; 1; cptr++)
		{
			// printf ("%d\n", *cptr);
			if (*cptr == ' ' || *cptr == '\n' || *cptr == '\0')
			{
				*cptr = '\0';
				*(attrib_data + data_idx) = atof(rptr);
				// printf ("Idx %d: Read %3.3f\n", data_idx, *(attrib_data + data_idx));
				data_idx++;
				rptr = cptr + 1;
				if (*rptr ==  '\0')
				{
					break;
				}
			}
		}
	}
	return getVAOFromAttribData (attrib_data, num_of_vertex, num_of_floats_per_attribute, num_of_attributes);
	
}
#endif