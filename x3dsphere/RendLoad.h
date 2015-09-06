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
/* 
 * File:   RendLoad.h
 * Author: vanag
 *
 * Created on July 28, 2014, 3:31 PM
 */

#ifndef RENDLOAD_H
#define	RENDLOAD_H
#include <pthread.h>
#include "promorend.h"
#include "zthread.h"
#include<iostream>
#include "RendStreamer.h"
#include "GLFWWindow.h"
#include <H3D/H3DWindowNode.h>
#include <H3D/Scene.h>
#include <H3D/Viewpoint.h>
#include <H3D/DEFNodes.h>
using namespace std;
using namespace H3D;

typedef X3DViewpointNode::ViewpointList::iterator ListIterator;

typedef struct {
    GLFWWindow* some_offscreen;
    RendStreamer* some_streamer; 
    Scene * some_scene;
    wqueue<WorkItem*>* some_frame_queue;
    wqueue<string>* some_message_queue;     
}ipromotion_renderer  ;

class RendLoad  : public ZThread{
public :
        pthread_mutex_t  m_mutex;  
        bool started;
        bool commanded;
        bool initialized;
        int viewpoints=-1;
        int current_viewpoint=-1;
        string *tviewpoints;
        bool pausetask(ipromotion_task_request * task);
        bool resumetask(ipromotion_task_request * task);        
        void stoptask(ipromotion_task_request * task);
        bool zoomintask(ipromotion_task_request * task);        
        bool zoomouttask(ipromotion_task_request * task);        
        bool setviewpointtask(ipromotion_task_request * task);
        int getviewpointtask(ipromotion_task_request * task);
        bool mouseclicktask(ipromotion_task_request * task);
        string listviewpointstask(ipromotion_task_request * task);
        ipromotion_task_request *request;
        ipromotion_renderer * renderer;
        ipromotion_task_response * response; 
        ipromotion_task_response * addtask(ipromotion_task_request * task);       
        RendLoad(promoconf * some_conf);
        ~RendLoad();
        void* run();
        promoconf * render_conf;
};


#endif	/* RENDLOAD_H */

