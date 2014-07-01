#ifndef SNAKE_H
#define SNAKE_H

#include "framework.h"
#include "raster.h"
#include "mouse.h"
#include <deque>

struct Transformation {
    glm::mat4 ModelViewMatrix;
    glm::mat4 ProjectionMatrix;
    glm::mat4 NormalMatrix;
};

enum SnakeBodyType {
    HEAD,
    BODY,
    CORNER,
    TAIL
};

enum Direction {
    PosX,
    NegX,
    PosY,
    NegY,
    PosZ,
    NegZ
};

struct SnakePart {
    SnakeBodyType type;
    glm::ivec3 position;
    Direction forwardDirection;
    Direction backwardDirection;

    SnakePart(SnakeBodyType t, glm::ivec3 pos, Direction forw, Direction back) {
        type = t;
        position = pos;
        forwardDirection = forw;
        backwardDirection = back;
    }
};

class Snake {
public:
    Snake(Raster* raster);
    void draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);

    glm::mat4 getViewMatrix();

private:
    std::deque<SnakePart> parts;
    glm::vec3 up;

    Raster* raster;
    Mouse mouse;
    glm::mat4 modelMatrix;

    GLuint programID;
    GLuint meshes[4];
    GLuint transformationsUBOID;
    GLint transformationsBlockIndex;
    GLuint snakeTextureID;
    GLenum snakeTextureTarget;
    GLuint mouseTextureID;
    GLenum mouseTextureTarget;
    GLuint textureLocation;

    void loadObjects();
    void loadTexture();

    glm::mat4 rotatePart(SnakePart* part);
};

#endif SNAKE_H