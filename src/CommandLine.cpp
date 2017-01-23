#include "CommandLine.hpp"
#include "config.hpp"
#include "util.hpp"
#include <iostream>

dmp::CommandLine::CommandLine(int argc, char ** argv)
{
  std::string name;

  if (argc < 2)
    {
      ifDebug(std::cerr << "using default asset files: wasp" << std::endl);
      name = "wasp";
    }
  else name = argv[1];

  skinPath = std::string(modelDir) + "/" + name + ".skin";
  skelPath = std::string(modelDir) + "/" + name + ".skel";
  morphPath = std::string(modelDir) + "/" + name + ".morph";
}
