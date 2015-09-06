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
/// \file GLUTWindow.cpp
/// \brief CPP file for GLUTWindow.
///
//
//
//////////////////////////////////////////////////////////////////////////////
#include <math.h>
#include "GLFWWindow.h"
#include <H3D/X3DKeyDeviceSensorNode.h>
#include <H3D/MouseSensor.h>
#include "wqueue.h"
#include "workitem.h"


using namespace H3D;

int GLFWWindow::GLFW_init = 0;
// Add this node to the H3DNodeDatabase system.
H3DNodeDatabase GLFWWindow::database( "GLFWWindow", 
                                      &(newInstance<GLFWWindow>), 
                                      typeid( GLFWWindow ),
                                      &(H3DWindowNode::database) );

namespace GLFWWindowInternals {
  FIELDDB_ELEMENT( GLFWWindow , gameMode, INITIALIZE_ONLY );
}

/////////////////////////////////////////////////////////////////////////////
//
// Internal functions for GLUT window handling
//
namespace GLFWWindowInternals {
  void reshapeFunc( GLFWwindow* gwindow, int w, int h ) { 
    GLFWWindow *window = GLFWWindow::getGLFWWindow(gwindow);
    if( window )
      window->reshape( w, h );
  }

  void displayFunc(GLFWwindow* gwindow) {
    GLFWWindow *window = GLFWWindow::getGLFWWindow(gwindow);
    if( window )
      window->display();
  }
}



void GLFWWindow::initGLFW() 
{
  cout << "init1 " << endl;
  cout << GLFWWindow::GLFW_init << endl;
  if ( GLFWWindow::GLFW_init==0 ) {
    cout << "init2 " << endl;
    if(!glfwInit())
    {
        cout << "init3 " << endl;
        exit(EXIT_FAILURE);
    }
  }
  GLFWWindow::GLFW_init++;
  cout << "init4 " << endl;
}

GLFWWindow::GLFWWindow( Inst< SFInt32     > _width,
                        Inst< SFInt32     > _height,
                        Inst< SFBool      > _fullscreen,
                        Inst< SFBool      > _mirrored,
                        Inst< RenderMode  > _renderMode, 
                        Inst< SFViewpoint > _viewpoint,
                        Inst< SFInt32     > _posX,
                        Inst< SFInt32     > _posY,
                        Inst< SFBool      > _manualCursorControl,
                        Inst< SFString    > _cursorType,
                        Inst< SFString    > _gameMode ) :
  H3DWindowNode( _width, _height, _fullscreen, _mirrored, _renderMode,
                 _viewpoint, _posX, _posY, _manualCursorControl, _cursorType ),
  gameMode( _gameMode ){
  
  type_name = "GLFWWindow";
  database.initFields( this );
  last_fullscreen = fullscreen->getValue();  
  this->xchgbuffer=g_malloc (3 * this->width->getValue());
  this->mytitle= static_cast<ostringstream*>( &(ostringstream() << GLFWWindow::GLFW_init) )->str();
}

GLFWWindow::~GLFWWindow() 
{
    if(this->window_id!= NULL) 
    {
        glfwDestroyWindow(this->window_id);
    }
    GLFWWindow::GLFW_init--;  
    if( GLFWWindow::GLFW_init==0 )
    {
      glfwTerminate();
    } 
    
    if(this->xchgbuffer) g_free(this->xchgbuffer);
}

gpointer GLFWWindow::stealFB() {
  
  int default_x = 0;
  int default_y = 0;
  GLint viewport[4];
  glGetIntegerv( GL_VIEWPORT, viewport );
  GLenum error = glGetError();
  if( error != GL_NO_ERROR ) {
    ostringstream err;
    cout << "Could not get the current viewport dimensions"
      << ", screenshot will not be taken:"
      << gluErrorString(error);
    err << "Could not get the current viewport dimensions"
      << ", screenshot will not be taken:"
      << gluErrorString(error);
    return NULL;
  }
  default_x = viewport[0];
  default_y = viewport[1];
  gpointer buff = g_malloc (3 * viewport[2] * viewport[3]);
  //cout << "stole " << 3  << " p " << viewport[2] << " p " << viewport[3] << " p " << endl;
  glPixelStorei( GL_PACK_ALIGNMENT, 1 );
  glPixelStorei( GL_PACK_SKIP_PIXELS, 0 );
  glPixelStorei( GL_PACK_ROW_LENGTH, 0 );
  glPixelStorei( GL_PACK_SKIP_ROWS, 0 );
  glReadPixels( viewport[0], viewport[1], viewport[2], viewport[3], GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *)buff );
  error = glGetError();
  if( error != GL_NO_ERROR ) {
    ostringstream err;
    err << "ReadPixels failed"
      << ", screenshot will not be taken:"
      << gluErrorString(error);
    return NULL;
  }
  return buff;
}

void GLFWWindow::initWindow() {
    if( isInitialized() ) {
      Console(4) << "GLUTWindow does not support changing pixel format from/to "
                 << "quad buffered stereo support after initialization." << endl;
      if( last_render_mode == RenderMode::QUAD_BUFFERED_STEREO ) {
        renderMode->setValue( "QUAD_BUFFERED_STEREO" );
      } else {
        renderMode->setValue( "MONO" );
      }
      return;
    }

    cout << "start initialization " << endl;
    RenderMode::Mode stereo_mode = renderMode->getRenderMode();

    if( stereo_mode == RenderMode::QUAD_BUFFERED_STEREO ) {
      glfwWindowHint( GLFW_STEREO, GL_TRUE );
    }

    glfwWindowHint( GLFW_VISIBLE, GL_FALSE );
    this->window_id=glfwCreateWindow(
                    width->getValue(), height->getValue(),this->mytitle.c_str(),NULL,NULL);    
    if ( !this->window_id)
    {
        glfwTerminate();
        cout << "glfwterminate init " << endl;
        exit(EXIT_FAILURE);
    }    
    glfwMakeContextCurrent(this->window_id);
    glfwSetWindowPos(this->window_id,posX->getValue(), posY->getValue());
    setFullscreen( fullscreen->getValue() );    
    // set up GLFW callback functions
    glfwSetWindowRefreshCallback(this->window_id, GLFWWindowInternals::displayFunc  );
    glfwSetFramebufferSizeCallback (this->window_id, GLFWWindowInternals::reshapeFunc  );    
    glfwSetKeyCallback( this->window_id,GLFWWindow::glfwKeyboardCallaback);    
    glfwSetMouseButtonCallback(this->window_id, GLFWWindow::glfwMouseCallback);
    glfwSetScrollCallback(this->window_id, GLFWWindow::glfwMouseWheelCallback);
    glfwSetCursorPosCallback(this->window_id, GLFWWindow::glfwMotionCallback);
    //glfwShowWindow(this->window_id);
    window_is_made_active = true;
}


// Given the identifier of a GLUT window the GLWindow instance
// that created that window is returned.
GLFWWindow *GLFWWindow::getGLFWWindow( GLFWwindow* gwindow) {
  for( set< H3DWindowNode* >::iterator i = H3DWindowNode::windows.begin(); 
       i != H3DWindowNode::windows.end(); ++i ) {
    GLFWWindow *glfw_window = dynamic_cast< GLFWWindow * >( *i );
    if( glfw_window && glfw_window->getGLFWWindowId() == gwindow )
      return glfw_window;
  }
  return NULL;
}

void GLFWWindow::setFullscreen( bool fullscreen ) {
  if( fullscreen != last_fullscreen ) {
      if( fullscreen ) {
          GLFWmonitor* primary = glfwGetPrimaryMonitor();
          const GLFWvidmode * videomode= glfwGetVideoMode( primary );
          glfwDestroyWindow(this->window_id);
          this->window_id=glfwCreateWindow(videomode->width,videomode->height,
                        this->mytitle.c_str(),glfwGetPrimaryMonitor(),NULL);
        if ( !this->window_id)
        {
            glfwTerminate();
            cout <<" glfwterminate fullscreen" <<endl;
            exit(EXIT_FAILURE);
        }
        int xpos,ypos;
        glfwGetWindowPos(this->window_id, &xpos, &ypos);          
      } else {
          glfwSetWindowPos(this->window_id,this->width->getValue(), this->width->getValue());
          glfwSetWindowPos(this->window_id,posX->getValue(), posY->getValue());         
      }
      glfwMakeContextCurrent(this->window_id);
      last_fullscreen = fullscreen;
    }
}

void GLFWWindow::swapBuffers() {
  //glfwSwapBuffers(this->window_id);
}

void GLFWWindow::glfwKeyboardCallaback(GLFWwindow * gwindow,int key,int scancode,int action,int mods)
{
    GLFWWindow *window = GLFWWindow::getGLFWWindow(gwindow);
    int propagatedkey=key;
    bool special=true;
    switch( key ) 
    {
      case GLFW_KEY_F1: propagatedkey=X3DKeyDeviceSensorNode::F1;break;
      case GLFW_KEY_F2: propagatedkey=X3DKeyDeviceSensorNode::F2;break;
      case GLFW_KEY_F3: propagatedkey=X3DKeyDeviceSensorNode::F3;break;
      case GLFW_KEY_F4: propagatedkey=X3DKeyDeviceSensorNode::F4;break;
      case GLFW_KEY_F5: propagatedkey=X3DKeyDeviceSensorNode::F5;break;
      case GLFW_KEY_F6: propagatedkey=X3DKeyDeviceSensorNode::F6;break;
      case GLFW_KEY_F7: propagatedkey=X3DKeyDeviceSensorNode::F7;break;
      case GLFW_KEY_F8: propagatedkey=X3DKeyDeviceSensorNode::F8;break;
      case GLFW_KEY_F9: propagatedkey=X3DKeyDeviceSensorNode::F9;break;
      case GLFW_KEY_F10: propagatedkey=X3DKeyDeviceSensorNode::F10;break;
      case GLFW_KEY_F11: propagatedkey=X3DKeyDeviceSensorNode::F11;break;
      case GLFW_KEY_F12: propagatedkey=X3DKeyDeviceSensorNode::F12;break;
      case GLFW_KEY_HOME: propagatedkey=X3DKeyDeviceSensorNode::HOME;break;
      case GLFW_KEY_END: propagatedkey=X3DKeyDeviceSensorNode::END;break;
      case GLFW_KEY_PAGE_UP: propagatedkey=X3DKeyDeviceSensorNode::PGUP;break;
      case GLFW_KEY_PAGE_DOWN: propagatedkey=X3DKeyDeviceSensorNode::PGDN;break;
      case GLFW_KEY_UP: propagatedkey=X3DKeyDeviceSensorNode::UP;break;
      case GLFW_KEY_DOWN: propagatedkey=X3DKeyDeviceSensorNode::DOWN;break;
      case GLFW_KEY_LEFT: propagatedkey=X3DKeyDeviceSensorNode::LEFT;break;
      case GLFW_KEY_RIGHT: propagatedkey=X3DKeyDeviceSensorNode::RIGHT;break;
      default: { special=false;}
    }
    if(window)
    {
        if(action == GLFW_PRESS)
        {
            window->onKeyDown( propagatedkey, special );
        }
        else if(action == GLFW_RELEASE)
        {
             window->onKeyUp( propagatedkey, special );
        }
    }
}

void GLFWWindow::glfwMouseCallback(GLFWwindow * gwindow,int button,int action, int mods ) {
  GLFWWindow *window = GLFWWindow::getGLFWWindow( gwindow );
  if( window ) {
    switch( button ) {
      case GLFW_MOUSE_BUTTON_LEFT:
        window->onMouseButtonAction( MouseSensor::LEFT_BUTTON, 
                                     action == GLFW_PRESS ? 
                                     MouseSensor::DOWN : MouseSensor::UP );
        break;
      case GLFW_MOUSE_BUTTON_MIDDLE:
        window->onMouseButtonAction( MouseSensor::MIDDLE_BUTTON,
                                     action == GLFW_PRESS ?
                                     MouseSensor::DOWN : MouseSensor::UP );
        break;
      case GLFW_MOUSE_BUTTON_RIGHT:
        window->onMouseButtonAction( MouseSensor::RIGHT_BUTTON,
                                     action == GLFW_PRESS ?
                                     MouseSensor::DOWN : MouseSensor::UP );        
        break;
      default: {}
    }
  }
}
    
void GLFWWindow::glfwMotionCallback(GLFWwindow * gwindow, double xpos, double ypos ) 
{
  GLFWWindow *window = GLFWWindow::getGLFWWindow( gwindow );
  if( window ) {
      window->onMouseMotionAction( (int) round(xpos), (int) round(ypos) );
  }
}


void GLFWWindow::glfwMouseWheelCallback(GLFWwindow * gwindow, double xoffset, double yoffset )
{
   GLFWWindow *window = GLFWWindow::getGLFWWindow( gwindow );  
   if( window ) {
    if( yoffset > 0 )
      window->onMouseWheelAction( MouseSensor::FROM );
    else
      window->onMouseWheelAction( MouseSensor::TOWARDS );
  }
}


void GLFWWindow::loopMe()
{
    if(!this->window_id) return;
    while (!glfwWindowShouldClose(this->window_id))
    {
        /* Render here */
        for( set< Scene * >::iterator i = Scene::scenes.begin();
                i != Scene::scenes.end();++i )
        {
            if( (*i)->isActive() )  (*i)->idle();
        }
        glfwSwapBuffers(this->window_id);
        /* Poll for and process events */
        glfwPollEvents();
    }
}

void GLFWWindow::rendonstream(wqueue<WorkItem*>* frame_queue)
{
    if(!this->window_id) return;
    //cout << "rendonstream " << endl;    
    //cout << "swapped " << endl;
    for( set< Scene * >::iterator i = Scene::scenes.begin();
                        i != Scene::scenes.end();++i )
                {
                    if( (*i)->isActive() )  (*i)->idle();
                }
    glfwSwapBuffers(this->window_id);
    glfwPollEvents();
    if(frame_queue->size()==0)
    {        
        //cout << "rendonstream " << endl;                  
        //cout << "rend1" << endl;
        int tempwidth=this->width->getValue();
        int tempheight=this->height->getValue();
        //cout << "steal start" << endl;
        gpointer buff=this->stealFB();
        //cout << "steal end" << endl;
        int bound=this->height->getValue() / 2;
        int len=3*this->width->getValue();
        gpointer src=buff;
        gpointer dst=buff+(this->height->getValue()-1)*len;        
        //cout << "rend2" << endl;
        //cout << "len " << len << endl;
        //cout << "bound " << bound << endl;
        for(int i=0;i<bound;i++)
        {
            memcpy(this->xchgbuffer,src,len);
            memcpy(src,dst,len);
            memcpy(dst,this->xchgbuffer,len);    
            src+=len;
            dst-=len;
        }
        //cout << "rend3" << endl;
        WorkItem* item = new WorkItem(buff,3 * tempwidth * tempheight,tempwidth,tempheight);
        frame_queue->add(item);
        //cout << "rend4" << endl;        
    }
}