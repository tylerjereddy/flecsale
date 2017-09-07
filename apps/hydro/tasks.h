/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Simple tasks related to solving full hydro solutions.
////////////////////////////////////////////////////////////////////////////////

#pragma once

// hydro includes
#include "types.h"

// flecsi includes
#include <flecsale/io/io_exodus.h>
#include <flecsale/utils/string_utils.h>
#include <flecsi/execution/context.h>
#include <flecsi/execution/execution.h>

// system includes
#include <iomanip>

namespace apps {
namespace hydro {

////////////////////////////////////////////////////////////////////////////////
//! \brief The main task for setting initial conditions
//!
//! \param [in,out] mesh the mesh object
//! \param [in]     ics  the initial conditions to set
//! \return 0 for success
////////////////////////////////////////////////////////////////////////////////
void initial_conditions( 
  client_handle_r__<mesh_t>  mesh,
  inputs_t::ics_function_t ics, 
  eos_t eos,
  mesh_t::real_t soln_time,
  dense_handle_w__<state_data_t> U
) {

  using eqns_t = eqns__< mesh_t::num_dimensions >;

  // This doesn't work with lua input
  //#pragma omp parallel for
  for ( auto c : mesh.cells( flecsi::owned ) ) {
    std::tie( 
      eqns_t::density(U(c)),
      eqns_t::velocity(U(c)),
      eqns_t::pressure(U(c))
    ) = ics( c->centroid(), soln_time );
    eqns_t::update_state_from_pressure( U(c), eos );
  }

}


////////////////////////////////////////////////////////////////////////////////
//! \brief The main task to compute the time step size.
//!
//! \tparam E  The equation of state object to use.
//! \param [in,out] mesh the mesh object
//! \return 0 for success
////////////////////////////////////////////////////////////////////////////////
mesh_t::real_t evaluate_time_step(
  client_handle_r__<mesh_t> mesh,
  dense_handle_r__<state_data_t> U
) {
 
  using real_t = typename mesh_t::real_t;
  using eqns_t = eqns__< mesh_t::num_dimensions >;

  // Loop over each cell, computing the minimum time step,
  // which is also the maximum 1/dt
  real_t dt_inv(0);

  for ( auto c : mesh.cells( flecsi::owned ) ) {

    // loop over each face
    for ( auto f : mesh.faces(c) ) {
      // estimate the length scale normal to the face
      auto delta_x = c->volume() / f->area();
      // compute the inverse of the time scale
      auto dti = eqns_t::fastest_wavespeed( U(c), f->normal() ) / delta_x;
      // check for the maximum value
      dt_inv = std::max( dti, dt_inv );
    } // edge

  } // cell

  if ( dt_inv <= 0 ) 
    raise_runtime_error( "infinite delta t" );

  return 1 / dt_inv;

}

////////////////////////////////////////////////////////////////////////////////
//! \brief The main task to evaluate fluxes at each face.
//!
//! \param [in,out] mesh the mesh object
//! \return 0 for success
////////////////////////////////////////////////////////////////////////////////
void evaluate_fluxes( 
  client_handle_r__<mesh_t> mesh,
  dense_handle_r__<state_data_t> U,
  dense_handle_w__<flux_data_t> flux
) {

  // type aliases
  using eqns_t = eqns__< mesh_t::num_dimensions >;
  
  for ( auto f : mesh.faces( flecsi::owned ) ) 
  {
    
    // get the cell neighbors
    const auto & cells = mesh.cells(f);
    auto num_cells = cells.size();

    // get the left state
    const auto & w_left = U( cells[0] );
    
    // compute the face flux
    //
    // interior cell
    if ( num_cells == 2 ) {
      const auto & w_right = U( cells[1] );
      flux(f) = flux_function<eqns_t>( w_left, w_right, f->normal() );
    } 
    // boundary cell
    else {
      flux(f) = boundary_flux<eqns_t>( w_left, f->normal() );
    }
   
    // scale the flux by the face area
    flux(f) *= f->area();

  } // for
  //----------------------------------------------------------------------------

}

////////////////////////////////////////////////////////////////////////////////
//! \brief The main task to update the solution in each cell.
//!
//! \param [in,out] mesh the mesh object
//! \return 0 for success
////////////////////////////////////////////////////////////////////////////////
void apply_update( 
  client_handle_r__<mesh_t> mesh,
  eos_t eos,
  mesh_t::real_t delta_t,
  dense_handle_r__<flux_data_t> flux,
  dense_handle_rw__<state_data_t> U
) {

  // type aliases
  using eqns_t = eqns__<mesh_t::num_dimensions>;

  //----------------------------------------------------------------------------
  // Loop over each cell, scattering the fluxes to the cell

  for ( auto c : mesh.cells( flecsi::owned ) )
  {

    // initialize the update
    flux_data_t delta_u( 0 );

    // loop over each connected edge
    for ( auto f : mesh.faces(c) ) {
      
      // get the cell neighbors
      auto neigh = mesh.cells(f);
      auto num_neigh = neigh.size();

      // add the contribution to this cell only
      if ( neigh[0] == c )
        delta_u -= flux(f);
      else
        delta_u += flux(f);

    } // edge

    // now compute the final update
    delta_u *= delta_t/c->volume();

    // apply the update
    auto & u = U(c);
    eqns_t::update_state_from_flux( U(c), delta_u );

    // update the rest of the quantities
    eqns_t::update_state_from_energy( u, eos );

    // check the solution quantities
    if ( eqns_t::internal_energy(u) < 0 || eqns_t::density(u) < 0 ) 
      raise_runtime_error( "Negative density or internal energy encountered!" );

  } // for
  //----------------------------------------------------------------------------
  
}


////////////////////////////////////////////////////////////////////////////////
/// \brief output the solution
////////////////////////////////////////////////////////////////////////////////
void output( 
  client_handle_r__<mesh_t> mesh, 
  char_array_t filename,
  dense_handle_r__<state_data_t> U
) {
  clog(info) << "OUTPUT MESH TASK" << std::endl;
 
  // get the context
  auto & context = flecsi::execution::context_t::instance();
  auto rank = context.color();

  // figure out this ranks file name
  auto name_and_ext = utils::split_extension( filename.str() );
  auto output_filename = 
    name_and_ext.first + "_rank" + apps::common::zero_padded(rank) +
    "." + name_and_ext.second;

  // now outut the mesh
  // flecsale::io::io_exodus__<mesh_t>::write(
  //   output_filename, mesh, &d //, v, e, p, T, a
  // );
}

////////////////////////////////////////////////////////////////////////////////
/// \brief output the solution
////////////////////////////////////////////////////////////////////////////////
void print( 
  client_handle_r__<mesh_t> mesh,
  char_array_t filename
) {

  // get the context
  auto & context = flecsi::execution::context_t::instance();
  auto rank = context.color();

  clog(info) << "PRINT MESH ON RANK " << rank << std::endl;
 
  // figure out this ranks file name
  auto name_and_ext = utils::split_extension( filename.str() );
  auto output_filename = 
    name_and_ext.first + "_rank" + apps::common::zero_padded(rank) +
    "." + name_and_ext.second;

  // dump to file
  std::cout << "Dumping connectivity to: " << output_filename << std::endl;
  std::ofstream file( output_filename );
  mesh.dump( file );

  // close file
  file.close();
  
}


////////////////////////////////////////////////////////////////////////////////
// TASK REGISTRATION
////////////////////////////////////////////////////////////////////////////////

flecsi_register_task(initial_conditions, loc, single|flecsi::leaf);
flecsi_register_task(evaluate_time_step, loc, single|flecsi::leaf);
flecsi_register_task(evaluate_fluxes, loc, single|flecsi::leaf);
flecsi_register_task(apply_update, loc, single|flecsi::leaf);
flecsi_register_task(output, loc, single|flecsi::leaf);
flecsi_register_task(print, loc, single|flecsi::leaf);

} // namespace hydro
} // namespace apps
