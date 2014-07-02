//Snake 3D

#include "framework.h"
#include "raster.h"
#include "snake.h"

int wd = 640;
int ht = 640;
    
const int RASTER_WIDTH = 2;
const int VOXELS_PER_LINE = 9;


int main(int argc, char** argv)
{
    if (!viscg::OpenWindow(wd, ht, "Snake3D"))
    {
        std::cerr << "cannot create a window" << std::endl;
        exit(-1);
    }

    // create raster
    Raster raster(RASTER_WIDTH, VOXELS_PER_LINE);
    Snake snake(&raster);
    
    glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0.0f, 4.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    //glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    
    glm::mat4 projectionMatrix = glm::perspective(45.0f, static_cast<float>(wd)/static_cast<float>(ht), 0.001f, 100.0f);

    // load programs
    GLuint defaultProgramID = viscg::LoadSimpleProgram("shader/default.vs.glsl", "shader/default.fs.glsl");
    
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // main rendering loop
    while(viscg::ProcessAllMesages()) { 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        viewMatrix = snake.getViewMatrix();
        raster.draw(viewMatrix, projectionMatrix);
        snake.draw(viewMatrix, projectionMatrix);

        viscg::SwapBuffers();
    }
    return 0;
}
