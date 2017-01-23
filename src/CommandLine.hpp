#ifndef DMP_COMMANDLINE_HPP
#define DMP_COMMANDLINE_HPP

#include <string>

namespace dmp
{
  struct CommandLine
  {
  public:
    std::string skinPath;
    std::string skelPath;
    std::string morphPath;

    CommandLine(int argc, char ** argv);
  };
}

#endif
