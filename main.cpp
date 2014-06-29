//Snake 3D

#include "framework.h"

struct Material {
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	float shininess;
};

struct Transformation {
    glm::mat4 ModelViewMatrix;
    glm::mat4 ProjectionMatrix;
    glm::mat4 NormalMatrix;
};

int wd = 640;
int ht = 640;
    
const int MATERIALS_ID = 0;
const int TRANSFORMATIONS_ID = 1;
const int UBO_COUNT = 2;

const int SNAKE_PARTS = 3;

int main(int argc, char** argv)
{
    if (!viscg::OpenWindow(wd, ht, "Snake3D"))
    {
        std::cerr << "cannot create a window" << std::endl;
        exit(-1);
    }

    Material snakeMaterial;
    snakeMaterial.ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    snakeMaterial.diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    snakeMaterial.specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    snakeMaterial.shininess = 7.5f;

    //glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0.0f, -2.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 viewMatrix = glm::lookAt(glm::vec3(-1.5f, -0.5f, 0.0f), glm::vec3(1.0f, -0.5f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    
    glm::mat4 projectionMatrix = glm::perspective(45.0f, static_cast<float>(wd)/static_cast<float>(ht), 0.0f, 100.0f);

    Transformation snakeHeadTransformation;
    glm::mat4 headModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f))
        * glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(1.0f, 0.0f, 0.0f))
        * glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 1.0f, 0.0f))
        * glm::scale(glm::mat4(1.0f), glm::vec3(0.25f, 0.25f, 0.25f));
    snakeHeadTransformation.ModelViewMatrix = viewMatrix * headModelMatrix;
    snakeHeadTransformation.ProjectionMatrix = projectionMatrix;
    snakeHeadTransformation.NormalMatrix = snakeHeadTransformation.ModelViewMatrix;

    Transformation snakeBodyTransformation;
    glm::mat4 bodyModelMatrix = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 1.0f, 0.0f))
        * glm::scale(glm::mat4(1.0f), glm::vec3(0.25f, 0.25f, 0.25f));
    snakeBodyTransformation.ModelViewMatrix = viewMatrix * bodyModelMatrix;
    snakeBodyTransformation.ProjectionMatrix = projectionMatrix;
    snakeBodyTransformation.NormalMatrix = snakeBodyTransformation.ModelViewMatrix;

    Transformation snakeTailTransformation;
    glm::mat4 tailModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.0f, 0.0f))
        * glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 1.0f, 0.0f))
        * glm::scale(glm::mat4(1.0f), glm::vec3(0.25f, 0.25f, 0.25f));
    snakeTailTransformation.ModelViewMatrix = viewMatrix * tailModelMatrix;
    snakeTailTransformation.ProjectionMatrix = projectionMatrix;
    snakeTailTransformation.NormalMatrix = snakeTailTransformation.ModelViewMatrix;

	Transformation mouseTransformation;
	glm::mat4 mouseModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.0f, 0.0f)) 
		* glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(1.0f, 0.0f, 0.0f))
		* glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));

	mouseTransformation.ModelViewMatrix = viewMatrix * mouseModelMatrix;
	mouseTransformation.ProjectionMatrix = projectionMatrix;
	mouseTransformation.NormalMatrix = mouseTransformation.ModelViewMatrix;

    // material and transformation ubos
    GLuint ubos[UBO_COUNT];
    glGenBuffers(UBO_COUNT, ubos);
    glBindBuffer(GL_UNIFORM_BUFFER, ubos[MATERIALS_ID]);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(snakeMaterial), &snakeMaterial, GL_STATIC_DRAW);

    // snake meshes
    GLuint snakeMeshes[SNAKE_PARTS];
    viscg::GenMeshes(SNAKE_PARTS, snakeMeshes);
    //viscg::LoadObject(snakeMeshes[0], "objects/snakecurve.obj");   
    viscg::LoadObject(snakeMeshes[0], "objects/snakehead.obj");
    viscg::LoadObject(snakeMeshes[1], "objects/snakebody.obj");
    viscg::LoadObject(snakeMeshes[2], "objects/snaketail.obj");

	//mouse mesh
	GLuint mouseMesh[1];
	viscg::GenMeshes(1, mouseMesh);
	viscg::LoadObject(mouseMesh[0], "objects/mouse.obj");


    // snake textures
    GLenum target;
    GLuint textureID = viscg::LoadTexture("objects/snakehead.png", target, true);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(target, textureID);

    GLuint programID = viscg::LoadSimpleProgram("shader/main.vs.glsl", "shader/main.fs.glsl");
    glUseProgram(programID);

    GLint blockIndex;
    blockIndex = glGetUniformBlockIndex(programID, "Materials");
    glUniformBlockBinding(programID, blockIndex, MATERIALS_ID);
    glBindBufferBase(GL_UNIFORM_BUFFER, MATERIALS_ID, ubos[MATERIALS_ID]);
    blockIndex = glGetUniformBlockIndex(programID, "Transformations");
    glUniformBlockBinding(programID, blockIndex, TRANSFORMATIONS_ID);
    glBindBufferBase(GL_UNIFORM_BUFFER, TRANSFORMATIONS_ID, ubos[TRANSFORMATIONS_ID]);

    int tex_loc = glGetUniformLocation(programID, "basic_texture");
    glUniform1i(tex_loc, 0);

    // main rendering loop
    while(viscg::ProcessAllMesages())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindBuffer(GL_UNIFORM_BUFFER, ubos[TRANSFORMATIONS_ID]);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(snakeHeadTransformation), &snakeHeadTransformation, GL_STATIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, TRANSFORMATIONS_ID, ubos[TRANSFORMATIONS_ID]);
        viscg::DrawMesh(snakeMeshes[0]);

        glBindBuffer(GL_UNIFORM_BUFFER, ubos[TRANSFORMATIONS_ID]);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(snakeBodyTransformation), &snakeBodyTransformation, GL_STATIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, TRANSFORMATIONS_ID, ubos[TRANSFORMATIONS_ID]);
        viscg::DrawMesh(snakeMeshes[1]);

        glBindBuffer(GL_UNIFORM_BUFFER, ubos[TRANSFORMATIONS_ID]);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(snakeTailTransformation), &snakeTailTransformation, GL_STATIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, TRANSFORMATIONS_ID, ubos[TRANSFORMATIONS_ID]);
        viscg::DrawMesh(snakeMeshes[2]);

		glBindBuffer(GL_UNIFORM_BUFFER, ubos[TRANSFORMATIONS_ID]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(mouseTransformation), &mouseTransformation, GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, TRANSFORMATIONS_ID, ubos[TRANSFORMATIONS_ID]);
		viscg::DrawMesh(mouseMesh[0]);

        viscg::SwapBuffers();
    }
    return 0;
}
