// Copyright 2021 The Manifold Authors.
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

#pragma once
#include <functional>

#include "public.h"

namespace manifold {

/** @addtogroup Private
 *  @{
 */

/**
 * Polygon vertex.
 */
struct PolyVert {
  /// X-Y position
  glm::vec2 pos;
  /// ID or index into another vertex vector
  int idx;
};

using SimplePolygonIdx = std::vector<PolyVert>;
using PolygonsIdx = std::vector<SimplePolygonIdx>;

std::vector<glm::ivec3> Triangulate(const PolygonsIdx &polys,
                                    float precision = -1);

ExecutionParams &PolygonParams();
/** @} */
}  // namespace manifold