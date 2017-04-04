#!/bin/bash

module purge
module load openmpi/2.0.0-gcc_4.8.5
module load cmake/3.7.1
module load python/2.7.3

CURRENT_PATH=$(pwd)

export export CMAKE_CXX_FLAGS=-std=c++11


# Software to download: llvm 3.9.1, mesa 13.0.3, glu 9.0.0, paraview 5.2.0
wget http://llvm.org/releases/3.9.1/llvm-3.9.1.src.tar.xz
wget https://mesa.freedesktop.org/archive/13.0.3/mesa-13.0.3.tar.gz
wget ftp://ftp.freedesktop.org/pub/mesa/glu/glu-9.0.0.tar.bz2
wget http://www.paraview.org/files/v5.2/ParaView-v5.2.0.tar.gz



# LLVM
tar -xf llvm-3.9.1.src.tar.xz
mkdir llvm-3.9.1.src/install
mkdir llvm-3.9.1.src/build
cd llvm-3.9.1.src
MY_LLVM_PATH=$(pwd)
cd build

cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$MY_LLVM_PATH/install -DLLVM_BUILD_LLVM_DYLIB=ON -DLLVM_ENABLE_RTTI=ON -DLLVM_INSTALL_UTILS=ON -DLLVM_TARGETS_TO_BUILD:STRING=X86 ..

make -j32
make install

export PATH=$MY_LLVM_PATH/install/bin:$PATH
export LD_LIBRARY_PATH=$MY_LLVM_PATH/install/lib/:$LD_LIBRARY_PATH
cd ..
cd ..


# mesa
tar -zxf mesa-13.0.3.tar.gz
mkdir mesa-13.0.3/install
cd mesa-13.0.3
MY_MESA_PATH=$(pwd)
   
./configure --disable-xvmc \
    --disable-glx \
    --disable-dri \
    --with-dri-drivers= \
    --with-gallium-drivers=swrast \
    --enable-texture-float \
    --disable-egl \
    --with-egl-platforms= \
    --enable-gallium-osmesa \
    --enable-gallium-llvm=yes \
    --prefix=$MY_MESA_PATH/install

make -j32
make install

export LD_LIBRARY_PATH=$MY_MESA_PATH/install/lib/:$LD_LIBRARY_PATH
export LD_RUN_PATH=$MY_MESA_PATH/install/lib/:$LD_RUN_PATH
cd ..



# glu
tar -xf glu-9.0.0.tar.bz2
mkdir glu-9.0.0/install
cd glu-9.0.0
MY_GLU_PATH=$(pwd)

./configure --prefix=$MY_GLU_PATH/install
make -j32
make install

cd ..


# Paraview
tar -zxf ParaView-v5.2.0.tar.gz
mkdir ParaView-v5.2.0/build/
cd ParaView-v5.2.0
cd build
cmake .. -DPARAVIEW_BUILD_QT_GUI:BOOL=OFF -DPARAVIEW_USE_MPI:BOOL=ON \
         -DPARAVIEW_ENABLE_PYTHON:BOOL=ON -DVTK_USE_X:BOOL=OFF -DVTK_OPENGL_HAS_OSMESA:BOOL=ON \
         -DOPENGL_gl_LIBRARY:STRING="" -DOPENGL_INCLUDE_DIR:PATH="$MY_MESA_PATH/include/" -DOPENGL_glu_LIBRARY:FILEPATH="$MY_GLU_PATH/install/lib/libGLU.so" \
         -DOSMESA_INCLUDE_DIR:PATH="$MY_MESA_PATH/include/" -DOSMESA_LIBRARY:FILEPATH="$MY_MESA_PATH/install/lib/libOSMesa.so"

make -j32
cd ..
cd ..





touch modulePathSet.sh
echo "module purge" >> modulePathSet.sh
echo "module load openmpi/2.0.0-gcc_4.8.5" >> modulePathSet.sh
echo "module load cmake/3.7.1" >> modulePathSet.sh
echo "module load python/2.7.3" >> modulePathSet.sh
echo "" >> modulePathSet.sh

# LLVM
echo "export PATH=$MY_LLVM_PATH/install/bin:\$PATH">> modulePathSet.sh
echo "export LD_LIBRARY_PATH=$MY_LLVM_PATH/install/lib/:\$LD_LIBRARY_PATH">> modulePathSet.sh
echo "">> modulePathSet.sh

# Mesa
echo "export PATH=$MY_MESA_PATH/install/bin:\$PATH">> modulePathSet.sh
echo "export LD_LIBRARY_PATH=$MY_MESA_PATH/install/lib/:\$LD_LIBRARY_PATH">> modulePathSet.sh
echo "">> modulePathSet.sh

# Glu
echo "export LD_LIBRARY_PATH=$MY_GLU_PATH/install/lib/:\$LD_LIBRARY_PATH">> modulePathSet.sh
echo "">> modulePathSet.sh

chmod a+x modulePathSet.sh