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
package main

/*
#cgo CPPFLAGS: -IC:/langs/ipromotion/x3dsphere
#cgo LDFLAGS: -LC:/langs/ipromotion/x3dsphere/dist/Release/MinGW_1-Windows -IC:/langs/ipromotion/x3dsphere -lx3dsphere
#include <stdio.h>
#include <stdlib.h>
#include "promorend.h"

struct promoconf * PROMOAPI promoconf_create(int width,int height,unsigned int port,char * scenefile);
void PROMOAPI promoconf_destroy(struct promoconf *pconf);
struct promorend * PROMOAPI promorend_create(struct promoconf *pconf);
void PROMOAPI promorend_destroy(struct promorend* rend);
struct ipromotion_response * PROMOAPI promorend_instruct_args(struct promorend *active,const char * command,const char * args);
struct ipromotion_response * PROMOAPI promorend_instruct(struct promorend *active,const char * command);
void PROMOAPI response_destroy(struct ipromotion_response* response);
char * PROMOAPI response_mesg(struct ipromotion_response* response);
int PROMOAPI response_status(struct ipromotion_response* response);
*/
import "C"

import (
	"flag"
	"fmt"
	"github.com/emicklei/go-restful"
	"log"
	"net/http"
	"os"
	"strconv"
	"sync"
	"unsafe"
)

var mutex = &sync.Mutex{}

func promoconf_create(width, height uint, port uint, scenefile string) *C.struct_promoconf {
	cs := C.CString(scenefile)
	var pconf *C.struct_promoconf
	pconf = C.promoconf_create(C.int(width), C.int(height), C.uint(port), cs)
	C.free(unsafe.Pointer(cs))
	return pconf
}

func promoconf_destroy(pconf *C.struct_promoconf) {
	C.promoconf_destroy(pconf)
}

func promorend_create(pconf *C.struct_promoconf) *C.struct_promorend {
	prend := C.promorend_create(pconf)
	return prend
}

func promorend_instruct_args(promo *C.struct_promorend, cmd string, cmdargs string) *C.struct_ipromotion_response {
	c1 := C.CString(cmd)
	c2 := C.CString(cmdargs)
	resp := C.promorend_instruct_args(promo, c1, c2)
	C.free(unsafe.Pointer(c1))
	C.free(unsafe.Pointer(c2))
	return resp
}

func promorend_instruct(promo *C.struct_promorend, cmd string) *C.struct_ipromotion_response {
	c1 := C.CString(cmd)
	resp := C.promorend_instruct(promo, c1)
	C.free(unsafe.Pointer(c1))
	return resp
}

func promorend_destroy(promo *C.struct_promorend) {
	C.promorend_destroy(promo)
}

func response_destroy(presp *C.struct_ipromotion_response) {
	C.response_destroy(presp)
}

func response_status(presp *C.struct_ipromotion_response) int {
	return int(C.response_status(presp))
}

func response_mesg(presp *C.struct_ipromotion_response) string {
	mesg := C.response_mesg(presp)
	return C.GoString(mesg)
}

var flag_width uint

func init() {
	flag.UintVar(&flag_width, "width", 0, "the width of the frame")
}

var flag_height uint

func init() {
	flag.UintVar(&flag_height, "height", 0, "the height of the frame")
}

var flag_sport uint

func init() {
	flag.UintVar(&flag_sport, "streamport", 0, "the port to connect to the player")
}

var flag_lport uint

func init() {
	flag.UintVar(&flag_lport, "listenerport", 0, "the port to connect to the player")
}

var flag_file string

func init() {
	flag.StringVar(&flag_file, "x3dfile", "", "the file to play")
}

type PlayerResp struct {
	Status bool
	Mesg   string
}

type Forwarder struct {
	Cmdargs string
}

type Coaction struct {
	Cmd     string
	Cmdargs string
}

var coactions chan Coaction
var responses chan PlayerResp

var system_of_ipromotion *C.struct_promorend
var identifier uint

func coordfcn(a chan Coaction, b chan PlayerResp) {
	fmt.Println("command listener started xxx")
	fmt.Println("In for loop")
	for x := range a {
		fmt.Println("renderer ", identifier, " received order =  ", x)
		switch x.Cmd {
		case "pause":
			{
				resp := promorend_instruct(system_of_ipromotion, "pause")
				temp_status := response_status(resp)
				temp_mesg := response_mesg(resp)
				if temp_status == 1 {
					b <- PlayerResp{true, temp_mesg}
				} else {
					b <- PlayerResp{false, temp_mesg}
				}
				response_destroy(resp)

			}
		case "zoomin":
			{
				resp := promorend_instruct(system_of_ipromotion, "zoomin")
				temp_status := response_status(resp)
				temp_mesg := response_mesg(resp)
				if temp_status == 1 {
					b <- PlayerResp{true, temp_mesg}
				} else {
					b <- PlayerResp{false, temp_mesg}
				}
				response_destroy(resp)

			}
		case "zoomout":
			{
				resp := promorend_instruct(system_of_ipromotion, "zoomout")
				temp_status := response_status(resp)
				temp_mesg := response_mesg(resp)
				if temp_status == 1 {
					b <- PlayerResp{true, temp_mesg}
				} else {
					b <- PlayerResp{false, temp_mesg}
				}
				response_destroy(resp)

			}
		case "listviewpoints":
			{
				resp := promorend_instruct(system_of_ipromotion, "listviewpoints")
				temp_status := response_status(resp)
				temp_mesg := response_mesg(resp)
				if temp_status == 1 {
					b <- PlayerResp{true, temp_mesg}
				} else {
					b <- PlayerResp{false, temp_mesg}
				}
				fmt.Println("pre destroy")
				response_destroy(resp)
				fmt.Println("post destroy")

			}
		case "getviewpoint":
			{
				resp := promorend_instruct(system_of_ipromotion, "getviewpoint")
				temp_status := response_status(resp)
				temp_mesg := response_mesg(resp)
				if temp_status == 1 {
					b <- PlayerResp{true, temp_mesg}
				} else {
					b <- PlayerResp{false, temp_mesg}
				}
				response_destroy(resp)

			}
		case "setviewpoint":
			{
				resp := promorend_instruct_args(system_of_ipromotion, "setviewpoint", x.Cmdargs)
				temp_status := response_status(resp)
				temp_mesg := response_mesg(resp)
				if temp_status == 1 {
					b <- PlayerResp{true, temp_mesg}
				} else {
					b <- PlayerResp{false, temp_mesg}
				}
				response_destroy(resp)

			}
		case "mouseclick":
			{
				resp := promorend_instruct_args(system_of_ipromotion, "mouseclick", x.Cmdargs)
				temp_status := response_status(resp)
				temp_mesg := response_mesg(resp)
				if temp_status == 1 {
					b <- PlayerResp{true, temp_mesg}
				} else {
					b <- PlayerResp{false, temp_mesg}
				}
				response_destroy(resp)

			}
		case "resume":
			{
				resp := promorend_instruct(system_of_ipromotion, "resume")
				temp_status := response_status(resp)
				temp_mesg := response_mesg(resp)
				if temp_status == 1 {
					b <- PlayerResp{true, temp_mesg}
				} else {
					b <- PlayerResp{false, temp_mesg}
				}
				response_destroy(resp)
			}
		default: //command
			{
				b <- PlayerResp{false, "ignored"}
			}
		}
	}
}

func promo_pause(request *restful.Request, response *restful.Response) { //pause a stream
	fmt.Println("Inside pause ", identifier)
	coactions <- Coaction{"pause", ""}
	p := <-responses
	response.WriteEntity(p)
}

func promo_zoomin(request *restful.Request, response *restful.Response) { //pause a stream
	fmt.Println("Inside zoomin ", identifier)
	coactions <- Coaction{"zoomin", ""}
	p := <-responses
	response.WriteEntity(p)
}

func promo_zoomout(request *restful.Request, response *restful.Response) { //pause a stream
	fmt.Println("Inside zoomout ", identifier)
	coactions <- Coaction{"zoomout", ""}
	p := <-responses
	response.WriteEntity(p)
}

func promo_resume(request *restful.Request, response *restful.Response) { //resume a stream
	fmt.Println("Inside resume ", identifier)
	coactions <- Coaction{"resume", ""}
	p := <-responses
	response.WriteEntity(p)
}

func promo_viewpoints(request *restful.Request, response *restful.Response) { //stop a stream
	fmt.Println("Inside viewpoints ", identifier)
	coactions <- Coaction{"listviewpoints", ""}
	p := <-responses
	response.WriteEntity(p)
}

func promo_getviewpoint(request *restful.Request, response *restful.Response) { //stop a stream
	fmt.Println("Inside getviewpoint ", identifier)
	mutex.Lock()
	coactions <- Coaction{"getviewpoint", ""}
	p := <-responses
	response.WriteEntity(p)
}

func promo_setviewpoint(request *restful.Request, response *restful.Response) { //stop a stream
	fmt.Println("Inside setviewpoint ", identifier)
	forwarder := new(Forwarder)
	err := request.ReadEntity(forwarder)
	if err != nil {
		response.AddHeader("Content-Type", "text/plain")
		response.WriteErrorString(http.StatusInternalServerError, err.Error())
		return
	}
	coactions <- Coaction{"setviewpoint", forwarder.Cmdargs}
	p := <-responses
	response.WriteEntity(p)
}

func promo_mouseclick(request *restful.Request, response *restful.Response) { //stop a stream
	fmt.Println("Inside mouseclick ", identifier)
	forwarder := new(Forwarder)
	err := request.ReadEntity(forwarder)
	if err != nil {
		response.AddHeader("Content-Type", "text/plain")
		response.WriteErrorString(http.StatusInternalServerError, err.Error())
		return
	}
	coactions <- Coaction{"mouseclick", forwarder.Cmdargs}
	p := <-responses
	response.WriteEntity(p)
}

func main() {

	flag.Parse()

	fmt.Println("Given start")
	fmt.Println("flag_file " + flag_file)
	fmt.Println("flag_width " + strconv.Itoa(int(flag_width)))
	fmt.Println("flag_height " + strconv.Itoa(int(flag_height)))
	fmt.Println("flag_sport " + strconv.Itoa(int(flag_sport)))
	fmt.Println("flag_lport " + strconv.Itoa(int(flag_lport)))
	fmt.Println("Given stop")

	if flag_file == "" {
		fmt.Println("File cannot be nil")
		os.Exit(-1)
	}

	if _, err := os.Stat(flag_file); os.IsNotExist(err) {
		fmt.Printf("no such file or directory: %s", flag_file)
		os.Exit(-1)
	}

	if flag_width == 0 {
		fmt.Println("width must be positive")
		os.Exit(-1)
	}
	if flag_height == 0 {
		fmt.Println("height must be positive")
		os.Exit(-1)
	}
	if flag_sport < 8000 || flag_sport > 8099 {
		fmt.Println("streaming port must be between 8000 and 8099")
		os.Exit(-1)
	}

	if flag_lport < 8100 || flag_lport > 8199 {
		fmt.Println("listener port must be between 8100 and 8199")
		os.Exit(-1)
	}

	if flag_lport == flag_sport {
		fmt.Println("listener port must not be equalt to listener port")
		os.Exit(-1)
	}

	identifier = flag_sport
	system_conf := promoconf_create(flag_width, flag_height, flag_sport, flag_file)
	system_of_ipromotion = promorend_create(system_conf)
	responses = make(chan PlayerResp)
	coactions = make(chan Coaction)

	go coordfcn(coactions, responses)
	wsContainer := restful.NewContainer()

	fmt.Println("Registering")
	ws := new(restful.WebService)
	ws.Path("/player").Consumes(restful.MIME_JSON).Produces(restful.MIME_JSON)
	ws.Route(ws.POST("/pause").To(promo_pause)).Consumes(restful.MIME_JSON).Produces(restful.MIME_JSON)
	ws.Route(ws.POST("/resume").To(promo_resume)).Consumes(restful.MIME_JSON).Produces(restful.MIME_JSON)
	ws.Route(ws.POST("/zoomin").To(promo_zoomin)).Consumes(restful.MIME_JSON).Produces(restful.MIME_JSON)
	ws.Route(ws.POST("/zoomout").To(promo_zoomout)).Consumes(restful.MIME_JSON).Produces(restful.MIME_JSON)
	ws.Route(ws.POST("/viewpoints").To(promo_viewpoints)).Consumes(restful.MIME_JSON).Produces(restful.MIME_JSON)
	ws.Route(ws.POST("/viewpoint").To(promo_getviewpoint)).Consumes(restful.MIME_JSON).Produces(restful.MIME_JSON)
	ws.Route(ws.POST("/mouseclick").To(promo_mouseclick)).Consumes(restful.MIME_JSON).Produces(restful.MIME_JSON)
	ws.Route(ws.POST("/alterviewpoint").To(promo_setviewpoint)).Consumes(restful.MIME_JSON).Produces(restful.MIME_JSON)
	wsContainer.Add(ws)

	// Add container filter to enable CORS
	/*
		cors := restful.CrossOriginResourceSharing{
			ExposeHeaders:  []string{"X-My-Header"},
			AllowedHeaders: []string{"Content-Type"},
			CookiesAllowed: false,
			Container:      wsContainer}
		wsContainer.Filter(cors.Filter)

		// Add container filter to respond to OPTIONS
		wsContainer.Filter(wsContainer.OPTIONSFilter)
	*/

	log.Printf("start listening on localhost:" + strconv.Itoa(int(flag_lport)))
	server := &http.Server{Addr: ":" + strconv.Itoa(int(flag_lport)), Handler: wsContainer}
	log.Fatal(server.ListenAndServe())
}
