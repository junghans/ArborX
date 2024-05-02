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
#include <ArborX_DetailsVector.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(make_euclidean_vector)
{
  using Point = ArborX::ExperimentalHyperGeometry::Point<3, float>;
  using Vector = ArborX::Details::Vector<3, float>;
  static_assert(Point{1, 2, 3} - Point{0, 0, 0} == Vector{1, 2, 3});
  static_assert(Point{4, 5, 6} - Point{1, 2, 3} == Vector{3, 3, 3});
}

BOOST_AUTO_TEST_CASE(vector_dot_product)
{
  using Vector = ArborX::Details::Vector<3, float>;
  static_assert(Vector{1, 0, 0}.dot(Vector{1, 0, 0}) == 1);
  static_assert(Vector{1, 0, 0}.dot(Vector{0, 1, 0}) == 0);
  static_assert(Vector{1, 0, 0}.dot(Vector{0, 0, 1}) == 0);
  static_assert(Vector{1, 1, 1}.dot(Vector{1, 1, 1}) == 3);
}

BOOST_AUTO_TEST_CASE(vector_norm)
{
  using Vector = ArborX::Details::Vector<3, float>;
  BOOST_TEST((Vector{3, 4}.norm()) == 5);
  BOOST_TEST((Vector{6, 13, 18}.norm()) == 23);
}

BOOST_AUTO_TEST_CASE(vector_cross_product)
{
  using Vector = ArborX::Details::Vector<3, float>;

  // clang-format off
  static_assert(Vector{1, 0, 0}.cross(Vector{1, 0, 0}) == Vector{0, 0, 0});
  static_assert(Vector{1, 0, 0}.cross(Vector{0, 1, 0}) == Vector{0, 0, 1});
  static_assert(Vector{1, 0, 0}.cross(Vector{0, 0, 1}) == Vector{0, -1, 0});
  static_assert(Vector{0, 1, 0}.cross(Vector{1, 0, 0}) == Vector{0, 0, -1});
  static_assert(Vector{0, 1, 0}.cross(Vector{0, 1, 0}) == Vector{0, 0, 0});
  static_assert(Vector{0, 1, 0}.cross(Vector{0, 0, 1}) == Vector{1, 0, 0});
  static_assert(Vector{1, 1, 1}.cross(Vector{1, 1, 1}) == Vector{0, 0, 0});
  // clang-format on
}
#undef static_assert
