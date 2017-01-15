#include "Scene.hpp"

#include "Scene/Object.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "config.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace Assimp;
using namespace dmp;

static void processMesh(std::vector<ObjectVertex> & verts,
                        std::vector<GLuint> & idxs,
                        aiMesh * mesh,
                        const aiScene * scene)
{
  for (size_t i = 0; i < mesh->mNumVertices; ++i)
    {
      ObjectVertex v =
        {
          {
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z,
            1.0f
          },
          {
            mesh->mNormals[i].x,
            mesh->mNormals[i].y,
            mesh->mNormals[i].z,
            1.0f
          },
          { // TODO: texcoords
            0.0f,
            0.0f
          }
        };

      verts.push_back(v);
    }

  for (size_t i = 0; i < mesh->mNumFaces; ++i)
    {
      auto face = mesh->mFaces[i];
      if (face.mNumIndices != 3) continue; // not willing to open this can of worms...

      for (size_t j = 0; j < face.mNumIndices; ++j)
        {
          idxs.push_back((GLuint) face.mIndices[j]);
        }
    }
}

static void processNode(std::vector<ObjectVertex> & verts,
                        std::vector<GLuint> & idxs,
                        aiNode * node,
                        const aiScene * scene)
{
  for (size_t i = 0; i < node->mNumMeshes; ++i)
    {
      aiMesh * mesh = scene->mMeshes[node->mMeshes[i]];
      processMesh(verts, idxs, /*mats,*/ mesh, scene);
    }

  for (size_t i = 0; i < node->mNumChildren; ++i)
    {
      processNode(verts, idxs, /*mats,*/ node->mChildren[i], scene);
    }
}

static void loadTestModel(std::vector<ObjectVertex> & verts,
                          std::vector<GLuint> & idxs)
{
  Importer imp;
  imp.SetPropertyBool(AI_CONFIG_PP_PTV_NORMALIZE, true);

  std::cerr << "loading: " << testModel << "..." << std::endl;
  auto model = imp.ReadFile(testModel,
                            aiProcess_Triangulate
                            | aiProcess_GenNormals
                            /*| aiProcess_JoinIdenticalVertices
                            | aiProcess_Triangulate
                            | aiProcess_GenSmoothNormals
                            | aiProcess_PreTransformVertices
                            | aiProcess_ValidateDataStructure
                            | aiProcess_ImproveCacheLocality
                            | aiProcess_RemoveRedundantMaterials
                            | aiProcess_FindInvalidData
                            | aiProcess_GenUVCoords
                            | aiProcess_TransformUVCoords
                            | aiProcess_FindInstances
                            | aiProcess_OptimizeMeshes
                            | aiProcess_OptimizeGraph
                            | aiProcess_Debone*/);

  std::cerr << "processing..." << std::endl;

  expect("Read model file",
         !(!model || model->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !model->mRootNode));

  processNode(verts, idxs, model->mRootNode, model);

  std::cerr << "done!" << std::endl;
}

void dmp::Scene::build(std::function<bool(glm::mat4 &, float)> cameraFn)
{
  graph = std::make_unique<Branch>();

  //loadTestModel(verts, idxs);

  skeleton = std::make_unique<Skeleton>("dragon.skel");

  skeleton->insertInScene(graph.get(),
                          objects,
                          0, 0);
  // Object o(Cube, glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f),
  //          glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
  //          glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
  //          0, 0);
  // objects.push_back(graph->insert(o));
  // o = Object(Cube, glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f),
  //            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
  //            glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
  //            1, 0);
  // objects.push_back(graph->insert(o));
  Object::sortByMaterial(objects);

  materials.push_back( // Ruby = 0
    {
      {
        0.1745f, 0.01175f, 0.01175f, 1.0f
      },
      {
        0.61424f, 0.04136f, 0.04136f, 1.0f
      },
      {
        0.727811f, 0.626959f, 0.626959f, 1.0f
      },
      0.6f
    });
  materials.push_back( // Pearl = 1
    {
      {
        0.25f, 0.20725f, 0.20725f, 1.0f
      },
      {
        1.0f, 0.829f, 0.829f, 1.0f
      },
      {
        0.296648, 0.296648, 0.296648, 1.0f
      },
      0.088f
    });

  std::string tex("");
  textures.emplace_back(tex);

  materialConstants =
    std::make_unique<UniformBuffer>(materials.size(),
                                    Material::std140Size());

  for (size_t i = 0; i < materials.size(); ++i)
    {
      materialConstants->update(i, materials[i]);
    }

  lights.push_back({
      {1.0f, 1.0f, 1.0f, 1.0f},
      {0.0f, -0.1f, 1.0f, 0.0f},
      glm::mat4()
    });
  lights.push_back({
      {0.1f, 0.1f, 0.1f, 1.0f},
      {0.0f, -1.0f, 0.1f, 0.0f},
      glm::mat4()
    });

  auto antiY = graph->transform([](glm::mat4 & M, float deltaT)
    {
      M = glm::rotate(M,
                      -(deltaT / 2.0f),
                      glm::vec3(0.0f, 1.0f, 0.0f));

      return true;
    });
  auto cam = graph->transform(cameraFn);

  antiY->insert(lights[0]);
  graph->insert(lights[1]);

  cameras.emplace_back();
  graph->insert(cameras[0].focus());
  cam->insert(cameras[0].pos());

  objectConstants
    = std::make_unique<UniformBuffer>(objects.size(),
                                      ObjectConstants::std140Size());

  std::vector<const char *> sb;
  for (size_t i = 0; i < 6; ++i) sb.push_back(skyBox[i]);

  skybox = std::make_unique<Skybox>(sb);

  graph->update(0.0f, glm::mat4(), true);
}

void dmp::Scene::update(float deltaT)
{
  expect("Object constant buffer not null",
         objectConstants);

  graph->update(deltaT);

  for (size_t i = 0; i < objects.size(); ++i)
    {
      if (objects[i]->isDirty())
        {
          objectConstants->update(i, objects[i]->getObjectConstants());
          objects[i]->setClean();
        }
    }

  for (auto & curr : cameras)
    {
      curr.update();
    }
}

void dmp::Scene::free()
{
  for (auto & curr : objects)
    {
      curr->freeObject();
    }

  for (auto & curr : textures)
    {
      curr.freeTexture();
    }

  skybox->freeSkybox();
}
