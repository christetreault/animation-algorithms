#include "CommandLine.hpp"
#include "config.hpp"
#include "util.hpp"
#include "Scene/Model/parsing.hpp"
#include <iostream>

static std::string fullyQualify(const std::string & prefix,
                                const std::string & s,
                                const std::string & suffix)
{
  return prefix + s + suffix;
}

dmp::CommandLine::CommandLine(int argc, char ** argv)
{
  using namespace boost;
  std::string prefix = std::string(modelDir) + "/";

  if (argc < 2)
    {
      ifDebug(std::cerr << "using default asset files: wasp and waspwalk" << std::endl);
      skinPath = fullyQualify(prefix, "wasp", ".skin");
      skelPath = fullyQualify(prefix, "wasp", ".skel");
      animPath = fullyQualify(prefix, "waspwalk", ".anim");
      morphPaths = {};
      return;
    }

  std::string dataStr = "";
  for (int i = 1; i < argc; ++i)
    {
      dataStr += " ";
      dataStr += argv[i];
    }

  auto sep = whitespaceSeparator;
  tokenizer<char_separator<char>> tokens(dataStr, sep);

  auto iter = tokens.begin();
  auto end = tokens.end();

  std::string name;

  parseString(name, iter, end);

  int morphCount = 0;
  std::string animSuffix = "";

  auto morphCountFn = [&morphCount](TokenIterator & beg, TokenIterator & end)
    {
      parseInt(morphCount, beg, end);
    };
  auto animSuffixFn = [&animSuffix](TokenIterator & beg, TokenIterator & end)
    {
      parseString(animSuffix, beg, end);
    };

  someParse({morphCountFn, animSuffixFn}, iter, end);

  skinPath = fullyQualify(prefix, name, ".skin");
  skelPath = fullyQualify(prefix, name, ".skel");

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

  if (animSuffix == "") animPath = "";
  else
    {
      animPath = fullyQualify(prefix, name + animSuffix, ".anim");
    }

  std::cerr << "Got command line:" << std::endl
            << "skin = " << skinPath << std::endl
            << "skel = " << skelPath << std::endl
            << "|morphs| = " << morphPaths.size() << std::endl
            << "anim = " << animPath << std::endl;
}
