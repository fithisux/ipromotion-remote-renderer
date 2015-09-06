#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=MinGW_1-Windows
CND_DLIB_EXT=dll
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/GLFWWindow.o \
	${OBJECTDIR}/RendLoad.o \
	${OBJECTDIR}/RendStreamer.o \
	${OBJECTDIR}/main1.o \
	${OBJECTDIR}/promorend.o \
	${OBJECTDIR}/zthread.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L../../../../../langs/h3d/builds/lib64 -L../../../../../langs/msys64/mingw64/x86_64-w64-mingw32/lib -L../../../../../langs/msys64/mingw64/lib/glib-2.0 -L../../../../../langs/msys64/mingw64/lib -lH3DAPI.dll -lH3DUtil.dll -lHAPI.dll -lwinpthread.dll -lglfw3.dll -lopengl32 -lglu32 -lgobject-2.0.dll -lgstapp-1.0.dll -lgstbase-1.0.dll -lgstreamer-1.0.dll -lgio-2.0.dll -lglib-2.0.dll -lgstvideo-1.0.dll

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libx3dsphere.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libx3dsphere.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libx3dsphere.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared

${OBJECTDIR}/GLFWWindow.o: GLFWWindow.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDLL_EXPORT -I../../../../../langs/h3d/h3dapi-2.3.0/include -I../../../../../langs/h3d/hapi-1.3.0/include -I../../../../../langs/h3d/h3dutil-1.3.0/include -I../../../../../langs/msys64/mingw64/x86_64-w64-mingw32/include -I../../../../../langs/msys64/mingw64/include/glib-2.0 -I../../../../../langs/msys64/mingw64/lib/glib-2.0/include -I../../../../../langs/msys64/mingw64/include/gdk-pixbuf-2.0 -I../../../../../langs/msys64/mingw64/include -I../../../../../langs/msys64/mingw64/include/gstreamer-1.0 -I../../msys64/mingw64/include/boost  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/GLFWWindow.o GLFWWindow.cpp

${OBJECTDIR}/RendLoad.o: RendLoad.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDLL_EXPORT -I../../../../../langs/h3d/h3dapi-2.3.0/include -I../../../../../langs/h3d/hapi-1.3.0/include -I../../../../../langs/h3d/h3dutil-1.3.0/include -I../../../../../langs/msys64/mingw64/x86_64-w64-mingw32/include -I../../../../../langs/msys64/mingw64/include/glib-2.0 -I../../../../../langs/msys64/mingw64/lib/glib-2.0/include -I../../../../../langs/msys64/mingw64/include/gdk-pixbuf-2.0 -I../../../../../langs/msys64/mingw64/include -I../../../../../langs/msys64/mingw64/include/gstreamer-1.0 -I../../msys64/mingw64/include/boost  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RendLoad.o RendLoad.cpp

${OBJECTDIR}/RendStreamer.o: RendStreamer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDLL_EXPORT -I../../../../../langs/h3d/h3dapi-2.3.0/include -I../../../../../langs/h3d/hapi-1.3.0/include -I../../../../../langs/h3d/h3dutil-1.3.0/include -I../../../../../langs/msys64/mingw64/x86_64-w64-mingw32/include -I../../../../../langs/msys64/mingw64/include/glib-2.0 -I../../../../../langs/msys64/mingw64/lib/glib-2.0/include -I../../../../../langs/msys64/mingw64/include/gdk-pixbuf-2.0 -I../../../../../langs/msys64/mingw64/include -I../../../../../langs/msys64/mingw64/include/gstreamer-1.0 -I../../msys64/mingw64/include/boost  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RendStreamer.o RendStreamer.cpp

${OBJECTDIR}/main1.o: main1.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDLL_EXPORT -I../../../../../langs/h3d/h3dapi-2.3.0/include -I../../../../../langs/h3d/hapi-1.3.0/include -I../../../../../langs/h3d/h3dutil-1.3.0/include -I../../../../../langs/msys64/mingw64/x86_64-w64-mingw32/include -I../../../../../langs/msys64/mingw64/include/glib-2.0 -I../../../../../langs/msys64/mingw64/lib/glib-2.0/include -I../../../../../langs/msys64/mingw64/include/gdk-pixbuf-2.0 -I../../../../../langs/msys64/mingw64/include -I../../../../../langs/msys64/mingw64/include/gstreamer-1.0 -I../../msys64/mingw64/include/boost  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main1.o main1.cpp

${OBJECTDIR}/promorend.o: promorend.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDLL_EXPORT -I../../../../../langs/h3d/h3dapi-2.3.0/include -I../../../../../langs/h3d/hapi-1.3.0/include -I../../../../../langs/h3d/h3dutil-1.3.0/include -I../../../../../langs/msys64/mingw64/x86_64-w64-mingw32/include -I../../../../../langs/msys64/mingw64/include/glib-2.0 -I../../../../../langs/msys64/mingw64/lib/glib-2.0/include -I../../../../../langs/msys64/mingw64/include/gdk-pixbuf-2.0 -I../../../../../langs/msys64/mingw64/include -I../../../../../langs/msys64/mingw64/include/gstreamer-1.0 -I../../msys64/mingw64/include/boost  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/promorend.o promorend.cpp

${OBJECTDIR}/zthread.o: zthread.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DDLL_EXPORT -I../../../../../langs/h3d/h3dapi-2.3.0/include -I../../../../../langs/h3d/hapi-1.3.0/include -I../../../../../langs/h3d/h3dutil-1.3.0/include -I../../../../../langs/msys64/mingw64/x86_64-w64-mingw32/include -I../../../../../langs/msys64/mingw64/include/glib-2.0 -I../../../../../langs/msys64/mingw64/lib/glib-2.0/include -I../../../../../langs/msys64/mingw64/include/gdk-pixbuf-2.0 -I../../../../../langs/msys64/mingw64/include -I../../../../../langs/msys64/mingw64/include/gstreamer-1.0 -I../../msys64/mingw64/include/boost  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/zthread.o zthread.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libx3dsphere.${CND_DLIB_EXT}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
