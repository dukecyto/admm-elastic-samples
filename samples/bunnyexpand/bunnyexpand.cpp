// Copyright (c) 2016 University of Minnesota
// 
// ADMM-Elastic Uses the BSD 2-Clause License (http://www.opensource.org/licenses/BSD-2-Clause)
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other materials
//    provided with the distribution.
// THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE UNIVERSITY OF MINNESOTA, DULUTH OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
// OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
// IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "SimContext.hpp"
#include "MCL/Application.hpp"
#include "ArgParser.hpp"
#include <random>

using namespace mcl;
using namespace admm;

bool demo_mode = false;
void step_callback( System *system );

std::unique_ptr<SimContext> context;

// Random number stuff
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<double> dis(-0.75,0.75);

int main(int argc, char *argv[]){

	mcl::ArgParser args(argc,argv);
	demo_mode = args.exists( "demo" );
	bool single_point_init = args.exists( "point" );

	// Print usage
	if( args.exists("help") ){
		std::cout << "Usage:\n\t./bunnyexpand [--demo] [--point]" << std::endl;
		std::cout << "\tdemo: Turns on save screenshots and exits after 5 seconds" << std::endl;
		std::cout << "\tpoint: collapses to (0 0 0)" << std::endl;
		std::cout << std::endl;
		return 0;
	}

	std::stringstream conf_ss; conf_ss << SRC_ROOT_DIR << "/samples/bunnyexpand/bunnyexpand.xml";

	// Create and initialize the dynamics context
	context = std::unique_ptr<SimContext>( new SimContext );
	context->load( conf_ss.str() );
	context->initialize();

	// Add the step callback
	std::function<void (System*)> step_cb(step_callback);
	context->system->add_step_callback( step_cb );

	// Move the nodes of the vertices to a bad location
	if( single_point_init ){

		// Set every node position to 0
		for( int i=0; i<context->system->m_x.size(); i+=3 ){
			context->system->m_x[i]   = 0.0;
			context->system->m_x[i+1] = 0.0;
			context->system->m_x[i+2] = 0.0;
		}
		context->update( context->scene.get() );
		
	} else {
	
		// Scramble the node locations
		for( int i=0; i<context->system->m_x.size(); i+=3 ){
			context->system->m_x[i]   = dis(gen);
			context->system->m_x[i+1] = dis(gen);
			context->system->m_x[i+2] = dis(gen);
		}
		context->update( context->scene.get() );

	}

	// Load the gui
	mcl::Application app( context->scene.get(), context.get() );
	app.settings.gamma_correction = false;
	app.settings.subdivide_meshes = true;
	if( demo_mode ){ app.settings.save_frames=true; }


	if( single_point_init ){
		app.zoom = 6.f;
		// Need to remake the 3pt lighting since the starting scene radius is zero
		context->scene->make_3pt_lighting( trimesh::vec(0,0,0), app.zoom );
	} else { app.zoom = 6.f; }

	// Render
	app.display();

	return 0;
}


double elapsed_s = 0.f;
void step_callback( System *system ){
	if( !demo_mode ){ return; }
	if( elapsed_s > 5.f ){ exit(0); }
	elapsed_s += system->timestep_s;
}
