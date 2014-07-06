#include "snake.h"

const int TRANSFORMATIONS_ID = 0;

Snake::Snake(Raster* raster) {
    this->raster = raster;
    mouse.position = glm::ivec3(4, 5, 2);
    headPosition = raster->getPosition(glm::ivec3(4,4,4));
    headForward = glm::vec3(0.0f, 0.0f, -1.0f);
    headUp = glm::vec3(0.0f, 1.0f, 0.0f);
    headAngle = 0.0f;
    hasEnteredNextVoxel = false;

    viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    rotationAxis = headUp;

    newRelativeDirection = FORWARD;
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
    parts.push_back(SnakePart(TAIL, glm::ivec3(4, 4, 4), NegZ, PosZ, PosY));
    parts.push_back(SnakePart(BODY, glm::ivec3(4, 4, 5), NegZ, PosZ, PosY));
    parts.push_back(SnakePart(CORNER, glm::ivec3(4, 4, 6), NegZ, NegY, PosY));
    parts.push_back(SnakePart(BODY, glm::ivec3(4, 3, 6), PosY, NegY, PosZ));
    parts.push_back(SnakePart(CORNER, glm::ivec3(4, 2, 6), PosY, PosX, PosZ));
    parts.push_back(SnakePart(BODY, glm::ivec3(5, 2, 6), NegX, PosX, PosZ));
    parts.push_back(SnakePart(CORNER, glm::ivec3(6, 2, 6), NegX, NegZ, PosZ));
    parts.push_back(SnakePart(BODY, glm::ivec3(6, 2, 5), PosZ, NegZ, PosX));
    parts.push_back(SnakePart(BODY, glm::ivec3(6, 2, 4), PosZ, NegZ, PosX));
    parts.push_back(SnakePart(BODY, glm::ivec3(6, 2, 3), PosZ, NegZ, PosX));
    parts.push_back(SnakePart(BODY, glm::ivec3(6, 2, 2), PosZ, NegZ, PosX));
    parts.push_back(SnakePart(BODY, glm::ivec3(6, 2, 1), PosZ, NegZ, PosX));
    parts.push_back(SnakePart(CORNER, glm::ivec3(6, 2, 0), PosZ, NegX, PosX));
    parts.push_back(SnakePart(BODY, glm::ivec3(5, 2, 0), PosX, NegX, NegZ));
    parts.push_back(SnakePart(BODY, glm::ivec3(4, 2, 0), PosX, NegX, NegZ));
    parts.push_back(SnakePart(BODY, glm::ivec3(3, 2, 0), PosX, NegX, NegZ));
    parts.push_back(SnakePart(CORNER, glm::ivec3(2, 2, 0), PosX, PosY, NegZ));
    parts.push_back(SnakePart(BODY, glm::ivec3(2, 3, 0), NegY, PosY, NegZ));
    parts.push_back(SnakePart(CORNER, glm::ivec3(2, 4, 0), NegY, PosX, NegZ));
    parts.push_back(SnakePart(BODY, glm::ivec3(3, 4, 0), NegX, PosX, NegZ));
    parts.push_back(SnakePart(TAIL, glm::ivec3(4, 4, 0), NegX, PosX, NegZ));

	SetMouseRandomly();
}

void Snake::draw(glm::mat4 projectionMatrix, double elapsedTime) {
    elapsedTimeSinceLastMove += elapsedTime;
    
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

    if (elapsedTimeSinceLastMove <= 0.5f * TIME_PER_MOVE) {
        switch (currentRelativeDirection) {
        case FORWARD:
            transformation.ModelViewMatrix = rotatePart(&head) * transformation.ModelViewMatrix;

            headPosition = raster->getPosition(head.position);
            headPosition += raster->getVoxelWidth() * static_cast<float>(elapsedTimeSinceLastMove / TIME_PER_MOVE) 
                * glm::vec3(directionToVector(head.forwardDirection));
            transformation.ModelViewMatrix = glm::translate(glm::mat4(1.0f), headPosition)
                * transformation.ModelViewMatrix;

            rotationAxis = headUp;
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
            
            rotationAxis = headUp;
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

            rotationAxis = headUp;
            break;
        case UP:
            headAngle = 90 * (0.5f - static_cast<float>(elapsedTimeSinceLastMove / TIME_PER_MOVE));
            forwardCorrection = static_cast<float>(0.5 * (1 - glm::abs(glm::sin(glm::radians(headAngle))))) * raster->getVoxelWidth();
            sideCorrection = static_cast<float>(0.5 * (1 - glm::cos(glm::radians(headAngle)))) * raster->getVoxelWidth();

            rotationAxis = glm::cross(glm::vec3(directionToVector(head.upDirection)), glm::vec3(directionToVector(head.forwardDirection)));
            headRotation = glm::rotate(glm::mat4(1.0f), headAngle, rotationAxis);
            headUp = glm::vec3(headRotation * glm::vec4(glm::vec3(directionToVector(head.upDirection)), 1.0f));
            headRotation = headRotation * rotatePart(&head);
            transformation.ModelViewMatrix = headRotation * transformation.ModelViewMatrix;
            
            headPosition = raster->getPosition(head.position);
            headPosition += forwardCorrection * glm::vec3(directionToVector(head.forwardDirection));
            headPosition += sideCorrection * head.upDirection;
            transformation.ModelViewMatrix = glm::translate(glm::mat4(1.0f), headPosition)
                * transformation.ModelViewMatrix;
            break;
        case DOWN:
            headAngle = 270 + 90 * (0.5f + static_cast<float>(elapsedTimeSinceLastMove / TIME_PER_MOVE));
            forwardCorrection = static_cast<float>(0.5 * (1 - glm::abs(glm::sin(glm::radians(headAngle))))) * raster->getVoxelWidth();
            sideCorrection = static_cast<float>(0.5 * (1 - glm::cos(glm::radians(headAngle)))) * raster->getVoxelWidth();

            rotationAxis = glm::cross(glm::vec3(directionToVector(head.upDirection)), glm::vec3(directionToVector(head.forwardDirection)));
            headRotation = glm::rotate(glm::mat4(1.0f), headAngle, rotationAxis);
            headUp = glm::vec3(headRotation * glm::vec4(glm::vec3(directionToVector(head.upDirection)), 1.0f));
            headRotation = headRotation * rotatePart(&head);
            transformation.ModelViewMatrix = headRotation * transformation.ModelViewMatrix;

            headPosition = raster->getPosition(head.position);
            headPosition += forwardCorrection * glm::vec3(directionToVector(head.forwardDirection));
            headPosition += sideCorrection * rotationAxis;
            transformation.ModelViewMatrix = glm::translate(glm::mat4(1.0f), headPosition)
                * transformation.ModelViewMatrix;
            break;
        default:
            break;
        }
    } else {
        switch (currentRelativeDirection) {
        case FORWARD:
            transformation.ModelViewMatrix = rotatePart(&head) * transformation.ModelViewMatrix;

            headPosition = raster->getPosition(head.position);
            headPosition += raster->getVoxelWidth() * static_cast<float>(elapsedTimeSinceLastMove / TIME_PER_MOVE) 
                * glm::vec3(directionToVector(head.forwardDirection));
            transformation.ModelViewMatrix = glm::translate(glm::mat4(1.0f), headPosition)
                * transformation.ModelViewMatrix;

            rotationAxis = headUp;
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

            rotationAxis = headUp;
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

            rotationAxis = headUp;
            break;
        case UP:
            headAngle = 360 - 90 * (-0.5f + static_cast<float>(elapsedTimeSinceLastMove / TIME_PER_MOVE));
            forwardCorrection = static_cast<float>((0.5 * (1 + glm::abs(glm::sin(glm::radians(headAngle))))) * raster->getVoxelWidth());
            sideCorrection = static_cast<float>(0.5 * (1 - glm::cos(glm::radians(headAngle))) * raster->getVoxelWidth());

            rotationAxis = glm::cross(glm::vec3(directionToVector(head.upDirection)), glm::vec3(directionToVector(head.forwardDirection)));
            headRotation = glm::rotate(glm::mat4(1.0f), headAngle, rotationAxis);
            headUp = glm::vec3(headRotation * glm::vec4(glm::vec3(directionToVector(head.upDirection)), 1.0f));
            headRotation = headRotation * rotatePart(&head);
            transformation.ModelViewMatrix = headRotation * transformation.ModelViewMatrix;

            headPosition = raster->getPosition(head.position);
            headPosition += forwardCorrection * glm::vec3(directionToVector(head.forwardDirection));
            headPosition += sideCorrection * glm::cross(glm::vec3(directionToVector(head.forwardDirection)), headUp);
            transformation.ModelViewMatrix = glm::translate(glm::mat4(1.0f), headPosition)
                * transformation.ModelViewMatrix;
            break;
        case DOWN:
            headAngle = 90 * (-0.5f + static_cast<float>(elapsedTimeSinceLastMove / TIME_PER_MOVE));
            forwardCorrection = static_cast<float>((0.5 * (1 + glm::sin(glm::radians(headAngle)))) * raster->getVoxelWidth());
            sideCorrection = static_cast<float>(0.5 * (1 - glm::cos(glm::radians(headAngle))) * raster->getVoxelWidth());

            rotationAxis = glm::cross(glm::vec3(directionToVector(head.upDirection)), glm::vec3(directionToVector(head.forwardDirection)));
            headRotation = glm::rotate(glm::mat4(1.0f), headAngle, rotationAxis);
            headUp = glm::vec3(headRotation * glm::vec4(glm::vec3(directionToVector(head.upDirection)), 1.0f));
            headRotation = headRotation * rotatePart(&head);
            transformation.ModelViewMatrix = headRotation * transformation.ModelViewMatrix;

            headPosition = raster->getPosition(head.position);
            headPosition += forwardCorrection * glm::vec3(directionToVector(head.forwardDirection));
            headPosition += sideCorrection * rotationAxis;
            transformation.ModelViewMatrix = glm::translate(glm::mat4(1.0f), headPosition)
                * transformation.ModelViewMatrix;
            break;
        default:
            break;
        }
    }
    
    headForward = glm::vec3(glm::rotate(glm::mat4(1.0f), headAngle, rotationAxis) 
                * glm::vec4(glm::vec3(directionToVector(head.forwardDirection)), 1.0f));
    createViewMatrix();

    // Head zeichnen
    drawMesh(transformation, projectionMatrix, HEAD);

    // ersten Teil des Körpers direkt hinter Head zeichnen
    transformation.ModelViewMatrix = glm::translate(glm::mat4(1.0f), -raster->getVoxelWidth() * headForward)
        * transformation.ModelViewMatrix;
    drawMesh(transformation, projectionMatrix, BODY);

    // Head runternehmen, da der schon gerendert wurde
    parts.pop_front();

    // ersten Teil des Körpers runternehmen, da der auch schon gerendert wurde
    SnakePart firstBody = parts.front();
    parts.pop_front();

    // die übrigen Teile der Schlange rendern
    for (SnakePart part: parts){
        transformation.ModelViewMatrix = modelMatrix;
        transformation.ModelViewMatrix = rotatePart(&part) * transformation.ModelViewMatrix;
        transformation.ModelViewMatrix = glm::translate(glm::mat4(1.0f), raster->getPosition(part.position))
            * transformation.ModelViewMatrix;

        drawMesh(transformation, projectionMatrix, part.type);
    }

    // erstes Körper-Element und Head wieder hinzufügen
    parts.push_front(firstBody);
    parts.push_front(head);

    
    //// draw mouse
    //transformation.ModelViewMatrix = modelMatrix;
    //transformation.ModelViewMatrix = glm::translate(glm::mat4(1.0f), raster->getPosition(mouse.position))
    //    * glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f))
    //    * transformation.ModelViewMatrix;

    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(mouseTextureTarget, mouseTextureID);
    //glUniform1i(textureLocation, 0);

    drawMesh(transformation, projectionMatrix, 4);

    // Betreten des nächsten Voxels
    if (!hasEnteredNextVoxel && elapsedTimeSinceLastMove > 0.5f * TIME_PER_MOVE) {
        previousRelativeDirection = currentRelativeDirection;
        currentRelativeDirection = newRelativeDirection;
        newRelativeDirection = FORWARD;
        hasEnteredNextVoxel = true;
        headAngle = 0.0f;
    // Körper der Schlange weiterbewegen
    } else if (elapsedTimeSinceLastMove > TIME_PER_MOVE) {
        move();
        elapsedTimeSinceLastMove = 0.0f;
        hasEnteredNextVoxel = false;
        headAngle = 0.0f;
    }
}

void Snake::move() {
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
    
    Direction newForwardDirection = relativeDirectionToAbsoluteDirection(&oldHead, currentRelativeDirection);
    SnakePart newHead(HEAD, newPosition, newForwardDirection, invertDirection(newForwardDirection), oldHead.upDirection);

    if (currentRelativeDirection == UP) {
        newHead.upDirection = invertDirection(oldHead.forwardDirection);
        headUp = directionToVector(invertDirection(newHead.upDirection));
    } else if (currentRelativeDirection == DOWN) {
        newHead.upDirection = oldHead.forwardDirection;
        headUp = directionToVector(newHead.upDirection);
    }
    parts.push_front(newHead);

    parts.pop_back();
    parts.back().type = TAIL;

    headPosition = raster->getPosition(parts.front().position);
}

void Snake::navigate(RelativeDirection newDirection) {
    newRelativeDirection = newDirection;
}

void Snake::createViewMatrix() {
    SnakePart head = parts.front();
    
    glm::vec3 eye = headPosition;
    eye += raster->getVoxelWidth() / 2.0f * headUp;
    eye += (-1.1f * raster->getVoxelWidth()) * headForward;
    
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
    viscg::LoadObject(meshes[0], "objects/snakehead.obj");
    viscg::LoadObject(meshes[1], "objects/snakebody.obj");
    viscg::LoadObject(meshes[2], "objects/snakecurve.obj");   
    viscg::LoadObject(meshes[3], "objects/snaketail.obj");
    //viscg::LoadObject(meshes[4], "objects/mouse.obj");
}

glm::mat4 Snake::rotatePart(SnakePart* part) {
    glm::mat4 upRotation = glm::mat4(1.0f);
    glm::mat4 forwardRotation = glm::mat4(1.0f);
    glm::mat4 backwardRotation = glm::mat4(1.0f);


    if (part->type != CORNER) {
        switch (part->upDirection) {
        case PosX:
            if (part->forwardDirection == PosZ)
                upRotation = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 0.0f, 1.0f));
            else
                upRotation = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 0.0f, 1.0f));
            break;
        case NegX:
            upRotation = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 0.0f, 1.0f));
            break;
        case PosY:
            if (part->forwardDirection == PosZ)
                upRotation = glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));
            break;
        case NegY:
            if (part->forwardDirection != PosZ)
                upRotation = glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));
            break;
        case PosZ:
            switch (part->forwardDirection) {
            case PosX:
                upRotation = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 0.0f, 1.0f));
                break;
            case NegX:
                upRotation = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 0.0f, 1.0f));
                break;
            case NegY:
                upRotation = glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));
                break;
            default:
                break;
            }
            break;
        case NegZ:
            switch (part->forwardDirection) {
            case PosX:
                upRotation = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 0.0f, 1.0f));
                break;
            case NegX:
                upRotation = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 0.0f, 1.0f));
                break;
            case PosY:
                upRotation = glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));
                break;
            default:
                break;
            }
            break;
        default:
            break;
        } 
    }

    switch (part->forwardDirection) {
    case PosX:
        forwardRotation = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        break;
    case NegX:
        forwardRotation = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        break;
    case PosY:
        forwardRotation = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        break;
    case NegY:
        forwardRotation = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        break;
    case PosZ:
        forwardRotation = glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        break;
    default:
        break;
    }

    if (part->type == CORNER) {
        switch (part->forwardDirection){
        case PosX:
            switch (part->backwardDirection) {
            case PosY:
                backwardRotation = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
                break;
            case NegY:
                backwardRotation = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
                break;
            case NegZ:
                backwardRotation = glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
                break;
            default:
                break;
            }
            break;
        case NegX:
            switch (part->backwardDirection) {
            case PosY:
                backwardRotation = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
                break;
            case NegY:
                backwardRotation = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
                break;
            case PosZ:
                backwardRotation = glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
                break; 
            default:
                break;
            }
            break;
        case PosY:
            switch (part->backwardDirection) {
            case NegX:
                backwardRotation = glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
                break;
            case PosZ:
                backwardRotation = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
                break;
            case NegZ:
                backwardRotation = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
                break;
            default:
                break;
            }
            break;
        case NegY:
            switch (part->backwardDirection) {
            case NegX:
                backwardRotation = glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
                break;
            case PosZ:
                backwardRotation = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
                break;
            case NegZ:
                backwardRotation = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
                break;
            default:
                break;
            }
            break;
        case PosZ:
            switch (part->backwardDirection) {
            case NegX:
                backwardRotation = glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));
                break;
            case PosY:
                backwardRotation = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 0.0f, 1.0f));
                break;
            case NegY:
                backwardRotation = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 0.0f, 1.0f));
                break;
            default:
                break;
            }
            break;
        case NegZ:
            switch (part->backwardDirection) {
            case NegX:
                backwardRotation = glm::rotate(glm::mat4(1.0f), 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));
                break;
            case PosY:
                backwardRotation = glm::rotate(glm::mat4(1.0f), 90.0f, glm::vec3(0.0f, 0.0f, 1.0f));
                break;
            case NegY:
                backwardRotation = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(0.0f, 0.0f, 1.0f));
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
    return backwardRotation * forwardRotation * upRotation;
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

Direction Snake::relativeDirectionToAbsoluteDirection(SnakePart* part, RelativeDirection relDir) {
    switch (relDir) {
    case FORWARD:
        return part->forwardDirection;
    case LEFT:
        switch (part->forwardDirection) {
        case PosX:
            switch (part->upDirection) {
            case PosY:
                return NegZ;
            case NegY:
                return PosZ;
            case PosZ:
                return PosY;
            case NegZ:
                return NegY;
            default:
                return part->forwardDirection;
            }
        case NegX:
            switch (part->upDirection) {
            case PosY:
                return PosZ;
            case NegY:
                return NegZ;
            case PosZ:
                return NegY;
            case NegZ:
                return PosY;
            default:
                return part->forwardDirection;
            }
        case PosY:
            switch (part->upDirection) {
            case PosX:
                return PosZ;
            case NegX:
                return NegZ;
            case PosZ:
                return NegX;
            case NegZ:
                return PosX;
            default:
                return part->forwardDirection;
            }
        case NegY:
            switch (part->upDirection) {
            case PosX:
                return NegZ;
            case NegX:
                return PosZ;
            case PosZ:
                return PosX;
            case NegZ:
                return NegX;
            default:
                return part->forwardDirection;
            }
        case PosZ:
            switch (part->upDirection) {
            case PosX:
                return PosY;
            case NegX:
                return NegY;
            case PosY:
                return PosX;
            case NegY:
                return NegX;
            default:
                return part->forwardDirection;
            }
        case NegZ:
            switch (part->upDirection) {
            case PosX:
                return PosY;
            case NegX:
                return NegY;
            case PosY:
                return NegX;
            case NegY:
                return PosX;
            default:
                return part->forwardDirection;
            }
        default:
            return part->forwardDirection;
        }
        return part->forwardDirection;
    case RIGHT:
        switch (part->forwardDirection) {
        case PosX:
            switch (part->upDirection) {
            case PosY:
                return PosZ;
            case NegY:
                return NegZ;
            case PosZ:
                return NegY;
            case NegZ:
                return PosY;
            default:
                return part->forwardDirection;
            }
        case NegX:
            switch (part->upDirection) {
            case PosY:
                return NegZ;
            case NegY:
                return PosZ;
            case PosZ:
                return PosY;
            case NegZ:
                return NegY;
            default:
                return part->forwardDirection;
            }
        case PosY:
            switch (part->upDirection) {
            case PosX:
                return NegZ;
            case NegX:
                return PosZ;
            case PosZ:
                return PosX;
            case NegZ:
                return NegX;
            default:
                return part->forwardDirection;
            }
        case NegY:
            switch (part->upDirection) {
            case PosX:
                return PosZ;
            case NegX:
                return NegZ;
            case PosZ:
                return NegX;
            case NegZ:
                return PosX;
            default:
                return part->forwardDirection;
            }
        case PosZ:
            switch (part->upDirection) {
            case PosX:
                return PosY;
            case NegX:
                return NegY;
            case PosY:
                return NegX;
            case NegY:
                return PosX;
            default:
                return part->forwardDirection;
            }
        case NegZ:
            switch (part->upDirection) {
            case PosX:
                return NegY;
            case NegX:
                return PosY;
            case PosY:
                return PosX;
            case NegY:
                return NegX;
            default:
                return part->forwardDirection;
            }
        default:
            return part->forwardDirection;
        }
        return part->forwardDirection;
    case UP:
        return part->upDirection;
    case DOWN:
        return invertDirection(part->upDirection);
    default:
        return part->forwardDirection;
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