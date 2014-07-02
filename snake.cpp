#include "snake.h"

const int TRANSFORMATIONS_ID = 0;

Snake::Snake(Raster* raster) {
    this->raster = raster;
    mouse.position = glm::ivec3(4, 5, 2);
    headPosition = raster->getPosition(glm::ivec3(4,4,4));
    up = glm::vec3(0.0f, 1.0f, 0.0f);

    newDirection = NegZ;
    elapsedTimeSinceLastMove = 0.0f;

    programID = viscg::LoadSimpleProgram("shader/snake.vs.glsl", "shader/snake.fs.glsl");
    float scale = 1.0f * raster->getVoxelWidth() / 2.0f;
    modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
    
    glGenBuffers(1, &transformationsUBOID);
    transformationsBlockIndex = glGetUniformBlockIndex(programID, "Transformations");

    loadObjects();
    loadTexture();
    
    // Test-Schlange
    parts.push_back(SnakePart(TAIL, glm::ivec3(4, 4, 24), NegZ, PosZ));
    parts.push_back(SnakePart(BODY, glm::ivec3(4, 4, 5), NegZ, PosZ));
    parts.push_back(SnakePart(CORNER, glm::ivec3(4, 4, 6), NegZ, NegY));
    parts.push_back(SnakePart(BODY, glm::ivec3(4, 3, 6), PosY, NegY));
    parts.push_back(SnakePart(CORNER, glm::ivec3(4, 2, 6), PosY, PosX));
    parts.push_back(SnakePart(BODY, glm::ivec3(5, 2, 6), NegX, PosX));
    parts.push_back(SnakePart(CORNER, glm::ivec3(6, 2, 6), NegX, NegZ));
    parts.push_back(SnakePart(BODY, glm::ivec3(6, 2, 5), PosZ, NegZ));
    parts.push_back(SnakePart(BODY, glm::ivec3(6, 2, 4), PosZ, NegZ));
    parts.push_back(SnakePart(BODY, glm::ivec3(6, 2, 3), PosZ, NegZ));
    parts.push_back(SnakePart(BODY, glm::ivec3(6, 2, 2), PosZ, NegZ));
    parts.push_back(SnakePart(BODY, glm::ivec3(6, 2, 1), PosZ, NegZ));
    parts.push_back(SnakePart(CORNER, glm::ivec3(6, 2, 0), PosZ, NegX));
    parts.push_back(SnakePart(BODY, glm::ivec3(5, 2, 0), PosX, NegX));
    parts.push_back(SnakePart(BODY, glm::ivec3(4, 2, 0), PosX, NegX));
    parts.push_back(SnakePart(BODY, glm::ivec3(3, 2, 0), PosX, NegX));
    parts.push_back(SnakePart(CORNER, glm::ivec3(2, 2, 0), PosX, PosY));
    parts.push_back(SnakePart(BODY, glm::ivec3(2, 3, 0), NegY, PosY));
    parts.push_back(SnakePart(CORNER, glm::ivec3(2, 4, 0), NegY, PosX));
    parts.push_back(SnakePart(BODY, glm::ivec3(3, 4, 0), NegX, PosX));
    parts.push_back(SnakePart(TAIL, glm::ivec3(4, 4, 0), NegX, PosX));
}

void Snake::draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrix, double elapsedTime) {
    elapsedTimeSinceLastMove += elapsedTime;
    
    if (elapsedTimeSinceLastMove > TIME_PER_MOVE) {
        move(NegZ);
        elapsedTimeSinceLastMove = 0.0f;
        elapsedTime = 0.0f;
    }
    
    glUseProgram(programID);

    glUniformBlockBinding(programID, transformationsBlockIndex, TRANSFORMATIONS_ID);
    glBindBufferBase(GL_UNIFORM_BUFFER, TRANSFORMATIONS_ID, transformationsUBOID);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(snakeheadTextureTarget, snakeheadTextureID);
    glUniform1i(textureLocation, 0);

    Transformation transformation;
    
    SnakePart head = parts.front();
    parts.pop_front();

    // draw head
    transformation.ModelViewMatrix = modelMatrix;
    
    if (newDirection == head.forwardDirection) {
        // draw head part
        headPosition += raster->getVoxelWidth() * static_cast<float>(elapsedTime / TIME_PER_MOVE) 
            * glm::vec3(directionToVector(newDirection));
        transformation.ModelViewMatrix = rotatePart(&head) * transformation.ModelViewMatrix;
        transformation.ModelViewMatrix = glm::translate(glm::mat4(1.0f), headPosition)
            * transformation.ModelViewMatrix;

        drawMesh(transformation, projectionMatrix, HEAD);

        // draw first part of the body
        transformation.ModelViewMatrix = rotatePart(&head) * modelMatrix;
        transformation.ModelViewMatrix = glm::translate(glm::mat4(1.0f), headPosition 
            + raster->getVoxelWidth() * glm::vec3(directionToVector(head.backwardDirection)))
            * transformation.ModelViewMatrix;

        drawMesh(transformation, projectionMatrix, BODY);
    } else {

    }

    // draw all other parts of the snake
    for each (SnakePart part in parts){
        transformation.ModelViewMatrix = modelMatrix;
        transformation.ModelViewMatrix = rotatePart(&part) * transformation.ModelViewMatrix;
        transformation.ModelViewMatrix = glm::translate(glm::mat4(1.0f), raster->getPosition(part.position))
            * transformation.ModelViewMatrix;

        drawMesh(transformation, projectionMatrix, part.type);
    }
    parts.push_front(head);

    /*
    // draw mouse
    transformation.ModelViewMatrix = modelMatrix;
    transformation.ModelViewMatrix = glm::translate(glm::mat4(1.0f), raster->getPosition(mouse.position))
        * glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f))
        * transformation.ModelViewMatrix;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(mouseTextureTarget, mouseTextureID);
    glUniform1i(textureLocation, 0);

    drawMesh(transformation, projectionMatrix, 4);
    */
}

void Snake::move(Direction direction) {
    SnakePart oldHead = parts.front();
    glm::ivec3 newPosition = oldHead.position + directionToVector(direction);

    if (raster->isOutOfBounds(newPosition)) {
        return;
    }
        
    if (direction == parts.front().forwardDirection) {
        parts.front().type = BODY;
    } else {
        parts.front().type = CORNER;
        parts.front().forwardDirection = direction;
    }
    parts.push_front(SnakePart(TAIL, newPosition, direction, invertDirection(direction)));

    parts.pop_back();
    parts.back().type = TAIL;

    headPosition = raster->getPosition(parts.front().position);
}

glm::mat4 Snake::getViewMatrix() {
    SnakePart head = parts.front();

    glm::vec3 direction = glm::vec3(directionToVector(head.backwardDirection));

    glm::vec3 eye = headPosition;
    eye += raster->getVoxelWidth() / 2.0f * up;
    eye += 2.0f * raster->getVoxelWidth() * direction;

    glm::vec3 center = headPosition;
    center += raster->getVoxelWidth() / 2.0f * up;

    glm::mat4 viewMatrix = glm::lookAt(eye, center, up);

    return viewMatrix;
}

void Snake::loadTexture() {
    glUseProgram(programID);
    snakeheadTextureID = viscg::LoadTexture("objects/snakehead.png", snakeheadTextureTarget, true);
	snakebodyTextureID = viscg::LoadTexture("objects/snakebody.png", snakebodyTextureTarget, true);
    mouseTextureID = viscg::LoadTexture("objects/mouse.png", mouseTextureTarget, true);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    textureLocation = glGetUniformLocation(programID, "basic_texture");
}

void Snake::loadObjects() {
    viscg::GenMeshes(5, meshes);
    viscg::LoadObject(meshes[0], "objects/snakehead.obj");
    viscg::LoadObject(meshes[1], "objects/snakebody.obj");
    viscg::LoadObject(meshes[2], "objects/snakecurve.obj");   
    viscg::LoadObject(meshes[3], "objects/snaketail.obj");
    //viscg::LoadObject(meshes[4], "objects/mouse.obj");
}

glm::mat4 Snake::rotatePart(SnakePart* part) {
    glm::mat4 rotationMatrix;
    
    switch (part->forwardDirection) {
    case PosX:
        rotationMatrix = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        break;
    case NegX:
        rotationMatrix = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        break;
    case PosY:
        rotationMatrix = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        break;
    case NegY:
        rotationMatrix = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        break;
    case PosZ:
        rotationMatrix = glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        break;
    default:
        break;
    }

    if (part->type == CORNER) {
        switch (part->forwardDirection){
        case PosX:
            switch (part->backwardDirection) {
            case PosY:
                rotationMatrix = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(1.0f, 0.0f, 0.0f)) * rotationMatrix;
                break;
            case NegY:
                rotationMatrix = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(1.0f, 0.0f, 0.0f)) * rotationMatrix;
                break;
            case NegZ:
                rotationMatrix = glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(1.0f, 0.0f, 0.0f)) * rotationMatrix;
                break;
            default:
                break;
            }
            break;
        case NegX:
            switch (part->backwardDirection) {
            case PosY:
                rotationMatrix = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(1.0f, 0.0f, 0.0f)) * rotationMatrix;
                break;
            case NegY:
                rotationMatrix = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(1.0f, 0.0f, 0.0f)) * rotationMatrix;
                break;
            case PosZ:
                rotationMatrix = glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(1.0f, 0.0f, 0.0f)) * rotationMatrix;
                break; 
            default:
                break;
            }
            break;
        case PosY:
            switch (part->backwardDirection) {
            case NegX:
                rotationMatrix = glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(0.0f, 1.0f, 0.0f)) * rotationMatrix;
                break;
            case PosZ:
                rotationMatrix = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 1.0f, 0.0f)) * rotationMatrix;
                break;
            case NegZ:
                rotationMatrix = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f)) * rotationMatrix;
                break;
            default:
                break;
            }
            break;
        case NegY:
            switch (part->backwardDirection) {
            case NegX:
                rotationMatrix = glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(0.0f, 1.0f, 0.0f)) * rotationMatrix;
                break;
            case PosZ:
                rotationMatrix = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 1.0f, 0.0f)) * rotationMatrix;
                break;
            case NegZ:
                rotationMatrix = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f)) * rotationMatrix;
                break;
            default:
                break;
            }
            break;
        case PosZ:
            switch (part->backwardDirection) {
            case NegX:
                rotationMatrix = glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(0.0f, 0.0f, 1.0f)) * rotationMatrix;
                break;
            case PosY:
                rotationMatrix = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 0.0f, 1.0f)) * rotationMatrix;
                break;
            case NegY:
                rotationMatrix = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 0.0f, 1.0f)) * rotationMatrix;
                break;
            default:
                break;
            }
            break;
        case NegZ:
            switch (part->backwardDirection) {
            case NegX:
                rotationMatrix = glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(0.0f, 0.0f, 1.0f)) * rotationMatrix;
                break;
            case PosY:
                rotationMatrix = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 0.0f, 1.0f)) * rotationMatrix;
                break;
            case NegY:
                rotationMatrix = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 0.0f, 1.0f)) * rotationMatrix;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
    return rotationMatrix;
}

void Snake::drawMesh(Transformation transformation, glm::mat4 projectionMatrix, int mesh) {
    glm::mat4 viewMatrix = getViewMatrix();
    transformation.ModelViewMatrix = viewMatrix * transformation.ModelViewMatrix;
    transformation.ProjectionMatrix = projectionMatrix;
    transformation.NormalMatrix = transformation.ModelViewMatrix;
    
    glBindBuffer(GL_UNIFORM_BUFFER, transformationsUBOID);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(transformation), &transformation, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, TRANSFORMATIONS_ID, transformationsUBOID);

    viscg::DrawMesh(meshes[mesh]);
}

glm::ivec3 Snake::directionToVector(Direction direction) {
    switch (direction) {
    case PosX:
        return glm::ivec3(1, 0, 0);
    case NegX:
        return glm::ivec3(-1, 0, 0);
    case PosY:
        return glm::ivec3(0, 1, 0);
    case NegY:
        return glm::ivec3(0, -1, 0);
    case PosZ:
        return glm::ivec3(0, 0, 1);
    default:
        return glm::ivec3(0, 0, -1);
    }    
}

Direction Snake::invertDirection(Direction direction) {
    switch (direction) {
    case PosX:
        return NegX;
    case NegX:
        return PosX;
    case PosY:
        return NegY;
    case NegY:
        return PosY;
    case PosZ:
        return NegZ;
    default:
        return PosZ;
    }
}