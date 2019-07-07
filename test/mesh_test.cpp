// Copyright 2019 Emmett Lalish
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <glm/gtc/constants.hpp>
#include "gtest/gtest.h"
#include "manifold.h"

namespace {

using namespace manifold;

void Identical(Mesh& mesh1, Mesh& mesh2) {
  ASSERT_EQ(mesh1.vertPos.size(), mesh2.vertPos.size());
  for (int i = 0; i < mesh1.vertPos.size(); ++i) {
    glm::vec3 v_in = mesh1.vertPos[i];
    glm::vec3 v_out = mesh2.vertPos[i];
    for (int j : {0, 1, 2}) ASSERT_NEAR(v_in[j], v_out[j], 0.0001);
  }
  ASSERT_EQ(mesh1.triVerts.size(), mesh2.triVerts.size());
  for (int i = 0; i < mesh1.triVerts.size(); ++i) {
    ASSERT_EQ(mesh1.triVerts[i], mesh2.triVerts[i]);
  }
}

void CheckIdx(int idx, int dir) {
  EdgeIdx edge(idx, dir);
  ASSERT_EQ(edge.Idx(), idx);
  ASSERT_EQ(edge.Dir(), dir);
}

void ExpectMeshes(const Manifold& manifold,
                  const std::vector<std::pair<int, int>>& numVertTri) {
  ASSERT_TRUE(manifold.IsValid());
  std::vector<Manifold> meshes = manifold.Decompose();
  ASSERT_EQ(meshes.size(), numVertTri.size());
  std::sort(meshes.begin(), meshes.end(),
            [](const Manifold& a, const Manifold& b) {
              return a.NumVert() != b.NumVert() ? a.NumVert() > b.NumVert()
                                                : a.NumTri() > b.NumTri();
            });
  for (int i = 0; i < meshes.size(); ++i) {
    EXPECT_TRUE(meshes[i].IsValid());
    EXPECT_EQ(meshes[i].NumVert(), numVertTri[i].first);
    EXPECT_EQ(meshes[i].NumTri(), numVertTri[i].second);
  }
}
}  // namespace

TEST(Mesh, EdgeIdx) {
  CheckIdx(0, 1);
  CheckIdx(0, -1);
  CheckIdx(1, 1);
  CheckIdx(1, -1);
}

TEST(Mesh, ReadWrite) {
  Mesh mesh = ImportMesh("data/gyroidpuzzle.ply");
  ExportMesh("data/gyroidpuzzle1.ply", mesh);
  Mesh mesh_out = ImportMesh("data/gyroidpuzzle1.ply");
  Identical(mesh, mesh_out);
}

TEST(Manifold, Regression) {
  Manifold manifold(ImportMesh("data/gyroidpuzzle.ply"));
  ASSERT_TRUE(manifold.IsValid());

  Manifold mesh1 = manifold.DeepCopy();
  mesh1.Translate(glm::vec3(5.0f));
  int num_overlaps = manifold.NumOverlaps(mesh1);
  ASSERT_EQ(num_overlaps, 237472);

  Mesh mesh_out = manifold.Extract();
  Manifold mesh2(mesh_out);
  Mesh mesh_out2 = mesh2.Extract();
  Identical(mesh_out, mesh_out2);
}

TEST(Manifold, Decompose) {
  std::vector<Manifold> meshList;
  meshList.push_back(Manifold::Tetrahedron());
  meshList.push_back(Manifold::Cube());
  meshList.push_back(Manifold::Octahedron());
  Manifold meshes(meshList);
  ASSERT_TRUE(meshes.IsValid());

  ExpectMeshes(meshes, {{8, 12}, {6, 8}, {4, 4}});
}

TEST(Manifold, Sphere) {
  int n = 25;
  Manifold sphere = Manifold::Sphere(4 * n);
  ASSERT_TRUE(sphere.IsValid());
  ASSERT_EQ(sphere.NumTri(), n * n * 8);
}

TEST(Manifold, BooleanTetra) {
  Manifold tetra = Manifold::Tetrahedron();
  ASSERT_TRUE(tetra.IsValid());

  Manifold tetra2 = tetra.DeepCopy();
  tetra2.Translate(glm::vec3(0.5f));
  Manifold result = tetra2 - tetra;

  ExpectMeshes(result, {{8, 12}});
}

TEST(Manifold, Volume) {
  Manifold cube = Manifold::Cube();
  ASSERT_TRUE(cube.IsValid());
  float vol = cube.Volume();
  EXPECT_FLOAT_EQ(vol, 8.0f);

  cube.Scale(glm::vec3(-1.0f));
  vol = cube.Volume();
  EXPECT_FLOAT_EQ(vol, -8.0f);
}

TEST(Manifold, SurfaceArea) {
  Manifold cube = Manifold::Cube();
  ASSERT_TRUE(cube.IsValid());
  float area = cube.SurfaceArea();
  EXPECT_FLOAT_EQ(area, 24.0f);

  cube.Scale(glm::vec3(-1.0f));
  area = cube.SurfaceArea();
  EXPECT_FLOAT_EQ(area, 24.0f);
}

TEST(Manifold, SelfSubtract) {
  Manifold cube = Manifold::Cube();
  Manifold empty = cube - cube;
  EXPECT_TRUE(empty.IsValid());
  EXPECT_FLOAT_EQ(empty.Volume(), 0.0f);
  // EXPECT_FLOAT_EQ(empty.SurfaceArea(), 0.0f);
}

TEST(Manifold, Split) {
  Manifold cube = Manifold::Cube();
  Manifold oct = Manifold::Octahedron();
  oct.Translate(glm::vec3(0.0f, 0.0f, 1.0f));
  std::pair<Manifold, Manifold> splits = cube.Split(oct);
  EXPECT_FLOAT_EQ(splits.first.Volume() + splits.second.Volume(),
                  cube.Volume());
}

TEST(Manifold, SplitByPlane) {
  Manifold cube = Manifold::Cube();
  cube.Translate({0.0f, 1.0f, 0.0f});
  cube.Rotate(0.0f, 0.0f, -60.0f);
  cube.Translate({2.0f, 0.0f, 0.0f});
  float phi = glm::pi<float>() / 6.0f;
  std::pair<Manifold, Manifold> splits =
      cube.SplitByPlane({glm::sin(phi), -glm::cos(phi), 0.0f}, 1.0f);
  EXPECT_NEAR(splits.first.Volume(), splits.second.Volume(), 1e-5);
}

TEST(Manifold, BooleanSphere) {
  Manifold sphere = Manifold::Sphere(12);
  Manifold sphere2 = sphere.DeepCopy();
  sphere2.Translate(glm::vec3(0.5));
  Manifold result = sphere - sphere2;

  ExpectMeshes(result, {{74, 144}});
}

TEST(Manifold, Boolean3) {
  Manifold gyroid(ImportMesh("data/gyroidpuzzle.ply"));
  ASSERT_TRUE(gyroid.IsValid());

  Manifold gyroid2 = gyroid.DeepCopy();
  gyroid2.Translate(glm::vec3(5.0f));
  Manifold result = gyroid + gyroid2;

  ExpectMeshes(result, {{31733, 63602}});
}

TEST(Manifold, BooleanSelfIntersecting) {
  std::vector<Manifold> meshList;
  meshList.push_back(Manifold::Tetrahedron());
  meshList.push_back(Manifold::Tetrahedron());
  meshList[1].Translate(glm::vec3(0, 0, 0.25));
  Manifold tetras(meshList);
  ASSERT_TRUE(tetras.IsValid());

  meshList[0].Translate(glm::vec3(0, 0, 0.5f));
  Manifold result = meshList[0] - tetras;

  ExpectMeshes(result, {{8, 12}, {4, 4}});
}

TEST(Manifold, BooleanSelfIntersectingAlt) {
  std::vector<Manifold> meshList;
  meshList.push_back(Manifold::Tetrahedron());
  meshList.push_back(Manifold::Tetrahedron());
  meshList[1].Translate(glm::vec3(0, 0, 0.5));
  Manifold tetras(meshList);
  ASSERT_TRUE(tetras.IsValid());

  meshList[0].Translate(glm::vec3(0, 0, -0.5f));
  Manifold result = meshList[0] - tetras;

  ExpectMeshes(result, {{8, 12}, {4, 4}});
}

TEST(Manifold, BooleanWinding) {
  std::vector<Manifold> meshList;
  meshList.push_back(Manifold::Tetrahedron());
  meshList.push_back(Manifold::Tetrahedron());
  meshList[1].Translate(glm::vec3(0.25));
  Manifold tetras(meshList);
  ASSERT_TRUE(tetras.IsValid());

  meshList[0].Translate(glm::vec3(-0.25f));
  Manifold result = tetras - meshList[0];

  ExpectMeshes(result, {{8, 12}, {8, 12}});
}

TEST(Manifold, BooleanHorrible) {
  Manifold random = Manifold::Sphere(8);
  std::mt19937 gen(12345);  // Standard mersenne_twister_engine
  std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
  random.Warp([&dis, &gen](glm::vec3& v) {
    v = glm::vec3(dis(gen), dis(gen), dis(gen));
  });
  Manifold random2 = random.DeepCopy();
  random2.Rotate(90);
  Manifold result = random ^ random2;
  EXPECT_TRUE(result.IsValid());
}

TEST(Manifold, BooleanHorrible2) {
  Manifold random = Manifold::Sphere(32);
  std::mt19937 gen(54321);  // Standard mersenne_twister_engine
  std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
  random.Warp([&dis, &gen](glm::vec3& v) {
    v = glm::vec3(dis(gen), dis(gen), dis(gen));
  });
  Manifold random2 = random.DeepCopy();
  random2.Rotate(90);
  Manifold result = random ^ random2;
  EXPECT_TRUE(result.IsValid());
}

TEST(Manifold, BooleanHorriblePlanar) {
  Manifold random = Manifold::Sphere(32);
  std::mt19937 gen(654321);  // Standard mersenne_twister_engine
  std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
  random.Warp(
      [&dis, &gen](glm::vec3& v) { v = glm::vec3(dis(gen), dis(gen), 0.0f); });
  Manifold random2 = random.DeepCopy();
  float a = 0.2;
  float phi = asin(a);
  random.Rotate(0, glm::degrees(-phi));
  random2.Rotate(glm::degrees(phi));
  Manifold result = random ^ random2;
  result.Rotate(0, 0, 45).Rotate(glm::degrees(atan(sqrt(2.0f) / tan(phi))));
  EXPECT_TRUE(result.IsValid());
  Box BB = result.BoundingBox();
  float tol = 1e-7;
  EXPECT_NEAR(BB.Center().x, 0.0f, tol);
  EXPECT_NEAR(BB.Center().y, 0.0f, tol);
  EXPECT_NEAR(BB.Size().x, 0.0f, tol);
  EXPECT_NEAR(BB.Size().y, 0.0f, tol);
  EXPECT_GT(BB.Size().z, 1.0f);
  EXPECT_LT(BB.Size().z, 4.0f);
}