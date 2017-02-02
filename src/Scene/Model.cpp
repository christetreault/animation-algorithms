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

  // Morphs

  if (c.morphPaths.size() == 0 || !skinExists) return;

  mMorphs.resize(1); // we want the null morph to be a location 0.
                     // We will overwrite this
  mMorphs.reserve(c.morphPaths.size() + 1);
  for (const auto & curr : c.morphPaths)
    {
      mMorphs.emplace_back(curr);
    }

  // TODO: this is a bit gross
  auto morphs = mMorphs.data();
  auto length = mMorphs.size();
  ++morphs;
  --length;
  mMorphs[0].initNullMorph(mSkin->askVerts(),
                           mSkin->askNormals(),
                           morphs,
                           length);
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

   if (mMorphs.size() > 0)
     {
       if (mMorphLerpInProgress)
         {
           mCurrT += deltaT;
           auto l = mCurrT / period;
           if (mCurrT >= period)
             {
               mMorphLerpInProgress = false;
               mSkin->applyMorph(mMorphs[mMorphNew]);
             }
           else if (l < 0.5f)
             { // transition old -> null
               mSkin->applyMorph(Morph(mMorphs[mMorphOld],
                                       mMorphs[0],
                                       l / 0.5f));
             }
           else
             { // transition null -> new
               mSkin->applyMorph(Morph(mMorphs[0],
                                       mMorphs[mMorphNew],
                                       (l - 0.5f) / 0.5f));
             }
         }
     }
}

void dmp::Model::applyMorph(size_t index, float time)
{
  expect("has skin", mSkin);
  expect("mas morphs", mMorphs.size() > 0);
  expect("index in range", mMorphs.size() > index);

  if (mMorphLerpInProgress)
    {
      ifDebug(std::cerr << "morph in progress, ignoring..." << std::endl);
      return;
    }
  else
    {
      mMorphLerpInProgress = true;
      mCurrT = 0.0f;
      mMorphOld = mMorphNew;
      mMorphNew = index;
    }
}
