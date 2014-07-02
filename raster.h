#ifndef RASTER_H
#define RASTER_H

#include "framework.h"
#include <set>

struct ivec3_compare {
    bool operator() (const glm::ivec3 lhs, const glm::ivec3 rhs) const {
        if (lhs.x < rhs.x)
            return true;
        if (lhs.x == rhs.x && lhs.y < rhs.y)
            return true;
        if (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z < rhs.z)
            return true;
        return false;
    }
};

class Raster {
public:
    Raster(unsigned int width, unsigned int voxelsPerLine);

    void draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
    bool isOutOfBounds(glm::ivec3 voxel);
    bool activateVoxel(glm::ivec3 voxel);
    bool activateVoxel(glm::vec3 position);
    bool deactivateVoxel(glm::ivec3 voxel);
    bool deactivateVoxel(glm::vec3 position);

    glm::vec3 getPosition(glm::ivec3 voxel);
    float getVoxelWidth();
private:
    std::vector<glm::vec3> raster;
    std::set<glm::ivec3, ivec3_compare> activatedVoxels;
    
    std::vector<unsigned int> gridIndices;
    std::vector<unsigned int> wallIndices;
    std::vector<unsigned int> voxelIndices;

    glm::mat4 modelMatrix;

    bool hasChanged;
    GLuint programID;
    GLint MVPLocation;
    GLint colorLocation;
    GLuint rasterVBOID;
    GLuint gridIBOID;
    GLuint wallIBOID;
    GLuint voxelIBOID;
    unsigned int width;
    unsigned int voxelsPerLine;
    float voxelWidth;

    glm::ivec3 calculateVoxel(glm::vec3 position);
    std::vector<unsigned int> createVoxelIndices();
};

#endif