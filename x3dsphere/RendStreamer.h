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
#ifndef REND_STREAMER_H
#define REND_STREAMER_H

#include <gio/gio.h>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include "workitem.h"
#include "wqueue.h"
#include "zthread.h"
#include "promorend.h"


#include<iostream>

using namespace std;
class RendStreamer  : public ZThread{
public :
     GMainLoop *loop;
     wqueue<WorkItem*>* frame_queue;
     GSocketService *service;


     public :
        promoconf *pconf;
        RendStreamer(promoconf *myconf,wqueue<WorkItem*>* myqueue);
        ~RendStreamer();
        void pause(){
            g_socket_service_stop (this->service);
            g_socket_listener_close(G_SOCKET_LISTENER (service));
        }
        void resume(){
            g_socket_service_start (this->service);            
        }
        void* run();

        friend void cb_need_data (GstElement *appsrc,guint unused_size,RendStreamer *  rendo);
};

void cb_need_data (GstElement *appsrc,guint unused_size,RendStreamer *  rendo);
#endif // REND_STREAMER_H
