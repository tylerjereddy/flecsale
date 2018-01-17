/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2018 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
////////////////////////////////////////////////////////////////////////////////
///
/// \file
/// 
/// \brief Tests related to the normal operation.
///
////////////////////////////////////////////////////////////////////////////////

// preprocessor directive for exception handling
#define ENABLE_EXCEPTIONS

// system includes
#include<cinchtest.h>
#include<vector>

// user includes
#include "flecsale/common/types.h"
#include "flecsale/geom/normal.h"
#include "flecsale/geom/point.h"

// explicitly use some stuff
using std::cout;
using std::endl;
using std::vector;

using namespace flecsale;
using namespace flecsale::geom;

//! the real type
using real_t = common::real_t;
// ! the test tolerance
using common::test_tolerance;

//! the point types
using point_2d_t = point<real_t, 2>;

//! the error types used
//using utils::ExceptionNotImplemented;

//=============================================================================
//! \brief Test 2d normal operator calculation result values.
//=============================================================================
TEST(normal, 2dCalculation)
{

  // 2D normal is (-dy, dx), (dy, -dx)
  // The library only returns
  // the first of the above values

  vector<point_2d_t> simple_points = { {0, 0}, {1, 0} };
  
  auto result_1 = normal( simple_points[0],
                          simple_points[1]);

  // NOTE: the Google Mock C++ framework could be used to directly
  // check array values in a single statement
  // see: https://stackoverflow.com/a/2797990/2942522
  // For now, index array data directly
  ASSERT_NEAR( 0.0, result_1[0], test_tolerance ) << " 2D normal calculation wrong ";
  ASSERT_NEAR( 1.0, result_1[1], test_tolerance ) << " 2D normal calculation wrong ";

} // TEST

/* Check that 2D normal calculation result has the
 * expected shape */

TEST(normal, 2dResultShape)
{
  // (-dy, dx) is returned by normal with 2D input
  // Expecting 1 row with 2 columns in the object
  // returned
  vector<point_2d_t> simple_points = { {0, 0}, {1, 0} };

  auto result = normal( simple_points[0],
                        simple_points[1]);

  ASSERT_EQ( 2, result.size());

} // TEST
