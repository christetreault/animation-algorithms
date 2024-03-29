#ifndef DMP_COMMANDLINE_HPP
#define DMP_COMMANDLINE_HPP

#include <string>
#include <vector>

namespace dmp
{
  struct CommandLine
  {
  public:
    std::string skinPath;
    std::string skelPath;
    std::vector<std::string> morphPaths;
    std::string animPath;

    CommandLine(int argc, char ** argv);

    bool hasSkin() const {return skinPath != "";}
    bool hasSkel() const {return skelPath != "";}
    bool hasMorphs() const {return morphPaths.size() > 0;}
    bool hasAnim() const {return animPath != "";}
  };
}

#endif
