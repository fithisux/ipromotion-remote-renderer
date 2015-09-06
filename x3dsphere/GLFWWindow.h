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

//////////////////////////////////////////////////////////////////////////////
//    Copyright 2004-2014, SenseGraphics AB
//
//    This file is part of H3D API.
//
//    H3D API is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    H3D API is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with H3D API; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//    A commercial license is also available. Please contact us at 
//    www.sensegraphics.com for more information.
//
//
/// \file GLUTWindow.h
/// \brief Header file for GLUTWindow.
///
//
//////////////////////////////////////////////////////////////////////////////
#ifndef __GLLFWWINDOW_H__
#define __GLFWWINDOW_H__

#include <H3D/H3DWindowNode.h>
#include <H3D/Scene.h>
#include <GLFW/glfw3.h>
#include <glib.h>
#include "wqueue.h"
#include "workitem.h"


namespace H3D {

  /// \ingroup Nodes
  /// \class GLUTWindow
  /// \brief H3DWindowNode implementet using GLUT. 
  /// 
  /// Valid values for the cursorType field are:
  /// "RIGHT_ARROW", "LEFT_ARROW", "INFO", "DESTROY", "HELP",
  /// "CYCLE", "SPRAYCAN", "WAIT", "TEXT", "CROSSHAIR", "UP_DOWN",
  /// "LEFT_RIGHT", "TOP_SIDE", "BOTTOM_SIDE", "LEFT_SIDE", 
  /// "RIGHT_SIDE", "TOP_LEFT_CORNER", "TOP_RIGHT_CORNER",
  /// "BOTTOM_RIGHT_CORNER", "BOTTOM_LEFT_CORNER",
  /// "FULL_CROSSHAIR", "NONE", "DEFAULT"
  ///
  /// For example usage of this class see the H3DLoad application.
  class  __declspec(dllexport) GLFWWindow : public H3DWindowNode {
  public:

    /// Constructor.
    GLFWWindow( Inst< SFInt32       > _width      = 0,
                Inst< SFInt32       > _height     = 0,
                Inst< SFBool        > _fullscreen = 0,
                Inst< SFBool        > _mirrored   = 0,
                Inst< RenderMode    > _renderMode = 0, 
                Inst< SFViewpoint   > _viewpoint  = 0, 
                Inst< SFInt32       > _posX       = 0,
                Inst< SFInt32       > _posY       = 0,
                Inst< SFBool        > _manualCursorControl = 0,
                Inst< SFString      > _cursorType = 0, 
                Inst< SFString      > _gameMode   = 0 );

    /// Destructor.
    ~GLFWWindow();

    /// Calls glutSwapBuffers.
    virtual void swapBuffers(); 

    /// Creates and initializes a GLUT window/
    virtual void initWindow();

    /// Initializes GLUT.
    virtual void initWindowHandler() { initGLFW(); }

    /// Sets fullscreen mode.
    virtual void setFullscreen( bool fullscreen );

    gpointer stealFB();
    
    void makeWindowActive(){glfwMakeContextCurrent(this->window_id);}
    
    /// Given the identifier of a GLUT window the GLWindow instance
    /// that created that window is returned.
    static GLFWWindow * getGLFWWindow( GLFWwindow* gwindow ); 

    void setSize(int awidth,int aheight)
    {
        this->width->setValue( awidth );
        this->height->setValue( aheight );
        if(this->xchgbuffer) g_free(this->xchgbuffer);
        this->xchgbuffer=g_malloc (3 * this->width->getValue());
    }
    /// Returns the GLUT window id for this window.
    GLFWwindow* getGLFWWindowId() {
      return window_id;
    }

    /// GLUT callback function for glutKeyboardFunc.
    static void glfwKeyboardCallaback(GLFWwindow * gwindow,int key,int scancode,int action,int mods);

    /// glut callback function. Calls onMouseButtonAction with the
    /// button and state translated to the enumerated values in KeySensor.h
    static void glfwMouseCallback(GLFWwindow * gwindow,int button,int action, int mods );
    
    /// glut callback function. Calls onMouseMotionAction
    static void glfwMotionCallback(GLFWwindow * gwindow, double xpos, double ypos );

    /// glut callback function for mouse scroll wheel.
    /// Calls onMouseWheelAction with direction translated to the
    /// values in KeySensor.h
    static void glfwMouseWheelCallback(GLFWwindow * gwindow, double xoffset, double yoffset );

    void zoomin(){
        glfwMouseWheelCallback(this->window_id,0,1);
    }

    void zoomout(){
        glfwMouseWheelCallback(this->window_id,0,-1);
    }
    /// Initialize GLUT. 
    static void initGLFW();

    /// Support for GLUT GameMode which is basically a string used to optimize
    /// the glut window for gaming. Some features are disabled for such things.
    /// For more information about GLUT GameMode see for example
    /// http://freeglut.sourceforge.net/docs/api.php#GameMode
    ///
    /// <b>Access type:</b> initializeOnly \n
    auto_ptr< SFString >  gameMode;
    
    /// The H3DNodeDatabase for this node.
    static H3DNodeDatabase database;
    string mytitle;   
    void loopMe();
    void rendonstream(wqueue<WorkItem*>* frame_queue);
  protected:
    static int GLFW_init;
    
    GLFWwindow* window_id;
    gpointer xchgbuffer;
    // Define these even though they are not used for when we use FREEGLUT.
    bool last_fullscreen;
  };
}
#endif
