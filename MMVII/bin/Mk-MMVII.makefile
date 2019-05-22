#
#== Directories
#
MMDir=../../
MMV2Dir=${MMDir}MMVII/
MMV2DirSrc=${MMV2Dir}src/
MMV2DirBin=${MMV2Dir}bin/
MMV2Objects=${MMV2Dir}object/
MMV2DirIncl=${MMV2Dir}include/
#MMV2ElisePath=${MMDir}/lib/libelise_SsQt.a
MMV2ElisePath=${MMDir}/lib/libelise.a
#
#
#       ===== INSTALLATION ========================================
#       => for setting MMVII directory
#
MMV2ResultInstal=${MMV2DirSrc}ResultInstall/ResultInstall.cpp
MMV2SrcInstal=${MMV2DirSrc}BinaireInstall/InstalMM.cpp
MMV2BinInstal=../Instal2007
#
#=========== Sous directory des sources
#
MMV2DirTLE=${MMV2DirSrc}TestLibsExtern/
SrcTLE=$(wildcard ${MMV2DirTLE}*.cpp)
ObjTLE=$(SrcTLE:.cpp=.o) 
#
MMV2DirT4MkF=${MMV2DirSrc}Test4Mkfile/
Src4Mkf= $(wildcard ${MMV2DirT4MkF}*.cpp)
ObjMkf= $(Src4Mkf:.cpp=.o) 
#
MMV2DirBench=${MMV2DirSrc}Bench/
SrcBench= $(wildcard ${MMV2DirBench}*.cpp)
ObjBench= $(SrcBench:.cpp=.o) 
#
#
MMV2DirAppli=${MMV2DirSrc}Appli/
SrcAppli= $(wildcard ${MMV2DirAppli}*.cpp)
ObjAppli= $(SrcAppli:.cpp=.o) 
#
#
MMV2DirUtils=${MMV2DirSrc}Utils/
SrcUtils= $(wildcard ${MMV2DirUtils}*.cpp)
ObjUtils= $(SrcUtils:.cpp=.o) 
#
#
MMV2DirSerial=${MMV2DirSrc}Serial/
SrcSerial= $(wildcard ${MMV2DirSerial}*.cpp)
ObjSerial= $(SrcSerial:.cpp=.o) 
#
#
MMV2DirMMV1=${MMV2DirSrc}MMV1/
SrcMMV1=$(wildcard ${MMV2DirMMV1}*.cpp)
ObjMMV1=$(SrcMMV1:.cpp=.o) 
#
#
MMV2DirRamImages=${MMV2DirSrc}RamImages/
SrcRamImages=$(wildcard ${MMV2DirRamImages}*.cpp)
ObjRamImages=$(SrcRamImages:.cpp=.o) 
#
#
MMV2DirMatrix=${MMV2DirSrc}Matrix/
SrcMatrix=$(wildcard ${MMV2DirMatrix}*.cpp)
ObjMatrix=$(SrcMatrix:.cpp=.o) 
#
#
MMV2DirPerso=${MMV2DirSrc}Perso/
SrcPerso=$(wildcard ${MMV2DirPerso}*.cpp)
ObjPerso=$(SrcPerso:.cpp=.o) 
#
#
#    => Le Main
MAIN=${MMV2DirSrc}main.cpp
#============ Calcul des objets
#
OBJ= ${ObjMatrix} ${ObjRamImages} ${ObjTLE} ${ObjMkf} ${ObjBench} ${ObjAppli} ${ObjUtils} ${ObjSerial} ${ObjMMV1} ${ObjPerso} 
#
#=========  Header ========
#
#
HEADER=$(wildcard ${MMV2DirIncl}*.h)
#
#  Binaries
#== CFLAGS etc...
#
CXX=g++
CFlags="-std=c++14" "-Wall" "-Werror" "-O4" "-march=native" -I${MMV2Dir} -I${MMDir}/include/
BOOST_LIBS= -lboost_system -lboost_serialization -lboost_regex -lboost_filesystem
QTAnnLibs= -lXext /usr/lib/x86_64-linux-gnu/libQt5Core.so.5.5.1 /usr/lib/x86_64-linux-gnu/libQt5Gui.so.5.5.1 /usr/lib/x86_64-linux-gnu/libQt5Xml.so.5.5.1 /usr/lib/x86_64-linux-gnu/libQt5OpenGL.so.5.5.1 -lGLU -lGL  -ldl -lpthread /usr/lib/x86_64-linux-gnu/libQt5Xml.so.5.5.1 /usr/lib/x86_64-linux-gnu/libQt5Concurrent.so.5.5.1 /usr/lib/x86_64-linux-gnu/libQt5OpenGL.so.5.5.1 /usr/lib/x86_64-linux-gnu/libQt5Widgets.so.5.5.1 /usr/lib/x86_64-linux-gnu/libQt5Gui.so.5.5.1 /usr/lib/x86_64-linux-gnu/libQt5Core.so.5.5.1 ../../lib/libANN.a
LibsFlags= ${MMV2ElisePath} -lX11  ${BOOST_LIBS}  ${QTAnnLibs}
MMV2Exe=MMVII
#
${MMV2DirBin}${MMV2Exe} :  ${OBJ} ${MAIN} ${MMV2ResultInstal} ${MMV2ElisePath}
	${CXX}  ${MAIN} ${CFlags}  ${OBJ}  ${LibsFlags}  -o ${MMV2DirBin}${MMV2Exe} 
	ar rvs P2007.a    ${OBJ}  
#
# ==========    INSTALLATION =================
#
${MMV2ResultInstal} : ${MMV2SrcInstal}
	${CXX} ${MMV2SrcInstal} -o ${MMV2BinInstal} ${BOOST_LIBS}
	mkdir -p `dirname ${MMV2ResultInstal}`
	${MMV2BinInstal}
	rm ${MMV2BinInstal}
#
# ================ Objects ==================
#
${MMV2DirPerso}%.o :  ${MMV2DirPerso}%.cpp   ${HEADER}
	${CXX} -c  $< ${CFlags} -o $@
${MMV2DirMMV1}%.o :  ${MMV2DirMMV1}%.cpp   ${HEADER}
	${CXX} -c  $< ${CFlags} -o $@
${MMV2DirTLE}%.o :  ${MMV2DirTLE}%.cpp   ${HEADER}
	${CXX} -c  $< ${CFlags} -o $@
${MMV2DirT4MkF}%.o :  ${MMV2DirT4MkF}%.cpp ${HEADER}
	${CXX} -c  $< ${CFlags} -o $@
${MMV2DirBench}%.o :  ${MMV2DirBench}%.cpp ${HEADER}
	${CXX} -c  $< ${CFlags} -o $@
${MMV2DirAppli}%.o :  ${MMV2DirAppli}%.cpp ${HEADER}
	${CXX} -c  $< ${CFlags} -o $@
${MMV2DirUtils}%.o :  ${MMV2DirUtils}%.cpp ${HEADER}
	${CXX} -c  $< ${CFlags} -o $@
${MMV2DirSerial}%.o :  ${MMV2DirSerial}%.cpp ${HEADER}
	${CXX} -c  $< ${CFlags} -o $@
${MMV2DirRamImages}%.o :  ${MMV2DirRamImages}%.cpp   ${HEADER}
	${CXX} -c  $< ${CFlags} -o $@
${MMV2DirMatrix}%.o :  ${MMV2DirMatrix}%.cpp   ${HEADER}
	${CXX} -c  $< ${CFlags} -o $@
#
#       ===== TEST ========================================
#
all : ${MMV2DirBin}${MMV2Exe} ${OBJ}
	${CXX}  ${MAIN} ${CFlags}  ${OBJ}  ${LibsFlags}  -o ${MMV2DirBin}${MMV2Exe}
Show:
	echo ${SrcUtils}
	echo DU=${MMV2DirUtils}
	echo ObjRamImages: ${ObjRamImages}
	echo SrcRamImages: ${SrcRamImages}
	echo MMV2DiRamImages: ${MMV2DiRamImages}
clean :
	rm -f ${OBJ}
#
#
