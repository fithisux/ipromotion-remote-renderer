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
#include "promorend.h"
#include "RendLoad.h"
#include "RendStreamer.h"
#include "wqueue.h"
#include "zthread.h"
#include<cstring>
#include<iostream>
#include<sstream>
#include <stdio.h>


struct promoconf * PROMOAPI promoconf_create(int width,int height,unsigned int port,char * scenefile)
{
    struct promoconf *x=new struct promoconf;
    x->width=width;
    x->height=height;
    x->port=port;
    if(scenefile)
    {
        x->scenefile=new char[strlen(scenefile)+1];
        memcpy(x->scenefile,scenefile,strlen(scenefile)+1);
    }
    else
    {
        x->scenefile=NULL;
    }

    printf("go1\n");
    return x;

}
void PROMOAPI promoconf_destroy(struct promoconf *pconf)
{
    printf("go2\n");
    delete pconf->scenefile;
    delete pconf;    
}

struct promorend * PROMOAPI promorend_create(struct promoconf *pconf)
{
    cout << "f1" << endl;
    struct promorend *x=new struct promorend;
    x->ipromotion_system=new RendLoad(pconf);
    ((RendLoad *) x->ipromotion_system)->start();    
    return x;
}


void PROMOAPI promorend_destroy(struct promorend *promo)
{         
    delete promo->ipromotion_system;    
    promoconf_destroy(promo->pconf);
    delete promo;
}

void PROMOAPI response_destroy(struct ipromotion_response* response)
{
    delete response->mesg;
    delete response;
}

char * PROMOAPI response_mesg(struct ipromotion_response* response)
{
    return response->mesg;
}

int PROMOAPI response_status(struct ipromotion_response* response)
{
    return response->result;
}

struct ipromotion_response * PROMOAPI promorend_instruct(struct promorend *active,const char * command)
{
    return promorend_instruct_args(active,command,NULL);
}

struct ipromotion_response *PROMOAPI promorend_instruct_args(struct promorend *promo,
        const char * command,
        const char * args)
{
    ipromotion_task_request *request=new ipromotion_task_request;
    request->args=NULL;
    request->cmd=new char[strlen(command)+1];
    strcpy(request->cmd,command); 
    if( args != NULL)
    {
        request->args=new char[strlen(args)+1];
        strcpy(request->args,args); 
    }
    return ((RendLoad *) promo->ipromotion_system)->addtask(request);
}

