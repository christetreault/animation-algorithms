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

  if (argc > 2)
    {
      int morphCount = stoi(std::string(argv[2]));
      morphPaths.reserve((size_t) morphCount);
      for (size_t i = 0; i < (size_t) morphCount; ++i)
        {
          morphPaths.push_back(std::string(modelDir)
                               + "/"
                               + name
                               + std::to_string(i + 1)
                               + ".morph");
        }
    }
  else
    {
      morphPaths = {};
    }
}
