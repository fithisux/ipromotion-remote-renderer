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
#ifndef __PROMOREND_H__
#define __PROMOREND_H__


typedef struct  {
    char * cmd;
    char * args;
} ipromotion_task_request;

typedef struct ipromotion_response {
    char * mesg;
    int result;
} ipromotion_task_response;



struct promoconf {
    char * scenefile;
    unsigned int port;
    int width;
    int height;
} ;

struct promorend {
    struct promoconf *pconf;
    void * ipromotion_system;
} ;

#ifdef DLL_EXPORT
#define PROMOAPI __declspec(dllexport)
#else
#define PROMOAPI __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif
struct promoconf * PROMOAPI promoconf_create(int width,int height,unsigned int port,char * scenefile);
void PROMOAPI promoconf_destroy(struct promoconf *pconf);
struct promorend * PROMOAPI promorend_create(struct promoconf *pconf);
void PROMOAPI promorend_destroy(struct promorend* rend);
struct ipromotion_response * PROMOAPI promorend_instruct_args(struct promorend *active,const char * command,const char * args);
struct ipromotion_response * PROMOAPI promorend_instruct(struct promorend *active,const char * command);
void PROMOAPI response_destroy(struct ipromotion_response* response);
char * PROMOAPI response_mesg(struct ipromotion_response* response);
int PROMOAPI response_status(struct ipromotion_response* response);
#ifdef __cplusplus
}
#endif


#endif


