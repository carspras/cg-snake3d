//Snake 3D

#include "framework.h"
#include "raster.h"
#include "snake.h"

#include <Windows.h>

int wd = 640;
int ht = 640;
    
const int RASTER_WIDTH = 2;
const int VOXELS_PER_LINE = 9;

void gameOver(Snake* snake) {
    std::cout << std::endl;
    
    if (snake->gameOver() == RAN_INTO_WALL) {
        std::cout << "You ran into a wall!";
    } else if (snake->gameOver() == BIT_ITSELF) {
        std::cout << "You bit yourself!";
    }
    
    std::cout << " GAME OVER!" << std::endl;
    std::cout << "You have eaten " << snake->getScore() << " mice!" << std::endl;
    std::cout << "You survived for " << snake->getMoves() << " moves!" << std::endl;
    std::cout << "Do you want to try again [y/n]? ";
    std::string input;
    std::cin >> input;

    if (input == "y") {
        snake->reset();
        Sleep(5000);
        return;
    } else if (input == "n") {
        exit(0);
    }
}


int main(int argc, char** argv)
{
    if (!viscg::OpenWindow(wd, ht, "Snake3D", 4, 30, 30, true))
    {
        std::cerr << "cannot create a window" << std::endl;
        exit(-1);
    }

    Raster raster(RASTER_WIDTH, VOXELS_PER_LINE);
    Snake snake(&raster);
    
    glm::mat4 projectionMatrix = glm::perspective(45.0f, static_cast<float>(wd)/static_cast<float>(ht), 0.001f, 100.0f);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    double previousTime = viscg::ElapsedTime();

    std::cout << std::endl << "Press 'Enter' to start the game!";
    std::cin.get();
    Sleep(5000);
    
    // main rendering loop
    while(viscg::ProcessAllMesages()) { 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (viscg::KeyPressed('w'))
            snake.navigate(UP);
        if (viscg::KeyPressed('s'))
            snake.navigate(DOWN);
        if (viscg::KeyPressed('a'))
            snake.navigate(LEFT);
        if (viscg::KeyPressed('d'))
            snake.navigate(RIGHT);

        double elapsedTimeSinceLastFrame = viscg::ElapsedTime() - previousTime;
        previousTime = viscg::ElapsedTime();

        snake.draw(projectionMatrix, elapsedTimeSinceLastFrame);
        raster.draw(snake.getViewMatrix(), projectionMatrix);

        if (snake.gameOver() != NOT_OVER) {
            gameOver(&snake);
            previousTime = viscg::ElapsedTime();
        }

        viscg::SwapBuffers();
    }
    return 0;
}
