#include "Model.hpp"
#include "Graph.hpp"
#include <fstream>
#include "Model/Skeleton.hpp"

static bool fileExists(std::string path)
{
  std::ifstream f(path, std::ios::in);

  if (f)
    {
      f.close();
      return true;
    }
  else
    {
      return false;
    }
}

dmp::Model::Model(const CommandLine & c,
                  std::vector<Object *> & objs,
                  size_t matIdx,
                  size_t texIdx)
{
  bool skelExists = fileExists(c.skelPath);
  bool skinExists = fileExists(c.skinPath);

  expect("either skin or skel file exists", skelExists || skinExists);

  if (skinExists)
    {
      mSkin = std::make_unique<Skin>(c.skinPath);
      mSkin->insertInScene(objs, matIdx, texIdx);
    }
  if (skelExists)
    {
      mSkeleton = std::make_unique<Skeleton>(c.skelPath);

      std::vector<glm::mat4> invBs;
      if (skinExists) invBs = mSkin->askBindings();

      else invBs.resize(0);

      auto beg = invBs.begin();
      auto end = invBs.end();

      mSkeleton->makeBones(objs, matIdx, texIdx, beg, end);

      if (!skinExists) mSkeleton->show();
    }
}

void dmp::Model::update(float deltaT,
                        glm::mat4 M,
                        bool dirty)
{
   if (mSkeleton)
     {
       mSkeleton->update(deltaT, M, dirty);
     }

   if (mSkeleton && mSkin)
     {
       mSkin->update(deltaT, M, dirty);
       auto Ms = mSkeleton->getMs();
       mSkin->tellBindingMats(Ms);
     }
   else if (mSkeleton && !mSkin)
     {

     }

   else
     {
       todo("implement support for skin without skeleton");
     }
}
