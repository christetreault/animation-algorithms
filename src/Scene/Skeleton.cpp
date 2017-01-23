#include "Skeleton.hpp"
#include <iostream>
#include <fstream>
#include <boost/tokenizer.hpp>
#include <limits>
#include <functional>
#include "Object.hpp"
#include "../util.hpp"

static void parseFloatTriple(boost::tokenizer<boost::char_separator<char>>::iterator & iter,
                             boost::tokenizer<boost::char_separator<char>>::iterator & end,
                             float & x, float & y, float & z,
                             std::string prefix)
{
  ++iter;
  x = stof(*iter);
  ++iter;
  y = stof(*iter);
  ++iter;
  z = stof(*iter);
  ++iter;
  expect("Parse error: garbage trailing " + prefix,
         iter == end || (*iter)[0] == '#');
}

static void parseFloatMinMax(boost::tokenizer<boost::char_separator<char>>::iterator & iter,
                             boost::tokenizer<boost::char_separator<char>>::iterator & end,
                             float & min, float & max,
                             std::string prefix)
{
  ++iter;
  min = stof(*iter);
  ++iter;
  max = stof(*iter);
  ++iter;
  expect("Parse error: garbage trailing " + prefix,
         iter == end || (*iter)[0] == '#');
}

std::function<bool(glm::mat4 &, float)> dmp::Skeleton::makeXformFn(dmp::Balljoint * bj)
{
  expect("balljoint pointer not null", bj);
  return [=](glm::mat4 & M, float deltaT)
    {
      if (!bj->rotateDirty) return false;

      float x = glm::clamp(bj->posex, bj->rotxmin, bj->rotxmax);
      float y = glm::clamp(bj->posey, bj->rotymin, bj->rotymax);
      float z = glm::clamp(bj->posez, bj->rotzmin, bj->rotzmax);

      ifDebug(std::cerr << "pose <x, y, z> = <" << x << ", " << y << ", " << z << ">" << std::endl);
      auto rotx = glm::rotate(glm::mat4(),
                              x,
                              glm::vec3(1.0f, 0.0f, 0.0f));
      auto roty = glm::rotate(glm::mat4(),
                              y,
                              glm::vec3(0.0f, 1.0f, 0.0f));
      auto rotz = glm::rotate(glm::mat4(),
                              z,
                              glm::vec3(0.0f, 0.0f, 1.0f));
      M = rotz * roty * rotx;
      bj->rotateDirty = false;
      return true;
    };
}

std::unique_ptr<dmp::Balljoint> dmp::Skeleton::parse(std::string currName,
                                                     std::string currCtor,
                                                     std::vector<std::string>::iterator & begin,
                                                     std::vector<std::string>::iterator & end)
{
  using namespace boost;

  auto retval = std::make_unique<Balljoint>();
  retval->name = currName;

  char_separator<char> sep(" \t\r\n");

  if (currCtor == "balljoint")
    {

      while (begin != end)
        {
          tokenizer<char_separator<char>> tokens(*begin, sep);
          ++begin;
          auto iter = tokens.begin();
          auto tend = tokens.end();

          if (iter == tokens.end()) continue;
          else if ((*iter)[0] == '#') continue;
          else if (*iter == "}") break;
          else if (*iter == "offset")
            {
              parseFloatTriple(iter, tend, retval->offsetx,
                               retval->offsety, retval->offsetz, "offset");
            }
          else if (*iter == "boxmin")
            {
              parseFloatTriple(iter, tend, retval->boxminx,
                               retval->boxminy, retval->boxminz, "boxmin");
            }
          else if (*iter == "boxmax")
            {
              parseFloatTriple(iter, tend, retval->boxmaxx,
                               retval->boxmaxy, retval->boxmaxz, "boxmax");
            }
          else if (*iter == "rotxlimit")
            {
              parseFloatMinMax(iter, tend, retval->rotxmin,
                               retval->rotxmax, "rotxlimit");
            }
          else if (*iter == "rotylimit")
            {
              parseFloatMinMax(iter, tend, retval->rotymin,
                               retval->rotymax, "rotylimit");
            }
          else if (*iter == "rotzlimit")
            {
              parseFloatMinMax(iter, tend, retval->rotzmin,
                               retval->rotzmax, "rotzlimit");
            }
          else if (*iter == "pose")
            {
              parseFloatTriple(iter, tend, retval->posex,
                               retval->posey,
                               retval->posez, "pose");
            }
          else if (*iter == "balljoint")
            {

              ++iter;
              auto name = *iter;
              ++iter;
              retval->children.emplace_back(parse(name, "balljoint",
                                                  begin, end));
            }
        }
      return retval;
    }
  impossible("mismatched { braces } in skel file!");
}

static void printSkel(dmp::Balljoint * bj, std::string padding)
{
  auto pad = padding + "  ";
  std::cerr << padding << "Balljoint " << bj->name << " {" << std::endl;
  std::cerr << pad << "offset = " << bj->offsetx
            << " " << bj->offsety << " " << bj->offsetz << std::endl;
  std::cerr << pad << "boxmin = " << bj->boxminx
            << " " << bj->boxminy << " " << bj->boxminz << std::endl;
  std::cerr << pad << "boxmax = " << bj->boxmaxx
            << " " << bj->boxmaxy << " " << bj->boxmaxz << std::endl;
  std::cerr << pad << "pose = " << bj->posex
            << " " << bj->posey << " " << bj->posez << std::endl;
  std::cerr << pad << "rotmin = " << bj->rotxmin
            << " " << bj->rotymin << " " << bj->rotzmin << std::endl;
  std::cerr << pad << "rotmax = " << bj->rotxmax
            << " " << bj->rotymax << " " << bj->rotzmax << std::endl;
  for (auto & curr : bj->children)
    {
      printSkel(curr.get(), pad);
    }
  std::cerr << padding << "}" << std::endl;
}

void dmp::Skeleton::initSkeleton(std::string skelPath)
{
  using namespace std;
  using namespace boost;

  ifstream file(skelPath, ios::in);
  string line;
  std::vector<std::string> lines;

  expect("opened skelPath", file.is_open());

  while (getline(file, line))
    {
      lines.push_back(line);
    }

  file.close();

  auto iter = lines.begin();

  line = *iter;
  ++iter;

  char_separator<char> sep(" \t\r\n");
  tokenizer<char_separator<char>> tokens(line, sep);
  auto tokIter = tokens.begin();

  if (*tokIter == "balljoint")
    {
      ++tokIter;
      auto lend = lines.end();
      auto name = *tokIter;
      mRoot = parse(name, "balljoint",
                    iter, lend);
    }
  else
    {
      impossible("parse error: invalid root");
    }
  ifDebug(printSkel(mRoot.get(), ""));
}

void dmp::Skeleton::insertInSceneImpl(dmp::Balljoint * bj,
                                      dmp::Branch * graph,
                                      std::vector<dmp::Object *> & objs,
                                      size_t matIdx,
                                      size_t texIdx)
{
  expect("bj not null", bj);
  expect("graph not null", graph);
  glm::vec4 min = {bj->boxminx, bj->boxminy, bj->boxminz, 1.0f};
  glm::vec4 max = {bj->boxmaxx, bj->boxmaxy, bj->boxmaxz, 1.0f};
  glm::vec4 off = {bj->offsetx, bj->offsety, bj->offsetz, 0.0f};
  Object obj(Cube, min, max, matIdx, texIdx);

  auto offset = graph->transform(glm::translate(glm::mat4(), glm::vec3(off)));
  auto xform = offset->transform(Skeleton::makeXformFn(bj));

  auto branch = xform->branch();
  objs.push_back(branch->insert(obj));

  for (auto & curr : bj->children)
    {
      expect("child not null", curr);
      Skeleton::insertInSceneImpl(curr.get(),
                                  branch,
                                  objs,
                                  matIdx,
                                  texIdx);
    }
}

void dmp::Skeleton::insertInScene(Branch * graph,
                                  std::vector<Object *> & objs,
                                  size_t matIdx,
                                  size_t texIdx)
{
  expect("root not null", mRoot);
  Skeleton::insertInSceneImpl(mRoot.get(),
                              graph,
                              objs,
                              matIdx,
                              texIdx);
}
