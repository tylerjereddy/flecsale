/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2016 Los Alamos National Laboratory, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/
///////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Defines a struct that contains all the inputs.
///////////////////////////////////////////////////////////////////////////////
#pragma once 

// user includes
#include "../inputs.h"

#include <ale/mesh/burton/burton.h>
#include <ale/mesh/factory.h>
#include <ale/utils/lua_utils.h>

// system includes
#include <string>

namespace apps {
namespace hydro {

///////////////////////////////////////////////////////////////////////////////
//! \brief A struct that contains all the inputs for a 2d case.
///////////////////////////////////////////////////////////////////////////////
class inputs_t : public inputs_<2> {
public:

  //! \brief The base class.
  using base_t = inputs_<2>;

  //===========================================================================
  //! \brief Load the input file
  //! \param [in] file  The name of the lua file to load.
  //===========================================================================
  static void load(const std::string & file) 
  {
    auto ext = ale::utils::file_extension(file);
    if ( ext == "lua" ) {
      load_lua(file);
    }
    else
      raise_runtime_error(
        "Unknown file extension for \""<<file<<"\""
      );
  }
  
  //===========================================================================
  //! \brief Load the lua input file
  //! \param [in] file  The name of the lua file to load.
  //===========================================================================
  static void load_lua(const std::string & file) 
  {
    // setup the lua interpreter and read the common inputs
    auto state = base_t::load_lua(file);

    // get the hydro table
    auto hydro_ics = state["hydro"];

    // now set some dimension specific inputs

    // set the ics function
    auto ics_func = hydro_ics["ics"];
    ics = [ics_func]( const vector_t & x )
      {
        real_t d, p;
        vector_t v(0);
        std::tie(d, v, p) = 
          ics_func(x[0], x[1]).as<real_t, vector_t, real_t>();
        return std::make_tuple( d, v, p );
      };
      
    // now set the mesh building function
    auto mesh_input = hydro_ics["mesh"];
    auto mesh_type = lua_try_access(mesh_input, "type", std::string );
    if ( mesh_type == "box" ) {
      auto dims = lua_try_access( mesh_input, "dimensions", std::vector<int> );
      auto lens = lua_try_access( mesh_input, "lengths", std::vector<real_t> );
      make_mesh = [=](void)
      {
        return ale::mesh::box<mesh_t>( 
          dims[0], dims[1], lens[0], lens[1]
        );
      };
    }
    else if (mesh_type == "read" ) {
      auto file = lua_try_access( mesh_input, "file", std::string );
      make_mesh = [=](void)
      {
        mesh_t m;
        ale::mesh::read_mesh(file, m);
        return m;
      };
    }
    else {
      raise_implemented_error("Unknown mesh type \""<<mesh_type<<"\"");
    }
  }
};

} // namespace hydro
} // namespace apps
