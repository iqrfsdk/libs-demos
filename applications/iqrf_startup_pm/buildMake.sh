project=iqrf_startup_pm

#expected build dir structure
buildexp=build/Unix_Makefiles

LIB_DIRECTORY=../../libs
currentdir=$PWD
builddir=./${buildexp}

mkdir -p ${builddir}

#get path to clibcdc libs
clibcdc=${LIB_DIRECTORY}/clibcdc/${buildexp}
pushd ${clibcdc}
clibcdc=$PWD
popd

#get path to clibspi libs
clibspi=${LIB_DIRECTORY}/clibspi/${buildexp}
pushd ${clibspi}
clibspi=$PWD
popd

#get path to clibdpa libs
clibdpa=${LIB_DIRECTORY}/clibdpa/${buildexp}
pushd ${clibdpa}
clibdpa=$PWD
popd

#get path to cutils libs
cutils=${LIB_DIRECTORY}/cutils/${buildexp}
pushd ${cutils}
cutils=$PWD
popd

#get path to iqrfd libs
iqrfd=../../daemon/${buildexp}
pushd ${iqrfd}
iqrfd=${PWD}
popd

#launch cmake to generate build environment
pushd ${builddir}
pwd
cmake -G "Unix Makefiles" -Dclibcdc_DIR:PATH=${clibcdc} -Dclibspi_DIR:PATH=${clibspi} -Dclibdpa_DIR:PATH=${clibdpa} -Dcutils_DIR:PATH=${cutils} -Diqrfd_DIR:PATH=${iqrfd} ${currentdir} -DCMAKE_BUILD_TYPE=Debug
popd

#build from generated build environment
cmake --build ${builddir}