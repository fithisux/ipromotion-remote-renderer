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

import (
	//	"bufio"
	"archive/zip"
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"github.com/emicklei/go-restful"
	"github.com/jmcvetta/napping"
	"io"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
	"time"
)

const Maxrends = 100

func createSequent() int {
	length := len(streamlut)
	if length >= Maxrends {
		return -1
	}
	if length == 0 {
		return 0
	}
	p := make([]int, length)
	j := 0
	for key, _ := range streamlut {
		p[j] = key
		j = j + 1
	}
	sort.Ints(p)
	if p[0] > 0 {
		return 0
	}
	for i := 0; i < length-1; i++ {
		if p[i]+1 != p[i+1] {
			return p[i] + 1
		}
	}
	return p[length-1] + 1
}

func heartbeat(reviver chan Coaction, answers chan NodeResp) {
	time.AfterFunc(1*time.Minute, func() {
		reviver <- Coaction{"ping", "", -1}
		s := <-answers
		fmt.Println(s.Mesg)
		heartbeat(reviver, answers)
	})
}

func keepalivefcn(reviver chan Coaction, answers chan NodeResp) {
	for revival := range reviver {
		switch revival.Instruction {
		case "ping":
			var b bool = false
			var expirations string = "expired "
			for key, value := range streamlut {
				secs := time.Since(value.sampled).Seconds()
				if secs > 60 {
					destroyPlayer(key)
					expirations = expirations + " " + string(key)
					b = true
				}
			}
			answers <- NodeResp{b, expirations}
		case "create":
			seqresp := createSequent()
			if seqresp == -1 {
				answers <- NodeResp{false, "Exhausted"}
			}
			rparams := new(RenderParams)
			fmt.Println(revival.Cmdargs)
			json.Unmarshal(bytes.NewBufferString(revival.Cmdargs).Bytes(), rparams)
			answers <- createPlayer(seqresp, rparams)
		case "list":
			lista := ListUsers()
			textual, _ := json.Marshal(lista)
			answers <- NodeResp{true, string(textual)}
		case "stop":
			_, found := streamlut[revival.Id]
			if !found {
				answers <- NodeResp{false, "Not Found"}
			}
			destroyPlayer(revival.Id)
			answers <- NodeResp{true, "Killed " + string(revival.Id)}
		default:
			lutentry, found := streamlut[revival.Id]
			if found {
				lutentry.sampled = time.Now()
				instruct(&revival)
			} else {
			}
			answers <- NodeResp{false, "Not found"}
		}
	}
	fmt.Println("oops")
}

type NodeResp struct {
	Status bool
	Mesg   string
}

type RenderParams struct {
	Width    int
	Height   int
	X3dfile  string
	Username string
}

type RenderConf struct {
	Rparams   *RenderParams
	Sport     int
	Lport     int
	Zipfolder string
}

type PlayerConf struct {
	Rconf   *RenderConf
	pid     *os.Process
	sampled time.Time
}

type Coaction struct {
	Instruction string
	Cmdargs     string
	Id          int
}

type Forwarder struct {
	Cmdargs string
}

type X3DFiles struct {
	Ids   []int
	Users []string
}

var streamlut map[int]*PlayerConf
var my_reviver chan Coaction
var my_answers chan NodeResp

func instruct(a *Coaction) *NodeResp {
	fmt.Println("Instructed " + a.Instruction + " for id " + strconv.Itoa(a.Id))
	v, _ := streamlut[a.Id]
	url := "http://127.0.0.1:" + strconv.Itoa(v.Rconf.Lport) + "/player/" + a.Instruction
	fmt.Println("Instructed as " + url)
	var answer NodeResp
	e := struct {
		Message string
		Errors  []struct {
			Resource string
			Field    string
			Code     string
		}
	}{}
	s := napping.Session{}

	var forwarder *Forwarder = nil

	if (a.Instruction == "alterviewpoint") || (a.Instruction == "mouseclick") {
		forwarder = &Forwarder{a.Cmdargs}
	}
	resp, err := s.Post(url, forwarder, &answer, &e)

	if err != nil {
		return &NodeResp{false, err.Error()}
	}

	if resp.Status() != 200 {
		return &NodeResp{false, strconv.Itoa(resp.Status()) + "..." + e.Message}
	}

	return &answer
}

func createPlayer(Id int, rparams *RenderParams) NodeResp {

	if rparams.Width <= 0 {
		return NodeResp{false, "width must be positive"}
	}
	if rparams.Height <= 0 {
		return NodeResp{false, "height must be positive"}
	}

	x3dfolder, err := unzipfile(rparams.X3dfile)
	if err != nil {
		return NodeResp{false, "unzip failed:" + err.Error()}
	}

	localx3dfile := x3dfolder + "/*.x3d"

	matches, err := filepath.Glob(localx3dfile)
	if err != nil {
		return NodeResp{false, "x3d in zip failed:" + err.Error()}
	}

	//we have the ports
	rconf := &RenderConf{rparams, 8000 + Id, 8100 + Id, x3dfolder}

	//build up command
	sargs := []string{
		"progname",
		"-width",
		strconv.Itoa(rparams.Width),
		"-height",
		strconv.Itoa(rparams.Height),
		"-streamport",
		strconv.Itoa(rconf.Sport),
		"-listenerport",
		strconv.Itoa(rconf.Lport),
		"-x3dfile",
		matches[0]}

	fmt.Println(sargs)
	procAttr := new(os.ProcAttr)
	procAttr.Files = []*os.File{nil, os.Stdout, os.Stderr}
	p, err := os.StartProcess("c:\\langs\\ipromotion\\promorenderer\\promorenderer.exe", sargs, procAttr)
	if err != nil {
		return NodeResp{false, "start process failed:" + err.Error()}
	}
	fmt.Println("Process " + string(p.Pid) + " created")

	fmt.Println("started")
	streamlut[Id] = &PlayerConf{rconf, p, time.Now()}
	return NodeResp{true, strconv.Itoa(Id)}
}

func destroyPlayer(Id int) {

	v, found := streamlut[Id]
	if !found {
		fmt.Println("could not kill nonexistent " + string(Id))
		log.Fatal("general error")
		os.Exit(-1)
	}
	delete(streamlut, Id)
	fmt.Println("Process " + string(v.pid.Pid) + " to be killed")
	if err := v.pid.Kill(); err != nil {
		fmt.Println("failed to kill " + string(Id) + " " + err.Error())
		log.Fatal("general error")
		os.Exit(-1)
	}
	err := os.RemoveAll(v.Rconf.Zipfolder)
	if err != nil {
		fmt.Println("failed to remove " + v.Rconf.Zipfolder + " " + err.Error())
		log.Fatal("general error")
		os.Exit(-1)
	}

}

func ListUsers() *X3DFiles {
	lista := new(X3DFiles)
	length := len(streamlut)
	p1 := make([]int, length)
	p2 := make([]string, length)
	j := 0
	for key, value := range streamlut {
		p1[j] = key
		p2[j] = value.Rconf.Rparams.Username
		j = j + 1
	}
	lista.Ids = p1
	lista.Users = p2
	return lista
}

func promo_listusers(request *restful.Request, response *restful.Response) { //list files
	fmt.Println("Inside listusers")
	my_reviver <- Coaction{"list", "", -1}
	nresp := <-my_answers
	response.WriteEntity(nresp)
}

func promo_keepalive(request *restful.Request, response *restful.Response) { //resume a stream
	fmt.Println("Inside keepalive")
	id, xresp := recoverID(request)
	if xresp != nil {
		response.WriteEntity(xresp)
		return
	}
	my_reviver <- Coaction{"keepalive", "", id}
	nresp := <-my_answers
	response.WriteEntity(nresp)
}

func promo_pause(request *restful.Request, response *restful.Response) { //command a stream
	fmt.Println("Inside pause")
	id, xresp := recoverID(request)
	if xresp != nil {
		response.WriteEntity(xresp)
		return
	}
	my_reviver <- Coaction{"pause", "", id}
	nresp := <-my_answers
	response.WriteEntity(nresp)
}

func promo_resume(request *restful.Request, response *restful.Response) { //keepalive a stream
	fmt.Println("Inside resume")
	id, xresp := recoverID(request)
	if xresp != nil {
		response.WriteEntity(xresp)
		return
	}
	my_reviver <- Coaction{"resume", "", id}
	nresp := <-my_answers
	response.WriteEntity(nresp)
}

func promo_zoomin(request *restful.Request, response *restful.Response) { //keepalive a stream
	fmt.Println("Inside zoomin")
	id, xresp := recoverID(request)
	if xresp != nil {
		response.WriteEntity(xresp)
		return
	}
	my_reviver <- Coaction{"zoomin", "", id}
	nresp := <-my_answers
	response.WriteEntity(nresp)
}

func promo_zoomout(request *restful.Request, response *restful.Response) { //keepalive a stream
	fmt.Println("Inside zoomout")
	id, xresp := recoverID(request)
	if xresp != nil {
		response.WriteEntity(xresp)
		return
	}
	my_reviver <- Coaction{"zoomout", "", id}
	nresp := <-my_answers
	response.WriteEntity(nresp)
}

func promo_stop(request *restful.Request, response *restful.Response) { //keepalive a stream
	fmt.Println("Inside stop")
	id, xresp := recoverID(request)
	if xresp != nil {
		response.WriteEntity(xresp)
		return
	}
	my_reviver <- Coaction{"stop", "", id}
	nresp := <-my_answers
	response.WriteEntity(nresp)
}

func promo_viewpoints(request *restful.Request, response *restful.Response) { //keepalive a stream
	fmt.Println("Inside viewpoints")
	id, xresp := recoverID(request)
	if xresp != nil {
		response.WriteEntity(xresp)
		return
	}
	my_reviver <- Coaction{"viewpoints", "", id}
	nresp := <-my_answers
	response.WriteEntity(nresp)
}

func promo_getviewpoint(request *restful.Request, response *restful.Response) { //keepalive a stream
	fmt.Println("Inside getviewpoint")
	id, xresp := recoverID(request)
	if xresp != nil {
		response.WriteEntity(xresp)
		return
	}
	my_reviver <- Coaction{"viewpoint", "", id}
	nresp := <-my_answers
	response.WriteEntity(nresp)
}

type Viewpointset struct {
	Val string
}

func promo_setviewpoint(request *restful.Request, response *restful.Response) { //keepalive a stream
	fmt.Println("Inside setviewpoint")
	id, xresp := recoverID(request)
	if xresp != nil {
		response.WriteEntity(xresp)
		return
	}
	vpset := new(Viewpointset)
	err := request.ReadEntity(vpset)
	if err != nil {
		response.AddHeader("Content-Type", "text/plain")
		response.WriteErrorString(http.StatusInternalServerError, err.Error())
		return
	}
	my_reviver <- Coaction{"alterviewpoint", vpset.Val, id}
	nresp := <-my_answers
	response.WriteEntity(nresp)
}

type Mouseclicking struct {
	Mousetype string
	X         int
	Y         int
}

func promo_mouseclick(request *restful.Request, response *restful.Response) { //keepalive a stream
	fmt.Println("Inside mouseclick")
	id, xresp := recoverID(request)
	if xresp != nil {
		response.WriteEntity(xresp)
		return
	}
	mouseclick := new(Mouseclicking)
	err := request.ReadEntity(mouseclick)
	if err != nil {
		response.AddHeader("Content-Type", "text/plain")
		response.WriteErrorString(http.StatusInternalServerError, err.Error())
		return
	}
	val := mouseclick.Mousetype + " " + strconv.Itoa(mouseclick.X) + " " + strconv.Itoa(mouseclick.Y)
	my_reviver <- Coaction{"mouseclick", val, id}
	nresp := <-my_answers
	response.WriteEntity(nresp)
}

func promo_create(request *restful.Request, response *restful.Response) { //stop a stream
	fmt.Println("Inside create")
	rparams := new(RenderParams)
	err := request.ReadEntity(rparams)
	if err != nil {
		response.AddHeader("Content-Type", "text/plain")
		response.WriteErrorString(http.StatusInternalServerError, err.Error())
		return
	}
	val, _ := json.Marshal(rparams)
	my_reviver <- Coaction{"create", string(val), -1}
	nresp := <-my_answers
	response.WriteEntity(nresp)
}

func recoverID(request *restful.Request) (int, *NodeResp) {
	userids := request.PathParameter("user-id")
	userid, err := strconv.Atoi(userids)
	if err != nil {
		nresp := &NodeResp{false, "problematic user id"}
		return 0, nresp
	} else {
		return userid, nil
	}
}

func cloneZipItem(f *zip.File, dest string) error {
	// Create full directory path
	path := filepath.Join(dest, f.Name)
	fmt.Println("Creating", path)
	err := os.MkdirAll(filepath.Dir(path), os.ModeDir|os.ModePerm)
	if err != nil {
		return err
	}

	// Clone if item is a file
	rc, err := f.Open()
	if err != nil {
		return err
	}
	if !f.FileInfo().IsDir() {
		// Use os.Create() since Zip don't store file permissions.
		fileCopy, err := os.Create(path)
		if err != nil {
			return err
		}
		_, err = io.Copy(fileCopy, rc)
		fileCopy.Close()
		if err != nil {
			return err
		}
	}
	rc.Close()
	return nil
}

func unzipfile(x3durl string) (string, error) {
	myname, err := ioutil.TempDir("", "ipromotion")
	if err != nil {
		return "", err
	}

	splits := strings.Split(x3durl, "/")

	lastitem := len(splits) - 1

	if lastitem < 0 {
		return "", errors.New("problematic url")
	}

	x3dzip := splits[lastitem]
	zippath := filepath.Join(myname, x3dzip)

	//create localzipfile
	out, err := os.Create(zippath)
	defer out.Close()
	if err != nil {
		return "", err
	}

	//get contents of zipfile
	resp, err := http.Get(x3durl + "/" + x3dzip)

	_, err1 := io.Copy(out, resp.Body)
	if err1 != nil {
		return "", err1
	}

	defer resp.Body.Close()

	//extract zip

	r, err := zip.OpenReader(zippath)
	if err != nil {
		return "", err
	}
	defer r.Close()
	for _, f := range r.File {
		err := cloneZipItem(f, myname)
		if err != nil {
			return "", err
		}
	}

	return myname, nil
}

func main() {
	streamlut = make(map[int]*PlayerConf)
	my_reviver = make(chan Coaction)
	my_answers = make(chan NodeResp)

	go keepalivefcn(my_reviver, my_answers)
	heartbeat(my_reviver, my_answers)
	wsContainer := restful.NewContainer()

	fmt.Println("Registering")
	ws := new(restful.WebService)
	ws.Path("/ipromotion").
		Consumes(restful.MIME_JSON).
		Produces(restful.MIME_JSON)
	ws.Route(ws.GET("").To(promo_listusers))
	ws.Route(ws.POST("").To(promo_create)).Consumes(restful.MIME_JSON).Produces(restful.MIME_JSON)
	ws.Route(ws.POST("/{user-id}/stop").To(promo_stop)).Consumes(restful.MIME_JSON).Produces(restful.MIME_JSON)
	ws.Route(ws.POST("/{user-id}/resume").To(promo_resume)).Consumes(restful.MIME_JSON).Produces(restful.MIME_JSON)
	ws.Route(ws.POST("/{user-id}/pause").To(promo_pause)).Consumes(restful.MIME_JSON).Produces(restful.MIME_JSON)
	ws.Route(ws.POST("/{user-id}/zoomin").To(promo_zoomin)).Consumes(restful.MIME_JSON).Produces(restful.MIME_JSON)
	ws.Route(ws.POST("/{user-id}/zoomout").To(promo_zoomout)).Consumes(restful.MIME_JSON).Produces(restful.MIME_JSON)
	ws.Route(ws.POST("/{user-id}/keepalive").To(promo_keepalive)).Consumes(restful.MIME_JSON).Produces(restful.MIME_JSON)
	ws.Route(ws.POST("/{user-id}/viewpoints").To(promo_viewpoints)).Consumes(restful.MIME_JSON).Produces(restful.MIME_JSON)
	ws.Route(ws.POST("/{user-id}/viewpoint").To(promo_getviewpoint)).Consumes(restful.MIME_JSON).Produces(restful.MIME_JSON)
	ws.Route(ws.POST("/{user-id}/mouseclick").To(promo_mouseclick)).Consumes(restful.MIME_JSON).Produces(restful.MIME_JSON)
	ws.Route(ws.POST("/{user-id}/alterviewpoint").To(promo_setviewpoint)).Consumes(restful.MIME_JSON).Produces(restful.MIME_JSON)
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

	log.Printf("start listening on localhost:8200")
	server := &http.Server{Addr: ":8200", Handler: wsContainer}
	log.Fatal(server.ListenAndServe())
}
