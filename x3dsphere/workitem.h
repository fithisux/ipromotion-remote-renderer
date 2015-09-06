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
#include <gst/gst.h>
#include <stdio.h>
#include <string.h>
#ifndef WORK_ITEM_H
#define WORK_ITEM_H


class WorkItem
{
public :
    gpointer buffer;
    guint bytes;
    int width;
    int height;
    WorkItem(gpointer bbuffer,guint bbytes,int mywidth,int myheight) :  width(mywidth), height(myheight), buffer(bbuffer), bytes(bbytes){}
    ~WorkItem()
    {
        if(this->buffer) g_free(this->buffer);
    }
};

#endif // WORK_ITEM_H
