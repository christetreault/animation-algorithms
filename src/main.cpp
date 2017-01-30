#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Program.hpp"
#include "util.hpp"
#include "CommandLine.hpp"

#include <glm/glm.hpp>

static void libsInit()
{
  expect("GLFW init failed!",
         glfwInit());
}

static void libsFinalize()
{
  glfwTerminate();
}

int main(int argc, char ** argv)
{
  using namespace dmp;

  ifDebug(std::cerr << "Built in debug mode..." << std::endl);
  ifRelease(std::cerr << "Built in release mode..." << std::endl);

  CommandLine cmd(argc, argv);

  int exitCode = EXIT_SUCCESS;

  try
    {
      libsInit();

      int width = 1280;
      int height = 720;
      Program p(width, height, "Petting a cat's tummy is dangerous,"
                "but nothing ventured nothing gained", cmd);

      exitCode = p.run();
    }
  catch (dmp::InvariantViolation & e)
    {
      std::cerr << "Invariant Violation!"
                << std::endl
                << "--------------------------------------------------------------------------------"
                << std::endl
                << e.what()
                << std::endl
                << "--------------------------------------------------------------------------------"
                << std::endl;

      exitCode = EXIT_FAILURE;
    }

  libsFinalize();
  return exitCode;
}
