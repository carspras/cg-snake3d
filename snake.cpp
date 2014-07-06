#include "snake.h"

const int TRANSFORMATIONS_ID = 0;

Snake::Snake(Raster* raster) {
    this->raster = raster;
    //mouse.position = glm::ivec3(4, 5, 2);
	headPosition = raster->getPosition(glm::ivec3(4,4,4));
    headUp = glm::vec3(0.0f, 1.0f, 0.0f);
    upDirection = PosY;
    headAngle = 0.0f;
    hasEnteredNextVoxel = false;

    viewMatrix = glm::lookAt(glm::vec3(0.0f, 4.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));

    newRelativeDirection = RIGHT;
    currentRelativeDirection = FORWARD;
    previousRelativeDirection = FORWARD;
    elapsedTimeSinceLastMove = 0.0f;

    programID = viscg::LoadSimpleProgram("shader/snake.vs.glsl", "shader/snake.fs.glsl");
    float scale = 1.0f * raster->getVoxelWidth() / 2.0f;
    modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
    
    glGenBuffers(1, &transformationsUBOID);
    transformationsBlockIndex = glGetUniformBlockIndex(programID, "Transformations");

    loadObjects();
    loadTexture();
    
    // Test-Schlange
    parts.push_back(SnakePart(TAIL, glm::ivec3(4, 4, 4), NegZ, PosZ));
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

	SetMouseRandomly();
}

void Snake::draw(glm::mat4 projectionMatrix, double elapsedTime) {
    elapsedTimeSinceLastMove += elapsedTime;
    
    if (!hasEnteredNextVoxel && elapsedTimeSinceLastMove > 0.5f * TIME_PER_MOVE) {
        previousRelativeDirection = currentRelativeDirection;
        currentRelativeDirection = newRelativeDirection;
        newRelativeDirection = FORWARD;
        hasEnteredNextVoxel = true;
        headAngle = 0.0f;
    } else if (elapsedTimeSinceLastMove > TIME_PER_MOVE) {
        move(NegX);
        elapsedTimeSinceLastMove = 0.0f;
        hasEnteredNextVoxel = false;
        headAngle = 0.0f;
    }

    glUseProgram(programID);

    glUniformBlockBinding(programID, transformationsBlockIndex, TRANSFORMATIONS_ID);
    glBindBufferBase(GL_UNIFORM_BUFFER, TRANSFORMATIONS_ID, transformationsUBOID);
    
    Transformation transformation;
    
    SnakePart head = parts.front();
    
    // draw head
    transformation.ModelViewMatrix = modelMatrix;
    float forwardCorrection = 0.0f;
    float sideCorrection = 0.0f;
    glm::mat4 headRotation(1.0f);
    float cos = 0.0f;

    if (elapsedTimeSinceLastMove < 0.5f * TIME_PER_MOVE) {
        switch (currentRelativeDirection) {
        case FORWARD:
            // draw head part
            transformation.ModelViewMatrix = rotatePart(&head) * transformation.ModelViewMatrix;

            headPosition = raster->getPosition(head.position);
            headPosition += raster->getVoxelWidth() * static_cast<float>(elapsedTimeSinceLastMove / TIME_PER_MOVE) 
                * glm::vec3(directionToVector(head.forwardDirection));
            transformation.ModelViewMatrix = glm::translate(glm::mat4(1.0f), headPosition)
                * transformation.ModelViewMatrix;
            break;
        case LEFT:
            headAngle = 270 + 90 * (0.5f + static_cast<float>(elapsedTimeSinceLastMove / TIME_PER_MOVE));
            forwardCorrection = static_cast<float>(0.5 * (1 - glm::abs(glm::sin(glm::radians(headAngle))))) * raster->getVoxelWidth();
            sideCorrection = static_cast<float>(0.5 * (1 - glm::cos(glm::radians(headAngle)))) * raster->getVoxelWidth();

            headRotation = rotatePart(&head);
            headRotation = glm::rotate(glm::mat4(1.0f), headAngle, headUp) * headRotation;
            transformation.ModelViewMatrix = headRotation * transformation.ModelViewMatrix;

            headPosition = raster->getPosition(head.position);
            headPosition += forwardCorrection * glm::vec3(directionToVector(head.forwardDirection));
            headPosition += -sideCorrection * glm::cross(glm::vec3(directionToVector(head.forwardDirection)), headUp);
            transformation.ModelViewMatrix = glm::translate(glm::mat4(1.0f), headPosition)
                * transformation.ModelViewMatrix;
            break;
        case RIGHT:
            headAngle = 90 * (0.5f - static_cast<float>(elapsedTimeSinceLastMove / TIME_PER_MOVE));
            forwardCorrection = static_cast<float>(0.5 * (1 - glm::abs(glm::sin(glm::radians(headAngle))))) * raster->getVoxelWidth();
            sideCorrection = static_cast<float>(0.5 * (1 - glm::cos(glm::radians(headAngle)))) * raster->getVoxelWidth();

            headRotation = rotatePart(&head);
            headRotation = glm::rotate(glm::mat4(1.0f), headAngle, headUp) * headRotation;
            transformation.ModelViewMatrix = headRotation * transformation.ModelViewMatrix;

            headPosition = raster->getPosition(head.position);
            headPosition += forwardCorrection * glm::vec3(directionToVector(head.forwardDirection));
            headPosition += sideCorrection * glm::cross(glm::vec3(directionToVector(head.forwardDirection)), headUp);
            transformation.ModelViewMatrix = glm::translate(glm::mat4(1.0f), headPosition)
                * transformation.ModelViewMatrix;
            break;
        case UP:
            break;
        case DOWN:
            break;
        default:
            break;
        }
    } else {
        switch (currentRelativeDirection) {
        case FORWARD:
            // draw head part
            transformation.ModelViewMatrix = rotatePart(&head) * transformation.ModelViewMatrix;

            headPosition = raster->getPosition(head.position);
            headPosition += raster->getVoxelWidth() * static_cast<float>(elapsedTimeSinceLastMove / TIME_PER_MOVE) 
                * glm::vec3(directionToVector(head.forwardDirection));
            transformation.ModelViewMatrix = glm::translate(glm::mat4(1.0f), headPosition)
                * transformation.ModelViewMatrix;
            break;
        case LEFT:
            headAngle = 90 * (-0.5f + static_cast<float>(elapsedTimeSinceLastMove / TIME_PER_MOVE));
            forwardCorrection = static_cast<float>((0.5 * (1 + glm::sin(glm::radians(headAngle)))) * raster->getVoxelWidth());
            sideCorrection = static_cast<float>(0.5 * (1 - glm::cos(glm::radians(headAngle))) * raster->getVoxelWidth());

            headRotation = rotatePart(&head);
            headRotation = glm::rotate(glm::mat4(1.0f), headAngle, headUp) * headRotation;
            transformation.ModelViewMatrix = headRotation * transformation.ModelViewMatrix;

            headPosition = raster->getPosition(head.position);
            headPosition += forwardCorrection * glm::vec3(directionToVector(head.forwardDirection));
            headPosition += -sideCorrection * glm::cross(glm::vec3(directionToVector(head.forwardDirection)), headUp);
            transformation.ModelViewMatrix = glm::translate(glm::mat4(1.0f), headPosition)
                * transformation.ModelViewMatrix;
            break;
        case RIGHT:
            headAngle = 360 - 90 * (-0.5f + static_cast<float>(elapsedTimeSinceLastMove / TIME_PER_MOVE));
            forwardCorrection = static_cast<float>((0.5 * (1 + glm::abs(glm::sin(glm::radians(headAngle))))) * raster->getVoxelWidth());
            sideCorrection = static_cast<float>(0.5 * (1 - glm::cos(glm::radians(headAngle))) * raster->getVoxelWidth());

            headRotation = rotatePart(&head);
            headRotation = glm::rotate(glm::mat4(1.0f), headAngle, headUp) * headRotation;
            transformation.ModelViewMatrix = headRotation * transformation.ModelViewMatrix;

            headPosition = raster->getPosition(head.position);
            headPosition += forwardCorrection * glm::vec3(directionToVector(head.forwardDirection));
            headPosition += sideCorrection * glm::cross(glm::vec3(directionToVector(head.forwardDirection)), headUp);
            transformation.ModelViewMatrix = glm::translate(glm::mat4(1.0f), headPosition)
                * transformation.ModelViewMatrix;
            break;
        case UP:
            break;
        case DOWN:
            break;
        default:
            break;
        }
    }
    
    createViewMatrix();
    drawMesh(transformation, projectionMatrix, TAIL);
    parts.pop_front();

    // draw all other parts of the snake
    for each (SnakePart part in parts){
        transformation.ModelViewMatrix = modelMatrix;
        transformation.ModelViewMatrix = rotatePart(&part) * transformation.ModelViewMatrix;
        transformation.ModelViewMatrix = glm::translate(glm::mat4(1.0f), raster->getPosition(part.position))
            * transformation.ModelViewMatrix;

        drawMesh(transformation, projectionMatrix, part.type);
    }
    parts.push_front(head);

    
    //// draw mouse
    //transformation.ModelViewMatrix = modelMatrix;
    //transformation.ModelViewMatrix = glm::translate(glm::mat4(1.0f), raster->getPosition(mouse.position))
    //    * glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f))
    //    * transformation.ModelViewMatrix;

    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(mouseTextureTarget, mouseTextureID);
    //glUniform1i(textureLocation, 0);

    //drawMesh(transformation, projectionMatrix, 4);
    
}

void Snake::move(Direction direction) {
    SnakePart oldHead = parts.front();
    parts.pop_front();

    glm::ivec3 newPosition = oldHead.position + directionToVector(oldHead.forwardDirection);

    if (raster->isOutOfBounds(newPosition)) {
        return;
    }
        
    if (previousRelativeDirection == FORWARD) {
        oldHead.type = BODY;
    } else {
        oldHead.type = CORNER;
        oldHead.backwardDirection = invertDirection(parts.front().forwardDirection);
    }
    parts.push_front(oldHead);

    Direction newForwardDirection = relativeDirectionToAbsoluteDirection(oldHead.forwardDirection, currentRelativeDirection);
    parts.push_front(SnakePart(TAIL, newPosition, newForwardDirection, invertDirection(newForwardDirection)));

    parts.pop_back();
    parts.back().type = TAIL;

    headPosition = raster->getPosition(parts.front().position);
}

void Snake::createViewMatrix() {
    SnakePart head = parts.front();
    
    glm::vec3 forward(directionToVector(head.forwardDirection));

    glm::vec3 rotatedForward(glm::rotate(glm::mat4(1.0f), headAngle, headUp) * glm::vec4(forward, 1.0f));

    glm::vec3 eye = headPosition;
    eye += raster->getVoxelWidth() / 2.0f * headUp;
    glm::vec3 correction = (-2.0f * raster->getVoxelWidth()) * rotatedForward;
    eye += (-2.0f * raster->getVoxelWidth()) * rotatedForward;
    
    glm::vec3 center = headPosition;
    center += raster->getVoxelWidth() / 2.0f * headUp;

    viewMatrix = glm::lookAt(eye, center, headUp);
}

glm::mat4 Snake::getViewMatrix() {
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
    //viscg::LoadObject(meshes[0], "objects/snakehead.obj");
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
    if (mesh == 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(snakeheadTextureTarget, snakeheadTextureID);
        glUniform1i(textureLocation, 0);
    } else {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(snakebodyTextureTarget, snakebodyTextureID);
        glUniform1i(textureLocation, 0);
    }
    
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

Direction Snake::relativeDirectionToAbsoluteDirection(Direction forward, RelativeDirection relDir) {
    switch (relDir) {
    case FORWARD:
        return forward;
    case LEFT:
        switch (forward) {
        case PosX:
            switch (upDirection) {
            case PosY:
                return NegZ;
            case NegY:
                return PosZ;
            case PosZ:
                return PosY;
            case NegZ:
                return NegY;
            default:
                return forward;
            }
        case NegX:
            switch (upDirection) {
            case PosY:
                return PosZ;
            case NegY:
                return NegZ;
            case PosZ:
                return NegY;
            case NegZ:
                return PosY;
            default:
                return forward;
            }
        case PosY:
            switch (upDirection) {
            case PosX:
                return PosZ;
            case NegX:
                return NegZ;
            case PosZ:
                return NegX;
            case NegZ:
                return PosX;
            default:
                return forward;
            }
        case NegY:
            switch (upDirection) {
            case PosX:
                return NegZ;
            case NegX:
                return PosZ;
            case PosZ:
                return PosX;
            case NegZ:
                return NegX;
            default:
                return forward;
            }
        case PosZ:
            switch (upDirection) {
            case PosX:
                return PosY;
            case NegX:
                return NegY;
            case PosY:
                return PosX;
            case NegY:
                return NegX;
            default:
                return forward;
            }
        case NegZ:
            switch (upDirection) {
            case PosX:
                return PosY;
            case NegX:
                return NegY;
            case PosY:
                return NegX;
            case NegY:
                return PosX;
            default:
                return forward;
            }
        default:
            return forward;
        }
        return forward;
    case RIGHT:
        switch (forward) {
        case PosX:
            switch (upDirection) {
            case PosY:
                return PosZ;
            case NegY:
                return NegZ;
            case PosZ:
                return NegY;
            case NegZ:
                return PosY;
            default:
                return forward;
            }
        case NegX:
            switch (upDirection) {
            case PosY:
                return NegZ;
            case NegY:
                return PosZ;
            case PosZ:
                return PosY;
            case NegZ:
                return NegY;
            default:
                return forward;
            }
        case PosY:
            switch (upDirection) {
            case PosX:
                return NegZ;
            case NegX:
                return PosZ;
            case PosZ:
                return PosX;
            case NegZ:
                return NegX;
            default:
                return forward;
            }
        case NegY:
            switch (upDirection) {
            case PosX:
                return PosZ;
            case NegX:
                return NegZ;
            case PosZ:
                return NegX;
            case NegZ:
                return PosX;
            default:
                return forward;
            }
        case PosZ:
            switch (upDirection) {
            case PosX:
                return PosY;
            case NegX:
                return NegY;
            case PosY:
                return NegX;
            case NegY:
                return PosX;
            default:
                return forward;
            }
        case NegZ:
            switch (upDirection) {
            case PosX:
                return NegY;
            case NegX:
                return PosY;
            case PosY:
                return PosX;
            case NegY:
                return NegX;
            default:
                return forward;
            }
        default:
            return forward;
        }
        return forward;
    case UP:
        return upDirection;
    case DOWN:
        return invertDirection(upDirection);
    default:
        return forward;
    }
}

bool Snake::IsAtPosition(glm::ivec3 position){

	std::deque<SnakePart>::iterator it = parts.begin();

	while (it != parts.end()){
		SnakePart current = *it++;
		if (position == current.position) return true;
	}

	return false;
}

void Snake::SetMouseRandomly(){

	int x, y, z;
	float f;
	bool snakeposition = true;

	while (snakeposition){

		x = rand() % (raster->getVoxelsPerLine()+1);
		y = rand() % (raster->getVoxelsPerLine()+1);
		z = rand() % (raster->getVoxelsPerLine()+1);

		snakeposition = IsAtPosition(glm::ivec3(x, y, z));
	}

	mouse.position = glm::ivec3(x, y, z);

}