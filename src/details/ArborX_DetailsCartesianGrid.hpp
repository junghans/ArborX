/****************************************************************************
 * Copyright (c) 2017-2022 by the ArborX authors                            *
 * All rights reserved.                                                     *
 *                                                                          *
 * This file is part of the ArborX library. ArborX is                       *
 * distributed under a BSD 3-clause license. For the licensing terms see    *
 * the LICENSE file in the top-level directory.                             *
 *                                                                          *
 * SPDX-License-Identifier: BSD-3-Clause                                    *
 ****************************************************************************/

#ifndef ARBORX_DETAILS_CARTESIAN_GRID_HPP
#define ARBORX_DETAILS_CARTESIAN_GRID_HPP

#include <ArborX_DetailsKokkosExtMathFunctions.hpp>
#include <ArborX_Exception.hpp>
#include <ArborX_GeometryTraits.hpp>
#include <ArborX_HyperBox.hpp>

#include <Kokkos_Macros.hpp>

namespace ArborX::Details
{

template <int DIM>
struct CartesianGrid
{
private:
  using Box = ExperimentalHyperGeometry::Box<DIM>;

public:
  static constexpr int dim = DIM;

  CartesianGrid(Box const &bounds, float h)
      : _bounds(bounds)
  {
    ARBORX_ASSERT(h > 0);
    for (int d = 0; d < DIM; ++d)
      _h[d] = h;
    buildGrid();
  }
  CartesianGrid(Box const &bounds, float const h[DIM])
      : _bounds(bounds)
  {
    for (int d = 0; d < DIM; ++d)
    {
      ARBORX_ASSERT(_h[d] > 0);
      _h[d] = h[d];
    }
    buildGrid();
  }

  template <typename Point, typename Enable = std::enable_if_t<
                                GeometryTraits::is_point<Point>{}>>
  KOKKOS_FUNCTION size_t cellIndex(Point const &point) const
  {
    static_assert(GeometryTraits::dimension<Point>::value == DIM);

    auto const &min_corner = _bounds.minCorner();
    size_t s = 0;
    for (int d = DIM - 1; d >= 0; --d)
    {
      int i = KokkosExt::floor((point[d] - min_corner[d]) / _h[d]);
      s = s * _n[d] + i;
    }
    return s;
  }

  KOKKOS_FUNCTION
  Box cellBox(size_t cell_index) const
  {
    auto min = _bounds.minCorner();
    decltype(min) max;
    for (int d = 0; d < DIM; ++d)
    {
      auto i = cell_index % _n[d];
      cell_index /= _n[d];

      max[d] = min[d] + (i + 1) * _h[d];
      min[d] += i * _h[d];
    }
    return {min, max};
  }

  KOKKOS_FUNCTION
  auto extent(int d) const
  {
    assert(0 <= d && d < DIM);
    return _n[d];
  }

private:
  void buildGrid()
  {
    auto const &min_corner = _bounds.minCorner();
    auto const &max_corner = _bounds.maxCorner();
    for (int d = 0; d < DIM; ++d)
    {
      auto delta = max_corner[d] - min_corner[d];
      if (delta != 0)
      {
        _n[d] = std::ceil(delta / _h[d]);
        ARBORX_ASSERT(_n[d] > 0);
      }
      else
      {
        _n[d] = 1;
      }
    }

    // Catch potential overflow in grid cell indices early. This is a
    // conservative check as an actual overflow may not occur, depending on
    // which cells are filled.
    constexpr auto max_size_t = std::numeric_limits<size_t>::max();
    auto m = max_size_t;
    for (int d = 1; d < DIM; ++d)
    {
      m /= _n[d - 1];
      ARBORX_ASSERT(_n[d] < m);
    }
  }

  Box _bounds;
  float _h[DIM];
  size_t _n[DIM];
};

} // namespace ArborX::Details

#endif
