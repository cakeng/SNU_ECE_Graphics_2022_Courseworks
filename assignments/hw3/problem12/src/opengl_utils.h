#ifndef OPENGL_UTILS_H
#define OPENGL_UTILS_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
class VAO{
public:
    unsigned int ID;
    unsigned int associatedVBOID;
    unsigned int num_of_vertex;
};
// This function is very general VAO loader. Make use of it.
VAO *getVAOFromAttribData(float *attrib_data, size_t num_of_vertex, size_t *num_of_floats_per_attribute, size_t num_of_attributes)
{
    VAO *vao = new VAO();
    //unsigned int VBO;
    glGenVertexArrays(1, &(vao->ID));
    glGenBuffers(1, &(vao->associatedVBOID));
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(vao->ID);
    glBindBuffer(GL_ARRAY_BUFFER, vao->associatedVBOID);

    size_t attribute_offset_by_num_of_floats = 0;
    size_t attribute_index = 0;
    size_t total_num_of_floats_per_vertex = 0;
    for (int i = 0; i < num_of_attributes; i++)
    {
        total_num_of_floats_per_vertex += *(num_of_floats_per_attribute + i);
    }
    vao->num_of_vertex = num_of_vertex;

    glBufferData(GL_ARRAY_BUFFER, num_of_vertex * total_num_of_floats_per_vertex * sizeof(float), &attrib_data[0], GL_STATIC_DRAW);

    for (int i = 0; i < num_of_attributes; i++)
    {
        glVertexAttribPointer(attribute_index, *(num_of_floats_per_attribute + i), 
            GL_FLOAT, GL_FALSE, total_num_of_floats_per_vertex * sizeof(float)
                , (void*)(attribute_offset_by_num_of_floats * sizeof(float)));
        glEnableVertexAttribArray(attribute_index);
        attribute_index++;
        attribute_offset_by_num_of_floats += *(num_of_floats_per_attribute + i);
    }
    free (num_of_floats_per_attribute);
    return vao;
}
#endif
