#ifndef SNAKE_H
#define SNAKE_H

#include "framework.h"
#include "raster.h"
#include "mouse.h"
#include <deque>

/**
* Time in seconds it takes the snake to move one voxel forward.
*/
const double TIME_PER_MOVE = 5.0f;


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

enum RelativeDirection {
    FORWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
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

    /**
    *   Draws the snake and the mouse
    */
    void draw(glm::mat4 projectionMatrix, double elapsedTime);

    /**
    *   Moves the body of the snake in the specified Direction. Replaces the head of the snake
    *   with a body or corner part and adds a new head in the forward direction. Removes the last
    *   part of the snake and replaces the second to last part with a tail. 
    */
    void move(Direction direction);

    /**
    *   Calculates the correct view-matrix according to the current position and direction
    *   of the head.
    */
    glm::mat4 getViewMatrix();
private:
    /**
    *   Parts of the snake. First part should be a head, last part a tail.
    */
    std::deque<SnakePart> parts;

    // current view-matrix
    glm::mat4 viewMatrix;

    /**
    *   Position of the head at the current frame.
    */
    glm::vec3 headPosition;

    // current up-vector of the head
    glm::vec3 headUp;
    Direction upDirection;

    // current angle of the head from the original forward-direction
    float headAngle;

    bool hasEnteredNextVoxel;

    Raster* raster;
    Mouse mouse;

    /**
    *   model-matrix to be used as a basis for all model-matrices.
    */
    glm::mat4 modelMatrix;

    /**
    *   the new direction for the snake relative to current direction
    */
    RelativeDirection newRelativeDirection;
    RelativeDirection currentRelativeDirection;
    RelativeDirection previousRelativeDirection;

    /**
    *   time in seconds elapsed since the last call of move()
    */
    double elapsedTimeSinceLastMove;

    GLuint programID;
    GLuint meshes[4];
    GLuint transformationsUBOID;
    GLint transformationsBlockIndex;
    GLuint snakeheadTextureID;
    GLenum snakeheadTextureTarget;
	GLuint snakebodyTextureID;
	GLenum snakebodyTextureTarget;
    GLuint mouseTextureID;
    GLenum mouseTextureTarget;
    GLuint textureLocation;

    /**
    *   Loads the object-files for all body parts of the snake and the mouse
    */
    void loadObjects();

    /**
    *   Loads the textures for all snake parts and the mouse.
    */
    void loadTexture();

    glm::mat4 rotatePart(SnakePart* part);
    void drawMesh(Transformation transformation, glm::mat4 projectionMatrix, int mesh);

    glm::ivec3 directionToVector(Direction direction);
    Direction invertDirection(Direction direction);
    Direction relativeDirectionToAbsoluteDirection(Direction forward, RelativeDirection relDir);

    void createViewMatrix();
};

#endif SNAKE_H