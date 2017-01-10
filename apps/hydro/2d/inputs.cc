/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
///////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief This is where the compile time inputs are defined.
///////////////////////////////////////////////////////////////////////////////

// hydro includes
#include "inputs.h"

namespace apps {
namespace hydro {

// type aliases confined to this translation unit
using size_t = inputs_t::size_t;
using real_t = inputs_t::real_t;
using vector_t = inputs_t::vector_t;
using string = std::string;
using base_t = inputs_t::base_t;


// the case prefix
template<> string base_t::prefix = "shock_box_2d";
template<> string base_t::postfix = "dat";

// output frequency
template<> size_t base_t::output_freq = 100;

//! \brief the CFL and final solution time
template<> real_t base_t::CFL = 1.0/2.0;
template<> real_t base_t::final_time = 0.2;
template<> size_t base_t::max_steps = 1e6;

// this is a lambda function to set the initial conditions
template<>
inputs_t::ics_function_t base_t::ics = []( const inputs_t::vector_t & x )
{
  real_t d, p;
  vector_t v(0);
  if ( x[0] < 0 && x[1] < 0 ) {
    d = 0.125;
    p = 0.1;
  }
  else {
    d = 1.0;
    p = 1.0;
  }    
  return std::make_tuple( d, v, p );
};

// This function builds and returns a mesh
template<>
inputs_t::mesh_function_t base_t::make_mesh = [](void)
{ 
  // the grid dimensions
  constexpr size_t num_cells_x = 10;
  constexpr size_t num_cells_y = 10;

  constexpr real_t length_x = 1.0;
  constexpr real_t length_y = 1.0;

  // this is the mesh object
  auto mesh = ale::mesh::box<mesh_t>( 
    num_cells_x, num_cells_y, length_x, length_y
  );

  return mesh;
};

} // namespace
} // namespace
