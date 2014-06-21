#include "framework.h"

int main(int argc, char** argv)
{
    if (!viscg::OpenWindow(600, 600, "Snake3D"))
    {
        std::cerr << "cannot create a window" << std::endl;
        exit(-1);
    }

    glm::vec3 positions[] =
    {
        glm::vec3(-1.0f, -1.0f, 0.0f),
        glm::vec3( 1.0f, -1.0f, 0.0f),
        glm::vec3( 0.0f,  1.0f, 0.0f)
    };
    
    GLuint snakeMeshes[1];
    viscg::GenMeshes(1, snakeMeshes);
    viscg::LoadObject(snakeMeshes[0], "gutelaunecube.obj");

    // buffer
    GLuint positionBufferID;
    glGenBuffers(1, &positionBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, positionBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    
    GLuint vaoID;
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    GLuint programID = viscg::LoadSimpleProgram("shader/main.vs.glsl", "shader/main.fs.glsl");
    glUseProgram(programID);

    // main rendering loop
    while(viscg::ProcessAllMesages())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        viscg::SwapBuffers();
    }
    return 0;
}
