#include "Skin.hpp"

#include <iostream>
#include <fstream>
#include "../../util.hpp"
#include "../Object.hpp"
#include "../Graph.hpp"
#include "Morph.hpp"
#include "parsing.hpp"

#include <glm/gtx/string_cast.hpp>

using namespace boost;
typedef token_iterator_generator<char_separator<char>>::type TokenIterator;

static const char * tokPositions = "positions";
static const char * tokNormals = "normals";
static const char * tokSkinweights = "skinweights";
static const char * tokTriangles = "triangles";
static const char * tokBindings = "bindings";
static const char * tokMatrix = "matrix";
static const char * tokTexcoords = "texcoords";
static const char * tokMaterial = "material";
static const char * tokTexture = "texture";
static const char * tokOpenBrace = "{";

static void parsePositions(dmp::SkinData & data,
                           TokenIterator & iter,
                           TokenIterator & end)
{
  auto t = [](dmp::SkinData & data,
              TokenIterator & iter,
              TokenIterator &)
    {
      auto size = stoi(*iter);
      expect("size not negative", size >= 0);
      data.verts.reserve((size_t) size);
    };

  auto f = [](dmp::SkinData & data,
               TokenIterator & iter,
               TokenIterator & end)
    {
      float x, y, z;
      dmp::parseVec3(x, y, z, iter, end);
      data.verts.push_back({x, y, z});
    };

  dmp::parseField<dmp::SkinData>(data, iter, end, tokPositions, t, f);
}

static void parseNormals(dmp::SkinData & data,
                         TokenIterator & iter,
                         TokenIterator & end)
{
  auto t = [](dmp::SkinData & data,
              TokenIterator & iter,
              TokenIterator &)
    {
      auto size = stoi(*iter);
      expect("size not negative", size >= 0);
      data.normals.reserve((size_t) size);
    };

  auto f = [](dmp::SkinData & data,
               TokenIterator & iter,
               TokenIterator & end)
    {
      float x, y, z;
      dmp::parseVec3(x, y, z, iter, end);
      data.normals.push_back({x, y, z});
    };

  dmp::parseField<dmp::SkinData>(data, iter, end, tokNormals, t, f);
}

static void parseSkinWeights(dmp::SkinData & data,
                             TokenIterator & iter,
                             TokenIterator & end)
{
  auto t = [](dmp::SkinData & data,
              TokenIterator & iter,
              TokenIterator &)
    {
      auto size = stoi(*iter);
      expect("size not negative", size >= 0);
      data.weights.reserve((size_t) size);
    };

  auto f = [](dmp::SkinData & data,
               TokenIterator & iter,
               TokenIterator & end)
    {
      auto count = stoi(*iter);
      expect("count not negative", count >= 0);
      dmp::SkinWeight w = {};
      w.count = (size_t) count;
      w.index.resize(w.count);
      w.weight.resize(w.count);

      safeIncr(iter, end); // advance to index
      for (size_t i = 0; i < w.count; ++i)
        {
          w.index[i] = (size_t) stoi(*iter);
          safeIncr(iter, end); // advance to weight
          w.weight[i] = stof(*iter);
          safeIncr(iter, end) // advance to next index or }
        }

      data.weights.push_back(w);
    };

  dmp::parseField<dmp::SkinData>(data, iter, end, tokSkinweights, t, f);
}

static void parseIdxs(dmp::SkinData & data,
                      TokenIterator & iter,
                      TokenIterator & end)
{
  auto t = [](dmp::SkinData & data,
              TokenIterator & iter,
              TokenIterator &)
    {
      auto size = stoi(*iter);
      expect("size not negative", size >= 0);
      data.idxs.reserve((size_t) size);
    };

  auto f = [](dmp::SkinData & data,
               TokenIterator & iter,
               TokenIterator & end)
    {
      auto i = stoi(*iter);
      expect("i not negative", i >= 0);
      data.idxs.push_back((size_t) i);
      safeIncr(iter, end); // advance to next index or }
    };

  dmp::parseField<dmp::SkinData>(data, iter, end, tokTriangles, t, f);
}

static void parseBindings(dmp::SkinData & data,
                          TokenIterator & iter,
                          TokenIterator & end)
{
  auto t = [](dmp::SkinData & data,
              TokenIterator & iter,
              TokenIterator &)
    {
      auto size = stoi(*iter);
      expect("size not negative", size >= 0);
      data.invBindings.reserve(size);
    };

  auto f = [](dmp::SkinData & data,
               TokenIterator & iter,
               TokenIterator & end)
    {
      expect("Actually parsing a matrix", *iter == tokMatrix)
      safeIncr(iter, end);
      expect("matrix followed by opening brace", *iter == tokOpenBrace);
      safeIncr(iter, end);

      float ax, ay, az, bx, by, bz, cx, cy, cz, dx, dy, dz;
      dmp::parseVec3(ax, ay, az, iter, end);
      dmp::parseVec3(bx, by, bz, iter, end);
      dmp::parseVec3(cx, cy, cz, iter, end);
      dmp::parseVec3(dx, dy, dz, iter, end);


      glm::mat4 B =
      {
        ax,   bx,   cx,   dx,
        ay,   by,   cy,   dy,
        az,   bz,   cz,   dz,
        0.0f, 0.0f, 0.0f, 1.0f
      };

      /*
      glm::mat4 B =
      {
        ax, ay, az, 0.0f,
        bx, by, bz, 0.0f,
        cx, cy, cz, 0.0f,
        dx, dy, dz, 1.0f
      };
      */

      data.invBindings.push_back(glm::transpose(glm::inverse(B)));
      safeIncr(iter, end); // advance to next matrix or }
    };

  dmp::parseField<dmp::SkinData>(data, iter, end, tokBindings, t, f);
}

static void parseTexCoords(dmp::SkinData & data,
                           TokenIterator & iter,
                           TokenIterator & end)
{
  auto t = [](dmp::SkinData & data,
              TokenIterator & iter,
              TokenIterator &)
    {
      size_t size = stoi(*iter);
      data.texCoords.reserve(size);
    };

  auto f = [](dmp::SkinData & data,
               TokenIterator & iter,
               TokenIterator & end)
    {
      float x, y;
      dmp::parseVec2(x, y, iter, end);
      data.texCoords.push_back({x, 1.0f - y});
    };

  dmp::parseField<dmp::SkinData>(data, iter, end, tokTexcoords, t, f);
}

static void parseTexFile(dmp::SkinData & data,
                         TokenIterator & iter,
                         TokenIterator & end)
{
  auto t = [](dmp::SkinData &,
              TokenIterator &,
              TokenIterator &)
    {
      // don't care about the texture name
    };

  auto f = [](dmp::SkinData & data,
               TokenIterator & iter,
               TokenIterator & end)
    {
      expect("first token is texture", *iter == tokTexture);
      safeIncr(iter, end); // advance to filename
      data.texFile = "res/textures/" + *iter;
      safeIncr(iter, end); // advance to }
    };

  dmp::parseField<dmp::SkinData>(data, iter, end, tokMaterial, t, f);
}

// -----------------------------------------------------------------------------
// End of parse functions
// -----------------------------------------------------------------------------

/*static void printSkin(const dmp::SkinData & data)
{
  for (size_t i = 0; i < data.idxs.size(); ++i)
    {
      std::cerr << data.idxs[i]
                << " -> {" << glm::to_string(data.verts[data.idxs[i]])
                << ", " << glm::to_string(data.normals[data.idxs[i]])
                << ", " << glm::to_string(data.texCoords[data.idxs[i]])
                << "}" << std::endl;
      std::cerr << data.idxs[i]
                << " -> {|attachments| = " << data.weights[data.idxs[i]].count << std::endl;
      for (size_t j = 0; j < data.weights[data.idxs[i]].count; ++j)
        {
          std::cerr << "   " << data.weights[data.idxs[i]].index[j] << " <-> "
                    << data.weights[data.idxs[i]].weight[j] << std::endl;
        }
      std::cerr << "}" << std::endl;
    }
  std::cerr
    << "binding matricies"
    << std::endl << "invB_0"
    << std::endl << "..."
    << std::endl << "invB_n"
    << std::endl;
  for (const auto & curr : data.invBindings)
    {
      std::cerr << glm::to_string(curr) << std::endl;
    }
    }*/

void dmp::Skin::initSkin(const std::string & skinPath)
{
  mSkinData = {};
  mSkinData.filename = skinPath;

  std::string dataStr;
  readFile(skinPath, dataStr);

  auto sep = whitespaceSeparator;
  tokenizer<char_separator<char>> tokens(dataStr, sep);

  auto iter = tokens.begin();
  auto end = tokens.end();

  bool hasTexCoords = false;
  bool hasTexPath = false;

  while (iter != end)
    {
      if (*iter == tokPositions) parsePositions(mSkinData, iter, end);
      else if (*iter == tokNormals) parseNormals(mSkinData, iter, end);
      else if (*iter == tokSkinweights) parseSkinWeights(mSkinData, iter, end);
      else if (*iter == tokTriangles) parseIdxs(mSkinData, iter, end);
      else if (*iter == tokBindings) parseBindings(mSkinData, iter, end);
      else if (*iter == tokTexcoords)
        {
          hasTexCoords = true;
          parseTexCoords(mSkinData, iter, end);
        }
      else if (*iter == tokMaterial)
        {
          hasTexPath = true;
          parseTexFile(mSkinData, iter, end);
        }
    }

  expect("either both tex coords and tex path found, or neither found",
         (hasTexCoords && hasTexPath)
         || (!hasTexCoords && !hasTexPath));

  mIsTextured = (hasTexCoords && hasTexPath);

  if (!mIsTextured)
    {
      mSkinData.texFile = "";
      while(mSkinData.texCoords.size() < mSkinData.verts.size())
        {
          mSkinData.texCoords.push_back({0.0f, 0.0f});
        }
    }

  //ifDebug(printSkin(mSkinData));

  expect("|verts| = |normals|",
         mSkinData.verts.size() == mSkinData.normals.size());
  expect("|verts| = |texCoords|",
         mSkinData.verts.size() == mSkinData.texCoords.size());

  expect("all SkinWeights vectors have same length",
         [&s=mSkinData](){
           for (const auto & curr : s.weights)
             {
               if (curr.index.size() != curr.count)
                 {
                   return false;
                 }
               else if (curr.count != curr.weight.size())
                 {
                   return false;
                 }
             }
           return true;
         }());

  for (size_t i = 0; i < mSkinData.weights.size(); ++i)
    {
      for (size_t j = 0; j < mSkinData.weights[i].index.size(); ++j)
        {
          expect("Skin weight index valid",
                 mSkinData.weights[i].index[j] < mSkinData.invBindings.size());
        }
    }
}

void dmp::Skin::insertInScene(std::vector<Object *> & objs,
                              size_t matIdx,
                              size_t texIdx)
{
  std::vector<ObjectVertex> verts;
  verts.reserve(mSkinData.verts.size());
  for (size_t i = 0; i < mSkinData.verts.size(); ++i)
    {
      auto weight = mSkinData.weights[i];
      expect("We support max 4 weights", weight.count <= 4);
      expect("there must be > 0 weights", weight.count > 0);
      int idxs[4] = {-1, -1, -1, -1};
      glm::vec4 wvals = {0.0f, 0.0f, 0.0f, 0.0f};

      for (int j = 0; j < (int) weight.count; ++j)
        {
          idxs[j] = (int) weight.index[j];
          wvals[j] = weight.weight[j];
        }

      ObjectVertex v = {mSkinData.verts[i],
                        mSkinData.normals[i],
                        mSkinData.texCoords[i],
                        wvals,
                        {idxs[0], idxs[1], idxs[2], idxs[3]}};
      verts.push_back(v);
    }
  std::vector<GLuint> idxs;
  idxs.reserve(mSkinData.idxs.size());
  for (const auto & curr : mSkinData.idxs)
    {
      idxs.push_back((GLuint) curr);
    }

  mObject = std::make_unique<Object>(verts,
                                     idxs,
                                     GL_TRIANGLES,
                                     matIdx,
                                     texIdx,
                                     GL_DYNAMIC_DRAW);
  mObject->show();

  objs.push_back(mObject.get());

}

void dmp::Skin::hide()
{
  if (mObject) mObject->hide();
}

void dmp::Skin::show()
{
  if (mObject) mObject->show();
}

void dmp::Skin::update(float deltaT,
                       glm::mat4 M,
                       bool dirty)
{
  if (mObject) mObject->setM(M);
}

void dmp::Skin::tellBindingMats(const std::vector<glm::mat4> & m)
{
  expect("bone Ms has same length as binding Ms",
         m.size() == mSkinData.invBindings.size());
  expect("object not null", mObject);

  std::vector<glm::mat4> outToObj(0);
  outToObj.reserve(m.size());

  for (size_t i = 0; i < m.size(); ++i)
    {
      outToObj.push_back(m[i] * mSkinData.invBindings[i]);
    }

  mObject->tellBindingMats(outToObj);
}

void dmp::Skin::applyMorph(const Morph & morph)
{
  auto f = [&morph](ObjectVertex * data,
                    size_t numElems)
    {
      for (const auto & curr : morph.verts)
        {
          expect("morph index in range", curr.first < numElems);
          auto normal = morph.normals.find(curr.first);
          expect("Morph has correspoinding normal",
                 normal != morph.normals.end());
          data[curr.first].position = curr.second;
          data[curr.first].normal = morph.normals.at(curr.first);
        }
    };

  mObject->updateVertices(f);
}
