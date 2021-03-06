//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// Copyright (c) 2016-20, Lawrence Livermore National Security, LLC
// and RAJA project contributors. See the RAJA/COPYRIGHT file for details.
//
// SPDX-License-Identifier: (BSD-3-Clause)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

#include "RAJA/RAJA.hpp"
#include "gtest/gtest.h"

template<typename T>
class TypedLayoutUnitTest : public ::testing::Test {};

using MyTypes = ::testing::Types<RAJA::Index_type,
                                 short,
                                 unsigned short,
                                 int,
                                 unsigned int,
                                 long,
                                 unsigned long,
                                 long int,
                                 unsigned long int,
                                 long long,
                                 unsigned long long>;


TYPED_TEST_SUITE(TypedLayoutUnitTest, MyTypes);

TYPED_TEST(TypedLayoutUnitTest, TypedLayoutConstructors)
{

  const RAJA::TypedLayout<TypeParam, RAJA::tuple<TypeParam, TypeParam>> l(10,5);

  ASSERT_EQ(TypeParam{0}, l(TypeParam{0}, TypeParam{0}));

  ASSERT_EQ(TypeParam{2}, l(TypeParam{0}, TypeParam{2}));

  ASSERT_EQ(TypeParam{10}, l(TypeParam{2}, TypeParam{0}));

  TypeParam x{5};
  TypeParam y{0};
  l.toIndices(TypeParam{10}, y, x);
  ASSERT_EQ(x, TypeParam{0});
  ASSERT_EQ(y, TypeParam{2});
}

TYPED_TEST(TypedLayoutUnitTest, 2D_accessor)
{
  using my_layout = RAJA::TypedLayout<TypeParam, RAJA::tuple<TypeParam, TypeParam>>;

  /*
   * Construct a 2D layout:
   *
   * I is stride 5
   * J is stride 1
   *
   * Linear indices range from [0, 15)
   *
   */

  // Construct using variadic "sizes" ctor
  const my_layout layout_a(3, 5);

  // Construct using copy ctor
  const my_layout layout_b(layout_a);

  // Test default ctor and assignment operator
  my_layout layout;
  layout = layout_b;


  ASSERT_EQ(0, layout(0, 0));

  ASSERT_EQ(5, layout(1, 0));
  ASSERT_EQ(15, layout(3, 0));

  ASSERT_EQ(1, layout(0, 1));
  ASSERT_EQ(5, layout(0, 5));

  // Check that we get the identity (mod 15)
  TypeParam pK = 0;
  for (int k = 0; k < 20; ++k) {

    // inverse map
    TypeParam i, j;
    layout.toIndices(pK, i, j);

    // forward map
    TypeParam k2 = layout(i, j);

    // check ident
    ASSERT_EQ(pK % 15, k2);

    // check with a and b
    ASSERT_EQ(k2, layout_a(i, j));
    ASSERT_EQ(k2, layout_b(i, j));
    pK++;
  }

}

TYPED_TEST(TypedLayoutUnitTest, 2D_IJ_ProjJ)
{
  using my_layout = RAJA::TypedLayout<TypeParam, RAJA::tuple<TypeParam, TypeParam>>;

  /*
   * Construct a 2D projective layout:
   *
   * I is stride 1
   * J is stride 0  -  projected out
   *
   * Linear indices range from [0, 7)
   *
   * values of J should have no effect on linear index
   *
   * All linear indices should produce J=0
   *
   */

  // Construct using variadic "sizes" ctor
  // Zero for J size should correctly produce projective layout
  const my_layout layout(7, 0);

  ASSERT_EQ(0, layout(0, 0));

  ASSERT_EQ(1, layout(1, 0));
  ASSERT_EQ(3, layout(3, 0));

  // J should be projected out
  ASSERT_EQ(0, layout(0, 1));
  ASSERT_EQ(0, layout(0, 5));

  TypeParam pK = 0;
  // Check that we get the identity (mod 7)
  for (int k = 0; k < 20; ++k) {

    // inverse map
    TypeParam i, j;
    layout.toIndices(pK, i, j);

    // forward map
    TypeParam k2 = layout(i, j);

    // check ident
    ASSERT_EQ(pK % 7, k2);

    // check projection of j
    ASSERT_EQ(j, 0);
    pK++;
  }
}

TYPED_TEST(TypedLayoutUnitTest, 2D_StaticLayout)
{
  RAJA::Layout<2> dynamic_layout(7, 5);
  using static_layout = RAJA::TypedStaticLayout<RAJA::PERM_IJ,RAJA::list<TypeParam,TypeParam>,7,5>;

  // Check that we get the same layout
  for (TypeParam i = 0; i < 7; ++i) {
    for (TypeParam j = 0; j < 5; ++j) {

      ASSERT_EQ(dynamic_layout(i, j), static_layout::s_oper(i,j));
    }
  }

}

TYPED_TEST(TypedLayoutUnitTest, 2D_PermutedStaticLayout)
{
  auto dynamic_layout =
    RAJA::make_permuted_layout({{7, 5}},
                               RAJA::as_array<RAJA::PERM_JI>::get());
  using static_layout = RAJA::TypedStaticLayout<RAJA::PERM_JI,
                                                RAJA::list<TypeParam,TypeParam>, 7,5>;

  // Check that we get the same layout
  for (TypeParam i = 0; i < 7; ++i) {
    for (TypeParam j = 0; j < 5; ++j) {
      ASSERT_EQ(dynamic_layout(i, j), static_layout::s_oper(i,j));
    }
  }
}

TYPED_TEST(TypedLayoutUnitTest, 3D_PermutedStaticLayout)
{
  auto dynamic_layout =
    RAJA::make_permuted_layout({{7, 13, 5}},
                               RAJA::as_array<RAJA::PERM_JKI>::get());
  using static_layout = RAJA::TypedStaticLayout<RAJA::PERM_JKI,
                                                RAJA::list<TypeParam,TypeParam,TypeParam>,
                                                7,13,5>;

  // Check that we get the same layout
  for (TypeParam i = 0; i < 7; ++i) {
    for (TypeParam j = 0; j < 9; ++j) {
      for (TypeParam k = 0; k < 5; ++k) {
        ASSERT_EQ(dynamic_layout(i, j, k), static_layout::s_oper(i,j,k));
      }
    }
  }
}


TYPED_TEST(TypedLayoutUnitTest, 4D_PermutedStaticLayout)
{
  auto dynamic_layout =
    RAJA::make_permuted_layout({{7, 13, 5, 17}},
                               RAJA::as_array<RAJA::PERM_LJKI>::get());
  using static_layout = RAJA::TypedStaticLayout<RAJA::PERM_LJKI,
                                                RAJA::list<TypeParam,TypeParam,TypeParam,TypeParam>,
                                                7,13,5,17>;

  // Check that we get the same layout
  for (TypeParam i = 0; i < 7; ++i) {
    for (TypeParam j = 0; j < 8; ++j) {
      for (TypeParam k = 0; k < 5; ++k) {
        for (TypeParam l = 0; l < 5; ++l) {
          ASSERT_EQ(dynamic_layout(i, j, k, l), static_layout::s_oper(i,j,k,l));
        }
      }
    }
  }
}
