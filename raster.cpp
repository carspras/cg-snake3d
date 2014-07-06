#include "raster.h"

Raster::Raster(unsigned int width, unsigned int voxelsPerLine) {
    this->width = width;
    this->voxelsPerLine = voxelsPerLine;
    voxelWidth = 1.0f * width / voxelsPerLine;

    programID = viscg::LoadSimpleProgram("shader/raster.vs.glsl", "shader/raster.fs.glsl");
    MVPLocation = glGetUniformLocation(programID, "MVP");
    colorLocation = glGetUniformLocation(programID, "color");

    GLuint bufferIDs[4];
    glGenBuffers(4, bufferIDs);
    
    rasterVBOID = bufferIDs[0];
    gridIBOID = bufferIDs[1];
    wallIBOID = bufferIDs[2];
    voxelIBOID = bufferIDs[3];
    hasChanged = true;

    modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, -1.0f, -1.0f));

    // Positionen der Eckpunkte jedes Voxels
    for (unsigned int i = 0; i < voxelsPerLine+1; i++) {
        for (unsigned int j = 0; j < voxelsPerLine+1; j++) {
            for (unsigned int k = 0; k < voxelsPerLine+1; k++) {
                raster.push_back(glm::vec3(k * voxelWidth, j * voxelWidth, i * voxelWidth));
            }
        }
    }

    // Positionen in den Buffer schreiben
    glBindBuffer(GL_ARRAY_BUFFER, rasterVBOID);
    glBufferData(GL_ARRAY_BUFFER, raster.size() * sizeof(glm::vec3), raster.data(), GL_STATIC_DRAW);

    // Indices für das Zeichnen des Grids
    // Linien in X-Richtung
    for (unsigned int i = 0; i < voxelsPerLine+1; i++) {
        for (unsigned int j = 0; j < voxelsPerLine+1; j++) {
            gridIndices.push_back(i * (voxelsPerLine+1)*(voxelsPerLine+1) + j * (voxelsPerLine+1));
            gridIndices.push_back(i * (voxelsPerLine+1)*(voxelsPerLine+1) + j * (voxelsPerLine+1) + voxelsPerLine);
        } 
    }

    // Linien in Y-Richtung
    for (unsigned int i = 0; i < voxelsPerLine+1; i++) {
        for (unsigned int j = 0; j < voxelsPerLine+1; j++) {
            gridIndices.push_back(i * (voxelsPerLine+1)*(voxelsPerLine+1) + j);
            gridIndices.push_back(i * (voxelsPerLine+1)*(voxelsPerLine+1) + (voxelsPerLine+1)*voxelsPerLine + j);
        }
    }

    // Linien in Z-Richtung
    for (unsigned int i = 0; i < voxelsPerLine+1; i++) {
        for (unsigned int j = 0; j < voxelsPerLine+1; j++) {
            gridIndices.push_back(i * (voxelsPerLine+1) + j);
            gridIndices.push_back(i * (voxelsPerLine+1) + (voxelsPerLine+1)*(voxelsPerLine+1)*voxelsPerLine + j);
        }
    }

    // Indices für das Gitter in den IndexBuffer schreiben
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridIBOID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, gridIndices.size() * sizeof(unsigned int), gridIndices.data(), GL_STATIC_DRAW);

    // back wall
    for (unsigned int i = 0; i < voxelsPerLine+1; i++) {
        wallIndices.push_back(i * (voxelsPerLine+1));
        wallIndices.push_back(i * (voxelsPerLine+1) + voxelsPerLine);
    }
    for (unsigned int i = 0; i < voxelsPerLine+1; i++) {
        wallIndices.push_back(i);
        wallIndices.push_back(i + (voxelsPerLine+1)*voxelsPerLine);
    }

    // front wall
    for (unsigned int i = 0; i < voxelsPerLine+1; i++) {
        wallIndices.push_back(i * (voxelsPerLine+1) + (voxelsPerLine+1)*(voxelsPerLine+1)*voxelsPerLine);
        wallIndices.push_back(i * (voxelsPerLine+1) + voxelsPerLine + (voxelsPerLine+1)*(voxelsPerLine+1)*voxelsPerLine);
    }
    for (unsigned int i = 0; i < voxelsPerLine+1; i++) {
        wallIndices.push_back(i + (voxelsPerLine+1)*(voxelsPerLine+1)*voxelsPerLine);
        wallIndices.push_back(i + (voxelsPerLine+1)*voxelsPerLine + (voxelsPerLine+1)*(voxelsPerLine+1)*voxelsPerLine);
    }

    // left wall
    for (unsigned int i = 0; i < voxelsPerLine+1; i++) {
        wallIndices.push_back(i * (voxelsPerLine+1)*(voxelsPerLine+1));
        wallIndices.push_back(i * (voxelsPerLine+1)*(voxelsPerLine+1) + (voxelsPerLine+1)*voxelsPerLine);
    }
    for (unsigned int i = 0; i < voxelsPerLine+1; i++) {
        wallIndices.push_back(i * (voxelsPerLine+1));
        wallIndices.push_back(i * (voxelsPerLine+1) + (voxelsPerLine+1)*(voxelsPerLine+1)*voxelsPerLine);
    }

    // right wall
    for (unsigned int i = 0; i < voxelsPerLine+1; i++) {
        wallIndices.push_back(i * (voxelsPerLine+1)*(voxelsPerLine+1) + voxelsPerLine);
        wallIndices.push_back(i * (voxelsPerLine+1)*(voxelsPerLine+1) + (voxelsPerLine+1)*voxelsPerLine + voxelsPerLine);
    }
    for (unsigned int i = 0; i < voxelsPerLine+1; i++) {
        wallIndices.push_back(i * (voxelsPerLine+1) + voxelsPerLine);
        wallIndices.push_back(i * (voxelsPerLine+1) + (voxelsPerLine+1)*(voxelsPerLine+1)*voxelsPerLine + voxelsPerLine);
    }
    
    // bottom
    for (unsigned int i = 0; i < voxelsPerLine+1; i++) {
        wallIndices.push_back(i * (voxelsPerLine+1)*(voxelsPerLine+1));
        wallIndices.push_back(i * (voxelsPerLine+1)*(voxelsPerLine+1) + voxelsPerLine);
    }
    for (unsigned int i = 0; i < voxelsPerLine+1; i++) {
        wallIndices.push_back(i);
        wallIndices.push_back(i + (voxelsPerLine+1)*(voxelsPerLine+1)*voxelsPerLine);
    }

    // top
    for (unsigned int i = 0; i < voxelsPerLine+1; i++) {
        wallIndices.push_back(i * (voxelsPerLine+1)*(voxelsPerLine+1) + (voxelsPerLine+1)*voxelsPerLine);
        wallIndices.push_back(i * (voxelsPerLine+1)*(voxelsPerLine+1) + voxelsPerLine + (voxelsPerLine+1)*voxelsPerLine);
    }
    for (unsigned int i = 0; i < voxelsPerLine+1; i++) {
        wallIndices.push_back(i + (voxelsPerLine+1)*voxelsPerLine);
        wallIndices.push_back(i + (voxelsPerLine+1)*(voxelsPerLine+1)*voxelsPerLine + (voxelsPerLine+1)*voxelsPerLine);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallIBOID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, wallIndices.size() * sizeof(unsigned int), wallIndices.data(), GL_STATIC_DRAW);
}

void Raster::draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {
    glUseProgram(programID);

    GLuint vaoID;
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);
    glEnableVertexAttribArray(0);

    glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;
    glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, &MVP[0][0]);

    // Gitter zeichnen    
    glBindBuffer(GL_ARRAY_BUFFER, rasterVBOID);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallIBOID);
    glUniform4f(colorLocation, 0.5f, 0.0f, 0.0f, 1.0f);
    glDrawElements(GL_LINES, wallIndices.size(), GL_UNSIGNED_INT, 0);

    /*
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridIBOID);
    glUniform4f(colorLocation, 1.0f, 1.0f, 1.0f, 1.0f);
    glDrawElements(GL_LINES, gridIndices.size(), GL_UNSIGNED_INT, 0);
    */

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, voxelIBOID);
    // Die Indices für die Voxel müssen nur neu geladen werden, wenn sich an activatedVoxels etwas geändert hat
    // seit dem letzten Zeichnen
    if (hasChanged) {
        voxelIndices = createVoxelIndices();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, voxelIndices.size() * sizeof(unsigned int), voxelIndices.data(), GL_STATIC_DRAW);
        hasChanged = false;
    }
    // aktivierte Voxel zeichnen
    glUniform4f(colorLocation, 0.0f, 0.5f, 0.5f, 0.0f);
    glDrawElements(GL_TRIANGLES, voxelIndices.size(), GL_UNSIGNED_INT, 0);
}

bool Raster::isOutOfBounds(glm::ivec3 voxel) {
    if (voxel.x < 0 || voxel.x >= static_cast<int>(voxelsPerLine) ||
        voxel.y < 0 || voxel.y >= static_cast<int>(voxelsPerLine) ||
        voxel.z < 0 || voxel.z >= static_cast<int>(voxelsPerLine))
        return true;
    return false;
}

bool Raster::activateVoxel(glm::ivec3 voxel) {
    // Prüfen, ob der übergebene Voxel innerhalb des Rasters liegt
    if (voxel.x > static_cast<int>(voxelsPerLine) || voxel.y > static_cast<int>(voxelsPerLine) ||
        voxel.z > static_cast<int>(voxelsPerLine))
        return false;

    activatedVoxels.insert(voxel);
    hasChanged = true;
    return true;
}

bool Raster::activateVoxel(glm::vec3 position) {
    return activateVoxel(calculateVoxel(position));
}

bool Raster::deactivateVoxel(glm::ivec3 voxel) {
    hasChanged = true;
    if (activatedVoxels.erase(voxel) > 0)
        return true;
    else
        return false;
}

bool Raster::deactivateVoxel(glm::vec3 position) {
    return deactivateVoxel(calculateVoxel(position));
}

glm::vec3 Raster::getPosition(glm::ivec3 voxel) {
    unsigned int index = voxel.z * (voxelsPerLine+1)*(voxelsPerLine+1) + voxel.y * (voxelsPerLine+1) + voxel.x;

    glm::vec3 position(raster[index].x + voxelWidth / 2, raster[index].y + voxelWidth / 2, raster[index].z + voxelWidth / 2);
    position = glm::vec3(modelMatrix * glm::vec4(position, 1.0f));

    return position;
}

float Raster::getVoxelWidth() {
    return voxelWidth;
}

int Raster::getVoxelsPerLine() {
	return voxelsPerLine;
}

glm::ivec3 Raster::calculateVoxel(glm::vec3 position) {
    unsigned int x = static_cast<int>(position.x / voxelWidth);
    unsigned int y = static_cast<int>(position.y / voxelWidth);
    unsigned int z = static_cast<int>(position.z / voxelWidth);

    return glm::ivec3(x,y,z);
}

std::vector<unsigned int> Raster::createVoxelIndices() {
    std::vector<unsigned int> voxelIndices;
    
    for each (glm::ivec3 voxel in activatedVoxels) {
        // Indices für die acht Ecken des Voxels ausrechnen
        unsigned int BLB = voxel.z * (voxelsPerLine+1)*(voxelsPerLine+1) + voxel.y * (voxelsPerLine+1) + voxel.x;
        unsigned int BRB = BLB + 1;
        unsigned int ULB = BLB + voxelsPerLine+1;
        unsigned int URB = ULB + 1;
        unsigned int BLF = BLB + (voxelsPerLine+1)*(voxelsPerLine+1);
        unsigned int BRF = BLF + 1;
        unsigned int ULF = BLF + voxelsPerLine+1;
        unsigned int URF = ULF + 1;
        
        // Indices für das linke obere Dreieck des Pixels
        // front side
        voxelIndices.push_back(BLF);
        voxelIndices.push_back(BRF);
        voxelIndices.push_back(ULF);
                
        voxelIndices.push_back(ULF);
        voxelIndices.push_back(BRF);
        voxelIndices.push_back(URF);
        
        // back side
        voxelIndices.push_back(BRB);
        voxelIndices.push_back(BLB);
        voxelIndices.push_back(URB);

        
        voxelIndices.push_back(URB);
        voxelIndices.push_back(BLB);
        voxelIndices.push_back(ULB);

        
        // left side
        voxelIndices.push_back(BLB);
        voxelIndices.push_back(BLF);
        voxelIndices.push_back(ULB);
        
        
        voxelIndices.push_back(ULB);
        voxelIndices.push_back(BLF);
        voxelIndices.push_back(ULF);

        // right side
        voxelIndices.push_back(BRF);
        voxelIndices.push_back(BRB);
        voxelIndices.push_back(URF);

        voxelIndices.push_back(URF);
        voxelIndices.push_back(BRB);
        voxelIndices.push_back(URB);

        // top side
        voxelIndices.push_back(ULF);
        voxelIndices.push_back(URF);
        voxelIndices.push_back(ULB);

        voxelIndices.push_back(ULB);
        voxelIndices.push_back(URF);
        voxelIndices.push_back(URB);
        
        // bottom side
        voxelIndices.push_back(BLB);
        voxelIndices.push_back(BRB);
        voxelIndices.push_back(BLF);

        voxelIndices.push_back(BLF);
        voxelIndices.push_back(BRB);
        voxelIndices.push_back(BRF);
    }
    return voxelIndices;
}