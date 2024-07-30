#include "simulation_old.h"

int main()
{
    if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
    }

    HopfSimulation sim("Hopf Simulation", WIDTH,HEIGHT);

    glfwTerminate();
    return 0;
}
