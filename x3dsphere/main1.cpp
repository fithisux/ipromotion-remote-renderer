/*
===========================================================================
IPROMOTION REMOTE RENDERER Source Code
This file is part of the IPROMOTION REMOTE RENDERER Source Code (?IPROMOTION REMOTE RENDERE Source Code?).  
IPROMOTION REMOTE RENDERE Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
IPROMOTION REMOTE RENDERE Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with IPROMOTION REMOTE RENDERE Source Code.  If not, see <http://www.gnu.org/licenses/>.
In addition, the IPROMOTION REMOTE RENDERE Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.
If you have questions concerning this license or the applicable additional terms, you may contact in writing Vasileios Anagnostopoulos, Campani 3 Street, Athens Greece, POBOX 11252.

Dr. Vasileios Anagnostopoulos is the developer of the initial code under project Ipromotion.
===========================================================================
*/
// This is the example from the H3D Manual(Examples section)

// Files included that are needed for setting up the scene graph
#include <H3D/X3D.h>
#include <H3D/Scene.h>
#include <H3D/MouseSensor.h>
#include <H3D/Material.h>
#include "GLFWWindow.h"

using namespace H3D;

// The Color class is of type SFColor and its value is determined by 
// the SFBool field that is routed to it. If its value is true the color
// is red, otherwise it is blue.
class Color : public TypedField< SFColor, SFBool > {
protected:
  virtual void update() {
    if( static_cast< SFBool * >(event.ptr)->getValue() )
      value = RGB( 1, 0, 0 );
    else
      value = RGB( 0, 0, 1 );
  }
};

int main(int argc, char* argv[]) {
  // Set up the scene graph by specifying a string 
  // and using createX3DNodeFromString
  string scene_graph_string = "<Group>"
                              "<Viewpoint position=\"0 0 1\" />"
                              "  <Shape>"
                              "    <Appearance>"
                              "      <Material DEF=\"MATERIAL\" />"
                              "    </Appearance>"
                              "    <Sphere radius=\"0.1\" />"
                              "  </Shape>"
                              "  <MouseSensor DEF=\"MS\" />"
                              "</Group>";

  // myDefNodes contains functions for getting a Node from the scenegraph
  // by giving the DEF name of the node as a string.
  X3D::DEFNodes def_nodes = X3D::DEFNodes();

  // createX3DNodeFromString returns an AutoRef containing a pointer 
  // to the top most node in the given string
  AutoRef< Node > group( X3D::createX3DNodeFromString( 
                            scene_graph_string, &def_nodes ) );

  // Getting the nodes needed for routes
  MouseSensor *mouse_sensor = 
    static_cast< MouseSensor * > (def_nodes.getNode("MS"));
  Material *material = 
    static_cast< Material * > (def_nodes.getNode("MATERIAL"));
  
  // Creating and instance of the Color class needed for routes.
  auto_ptr< Color > myColor( new Color() );

  // Setting up routes
  mouse_sensor->leftButton->route( myColor );
  myColor->route( material->diffuseColor );

  /// Create a new scene
  AutoRef< Scene > scene( new Scene );
  
  // create a window to display
  GLFWWindow *gl_window = new GLFWWindow;
  gl_window->setSize(100,100);
  // add the window to the scene.
  scene->window->push_back( gl_window );
  
  // add our group node that is to be displayed to the scene.
  scene->sceneRoot->setValue( group.get() );
  
  // start the main loop
  gl_window->loopMe();
}

