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
    Direction upDirection;

    SnakePart(SnakeBodyType t, glm::ivec3 pos, Direction forw, Direction back, Direction up) {
        type = t;
        position = pos;
        forwardDirection = forw;
        backwardDirection = back;
        upDirection = up;
    }
};

enum GameOverCondition {
    NOT_OVER,
    RAN_INTO_WALL,
    BIT_ITSELF
};

class Snake {
public:
    Snake(Raster* raster);

    // Draws the snake and the mouse
    void draw(glm::mat4 projectionMatrix, double elapsedTime);

    // checks if the game is over and returns the appropiate condition
    GameOverCondition gameOver();
    unsigned int getScore();
    unsigned int getMoves();

    //  resets all variables of the snake to the initial states
    void reset();

    /**
    *   Moves the body of the snake in the specified Direction. Replaces the head of the snake
    *   with a body or corner part and adds a new head in the forward direction. Removes the last
    *   part of the snake and replaces the second to last part with a tail. 
    */
    void move();

    void navigate(RelativeDirection newDirection);

    /**
    *   Calculates the correct view-matrix according to the current position and direction
    *   of the head.
    */
    glm::mat4 getViewMatrix();

	// sets mouse position new at a random point
	void Snake::SetMouseRandomly();

private:
    // current score of the player; should increase every time the snake eats a mouse
    unsigned int score;
    
    // moves the snake completed since the start of the game
    unsigned int moves;

    // time (in seconds) it takes the snake to move one voxel forward.
    double timePerMove;

    //  the new direction for the next move of the snake
    RelativeDirection newRelativeDirection;

    // the direction for the current move of the snake
    RelativeDirection currentRelativeDirection;

    // is used during the move to calculate the type and orientation of the first element of the body
    RelativeDirection previousRelativeDirection;

    // true if the snake has entered the next voxel, false if the snake is still in the original voxel
    bool hasEnteredNextVoxel;

    // true if the game is over because the snake has run into a wall.
    GameOverCondition isGameOver;
    
    // Parts of the snake. First part should be a head, last part a tail.
    std::deque<SnakePart> parts;

    // the food of the snake
    Mouse mouse;

    // time in seconds elapsed since the last call of move()
    double elapsedTimeSinceLastMove;

    // model-matrix to be used as a basis for all model-matrices.
    glm::mat4 modelMatrix;

    // current view-matrix
    glm::mat4 viewMatrix;
    
    // position of the head at the current frame.
    glm::vec3 headPosition;
    glm::vec3 headForward;

    // current up-vector of the head
    glm::vec3 headUp;

    // the current axis the head has to be rotated around
    glm::vec3 rotationAxis;

    // current angle of the head from the original forward-direction
    float headAngle;

    Raster* raster;
    
    // rendering stuff
    GLuint programID;
    GLuint meshes[5];
    GLuint transformationsUBOID;
    GLint transformationsBlockIndex;
    GLuint snakeheadTextureID;
    GLenum snakeheadTextureTarget;
	GLuint snakebodyTextureID;
	GLenum snakebodyTextureTarget;
    GLuint mouseTextureID;
    GLenum mouseTextureTarget;
    GLuint textureLocation;

    // Loads the object-files for all body parts of the snake and the mouse
    void loadObjects();

    // Loads the textures for all snake parts and the mouse.
    void loadTexture();

    // snake eats the mouse, creates a new mouse at a random position and increases the score
    void eat();

    // checks if any part of the snake body is at a given position
	bool IsAtPosition(glm::ivec3 position);

    /**
    *  rotates the part to the correct orientation for rendering
    *  according to the forward- and up-direction of the part
    */
    glm::mat4 rotatePart(SnakePart* part);
    
    /**
    *   Draws an instance of the mesh at position mesh of the meshes-array
    *   with the transformation- and projection-matrix passed and the current view-matrix
    */
    void drawMesh(Transformation transformation, glm::mat4 projectionMatrix, int mesh);

    // converts the passed direction into a normalized vector
    glm::ivec3 directionToVector(Direction direction);

    // Inverts the passed direction (PosX -> NegX etc.)
    Direction invertDirection(Direction direction);

    // Calculates the absolute direction of the part with the relative direction given
    Direction relativeDirectionToAbsoluteDirection(SnakePart* part, RelativeDirection relDir);

    //  Calculates the new view matrix after the head of the snake has moved
    void createViewMatrix();
};

#endif SNAKE_H