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
#include "RendLoad.h"
#include <H3D/X3D.h>
#include <H3D/Scene.h>
#include <H3D/MouseSensor.h>
#include<boost/tokenizer.hpp>
using namespace H3D;
using namespace boost;

RendLoad::RendLoad(promoconf * some_conf)
{
    pthread_mutex_init(&this->m_mutex, NULL);    
    this->render_conf=some_conf;
    this->request=NULL;
    this->response=NULL;
    this->commanded=false;
    this->initialized=false;
}



ipromotion_task_response * RendLoad::addtask(ipromotion_task_request * task)
{
    bool canprocess=false;
    do
    {
        pthread_mutex_lock(&this->m_mutex);
        canprocess=this->initialized;
        pthread_mutex_unlock(&this->m_mutex);
        if(!canprocess)
        {
            Sleep(100);
        }
    }while(!canprocess);
    
    bool cancommand=true;
    
    do
    {
        pthread_mutex_lock(&this->m_mutex);
        cancommand=!this->commanded;
        if(cancommand){
            this->commanded=true;
        }
        pthread_mutex_unlock(&this->m_mutex);
        if(!cancommand)
        {
            Sleep(100);
        }
    }while(!cancommand);
    
    
    pthread_mutex_lock(&this->m_mutex);
    if(this->request!=NULL)
    {
        cout << "error concurrent sending request" <<endl ;
        exit(-1);
    }
    
    if(this->response!=NULL)
    {
        cout << "error concurrent receiving response" <<endl ;
        exit(-1);
    }    
    this->request=task;
    pthread_mutex_unlock(&this->m_mutex);
    
    ipromotion_task_response * resp=NULL;;
    do
    {
        pthread_mutex_lock(&this->m_mutex);
        resp=this->response;
        if(resp!=NULL)        
        {
            this->response=NULL;
            if(this->request!=NULL)
            {
                cout << "error concurrent deleting request" <<endl ;
                exit(-1);
            }
        }                
        pthread_mutex_unlock(&this->m_mutex);      
        if(resp==NULL)
        {
            Sleep(100);
        }
    }while(resp==NULL);
    
    pthread_mutex_lock(&this->m_mutex);
    this->commanded=false;        
    pthread_mutex_unlock(&this->m_mutex);
    return resp;
}

bool RendLoad::zoomintask(ipromotion_task_request * task){
    cout << "zoomin" << endl;
    if(!this->started) return false;
    this->renderer->some_offscreen->zoomin();
    return true;
}

bool RendLoad::mouseclicktask(ipromotion_task_request * task){
    cout <<"mouse click args "<< task->args << endl;
    if(!this->started) return false;
    string s=task->args;
    string temp;
    tokenizer<> tok(s);
    tokenizer<>::iterator beg=tok.begin();
    if(beg==tok.end()) return false;    
    string button_type=*beg;
    ++beg;
    if(beg==tok.end()) return false;
    temp=*beg;
    int x=atoi(temp.c_str());
    ++beg;
    if(beg==tok.end()) return false;
    temp=*beg;
    int y=atoi(temp.c_str());
    if ( (x <0) || (x >= this->renderer->some_offscreen->width->getValue() )){
        return false;
    }
    if ( (y <0) || (y >= this->renderer->some_offscreen->height->getValue() ) ){
        return false;
    }
    cout << "x:" << x << " " <<"y:" <<" "<<y <<endl;
    
    this->renderer->some_offscreen->onMouseMotionAction( x, y);
    if(button_type=="LEFTUP") {
        cout << "LEFTUP" << endl;
        this->renderer->some_offscreen->onMouseButtonAction(MouseSensor::LEFT_BUTTON,MouseSensor::UP );
    } else if(button_type=="LEFTDOWN")  {
        cout << "LEFTDOWN" << endl;
        this->renderer->some_offscreen->onMouseButtonAction(MouseSensor::LEFT_BUTTON,MouseSensor::DOWN );
    } else if(button_type=="MIDDLEUP")     {
        cout << "MIDDLEUP" << endl;
        this->renderer->some_offscreen->onMouseButtonAction(MouseSensor::MIDDLE_BUTTON,MouseSensor::UP );
    } else if(button_type=="MIDDLEDOWN")     {
        cout << "MIDDLEDOWN" << endl;
        this->renderer->some_offscreen->onMouseButtonAction(MouseSensor::MIDDLE_BUTTON,MouseSensor::DOWN );
    } else if(button_type=="RIGHTUP")     {
        cout << "RIGHTUP" << endl;
        this->renderer->some_offscreen->onMouseButtonAction(MouseSensor::RIGHT_BUTTON,MouseSensor::UP );
    } else if(button_type=="RIGHTDOWN")     {
        cout << "RIGHTDOWN" << endl;
        this->renderer->some_offscreen->onMouseButtonAction(MouseSensor::RIGHT_BUTTON,MouseSensor::DOWN );
    } else {
        return false;
    }
    return true;
}

bool RendLoad::zoomouttask(ipromotion_task_request * task){
    cout << "zoomout" << endl;
    if(!this->started) return false;
    this->renderer->some_offscreen->zoomout();
    return true;
}

bool RendLoad::pausetask(ipromotion_task_request * task){
    if(!this->started) return false;
    //delete renderer->some_streamer;    
    //renderer->some_streamer=NULL;
    renderer->some_streamer->pause();
    this->started=false;
    return true;
}

bool RendLoad::resumetask(ipromotion_task_request * task){
    cout << "resumption" << endl;
    if(this->started) return false;
    if(renderer->some_frame_queue->size()!=0)
    {
        WorkItem* item=renderer->some_frame_queue->remove();
        delete item;
    } 
    //renderer->some_streamer=new RendStreamer(this->render_conf,renderer->some_frame_queue); 
    //renderer->some_streamer->start();
    renderer->some_streamer->resume();
    this->started=true;
    return true;
}

void RendLoad::stoptask(ipromotion_task_request * task){
    if(this->started)
    {
        delete renderer->some_streamer;
    }
    delete renderer->some_scene;
    promoconf_destroy(this->render_conf);
    
    if(renderer->some_frame_queue->size()!=0)
    {
        WorkItem* item=renderer->some_frame_queue->remove();
        delete item;
    }    
    delete renderer;
}

bool RendLoad::setviewpointtask(ipromotion_task_request * task){
    if(!this->started) return false;
    
    string ss( (char *) task->args);
    string vpnum=ss.erase(0, ss.find(":") + 1);
    
    int num=atoi(vpnum.c_str());    
    if( (num >= 0) && (num< this->viewpoints) )
    {
        this->current_viewpoint=num;
        X3DViewpointNode::ViewpointList vp_list = X3DViewpointNode::getAllViewpoints();
        int index=-1;
        for(ListIterator iter=vp_list.begin();iter!=vp_list.end();iter++)
        {
            index++;
            if(index==this->current_viewpoint)
            {
                (*iter)->set_bind->setValue(true);
                break;
            }            
        }
    }
    else
    {
        return false;
    }
    return true;
}

int RendLoad::getviewpointtask(ipromotion_task_request * task){
    return this->current_viewpoint;
}

string RendLoad::listviewpointstask(ipromotion_task_request * task)
{
    string s;
    if(this->viewpoints>0)
    {
        pthread_mutex_lock(&this->m_mutex);
        s=this->tviewpoints[0];
        for(int i=1;i<this->viewpoints;i++)
        {
            s+=";"+this->tviewpoints[i]; 
        }
         pthread_mutex_unlock(&this->m_mutex);
    }
    return s;
}

void* RendLoad::run()
{    
    this->renderer = new ipromotion_renderer;
    this->renderer->some_scene=new Scene;
    this->renderer->some_offscreen=new GLFWWindow;
    this->renderer->some_offscreen->setSize(this->render_conf->width,this->render_conf->height);
    this->renderer->some_frame_queue=new  wqueue<WorkItem*>();
    this->renderer->some_streamer=new RendStreamer(this->render_conf,renderer->some_frame_queue); 
    this->renderer->some_streamer->start();
    this->renderer->some_scene->window->push_back(renderer->some_offscreen );
    string file(this->render_conf->scenefile);  
    this->renderer->some_scene->loadSceneRoot( file ); 
    this->started=true; 
    cout << "GOON" << endl;          
    X3DViewpointNode::ViewpointList vp_list = X3DViewpointNode::getAllViewpoints();
    this->viewpoints=vp_list.size();
    cout << "VIEWPOINTS NUM" << viewpoints ;
    X3DViewpointNode *active_vp = X3DViewpointNode::getActive();    
    this->current_viewpoint=-1;    
    ListIterator it = find(vp_list.begin(),vp_list.end(),active_vp);    
    for(ListIterator iter=vp_list.begin();iter!=vp_list.end();iter++)
    {
        this->current_viewpoint++;
        if(iter==it)
        {
            break;
        }
    }
    this->tviewpoints=NULL;
    if(this->viewpoints>0)
    {
        this->tviewpoints=new string[this->viewpoints];
        int index=0;
        for(ListIterator iter=vp_list.begin();iter!=vp_list.end();iter++)
        {
            cout << "VIEPOINT" << (*iter)->description->getValue();
            this->tviewpoints[index++]=(*iter)->description->getValue();
        }
    }
    pthread_mutex_lock(&this->m_mutex);
    this->initialized=true;  
    pthread_mutex_unlock(&this->m_mutex);
    
    
    ipromotion_task_request * task=NULL;;
    string go="ok";
    bool status=true;
    while(true)
    {
        
        if(this->started)
        {
            this->renderer->some_offscreen->rendonstream(this->renderer->some_frame_queue);
        }
        
        pthread_mutex_lock(&this->m_mutex);
        task=this->request;
        pthread_mutex_unlock(&this->m_mutex); 
        
        go="ok";
        status=true;
        if(task!=NULL)
        {   
            if(strcmp(task->cmd,"stop") ==0)  { 
                this->stoptask(task);                
            } else if(strcmp(task->cmd,"pause") ==0) {
                if(!this->pausetask(task))
                {
                    status=false;
                    go="already paused";
                }
            }else if(strcmp(task->cmd,"zoomin") ==0) {
                if(!this->zoomintask(task))
                {
                    status=false;
                    go="cannot zoomin paused";
                }
            } else if(strcmp(task->cmd,"zoomout") ==0) {
                if(!this->zoomouttask(task))
                {
                    status=false;
                    go="cannot zoomout paused";
                }
            } else if(strcmp(task->cmd,"resume") ==0) {
                if(!this->resumetask(task))
                {
                    status=false;
                    go="already running";
                }
            } else if(strcmp(task->cmd,"setviewpoint") ==0) {
                if(!this->setviewpointtask(task))
                {
                    status=false;
                    go="could not set viewpoint";
                }
            } else if(strcmp(task->cmd,"mouseclick") ==0) {
                if(!this->mouseclicktask(task))
                {
                    status=false;
                    go="could not click mouse";
                }            
            } else if(strcmp(task->cmd,"getviewpoint") ==0) {
                char *buf=new char [100];
                go=itoa(this->getviewpointtask(task),buf,10);
                delete buf;
            } else if(strcmp(task->cmd,"listviewpoints") ==0) {
                go=this->listviewpointstask(task);                                 
            } else {
                status=false;
                go="invalid command";
            }
            ipromotion_task_response * myresponse=new ipromotion_task_response;
            myresponse->mesg=new char[strlen(go.c_str()+1)];
            strcpy(myresponse->mesg,go.c_str());
            if(status){
                myresponse->result=1;
            }
            else {
                myresponse->result=-1;
            }
            pthread_mutex_lock(&this->m_mutex);            
            this->request=NULL;
            this->response=myresponse;
            pthread_mutex_unlock(&this->m_mutex);            
            if(strcmp(task->cmd,"stop") ==0)
            {
                delete task->cmd;
                delete task;
                cout << "termination started t1000 " << endl;
                return NULL;
            }
            else
            {
                delete task->cmd;
                delete task;
            }
        }
        Sleep(10);
    }    
}


RendLoad::~RendLoad()
{
    this->join();
    cout << "termination continued t1000 " << endl;
    pthread_mutex_destroy(&this->m_mutex);  
    delete this->renderer->some_streamer;
    delete this->renderer->some_scene;
    delete this->renderer->some_offscreen;
    delete this->renderer->some_message_queue;
    delete this->renderer->some_frame_queue;
    delete this->renderer;
}
