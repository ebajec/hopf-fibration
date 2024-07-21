#include <iostream>
#include "simulation.h"

int main()
{
    if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
	}

    HopfSimulation sim("Hopf Simulation", WIDTH,HEIGHT);

    printf("-----------------------------\n");
    printf("Press ESC to toggle GUI access\n");
    printf("Use WASD and mouse to move\n");

    sim.waitForClose();
    glfwTerminate();
    return 0;
}
