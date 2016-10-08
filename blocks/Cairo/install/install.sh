#!/bin/bash

lower_case=$(echo "$1" | tr '[:upper:]' '[:lower:]')

if [ -z $1 ]; then 
	echo Need to provide platform. Possible platforms are linux, macosx, ios. Exiting!
	exit 
fi 

#########################
## create prefix dirs
#########################

PREFIX_BASE_DiR=`pwd`/tmp

PREFIX_LIBZ=${PREFIX_BASE_DiR}/libz_install
rm -rf $PREFIX_LIBZ
mkdir -p $PREFIX_LIBZ

PREFIX_LIBPNG=${PREFIX_BASE_DiR}/libpng_install
rm -rf $PREFIX_LIBPNG
mkdir -p $PREFIX_LIBPNG 

PREFIX_PIXMAN=${PREFIX_BASE_DiR}/pixman_install
rm -rf $PREFIX_PIXMAN 
mkdir -p $PREFIX_PIXMAN 

PREFIX_CAIRO=${PREFIX_BASE_DiR}/cairo_install
rm -rf $PREFIX_CAIRO
mkdir -p $PREFIX_CAIRO

##################################
## we use cinder to link freetype
##################################

CINDER_DIR=`pwd`/../../../ 
CINDER_LIB_DIR=${CINDER_DIR}/lib/${lower_case}/Release
CINDER_FREETYPE_INCLUDE_PATH=${CINDER_DIR}/freetype

#########################
## create final path
#########################

FINAL_PATH=`pwd`/..
FINAL_LIB_PATH=${FINAL_PATH}/lib/${lower_case}
rm -rf ${FINAL_LIB_PATH}
mkdir -p ${FINAL_LIB_PATH}

FINAL_INCLUDE_PATH=${FINAL_PATH}/include/${lower_case}
rm -rf ${FINAL_INCLUDE_PATH}
mkdir -p ${FINAL_INCLUDE_PATH}

export PATH="${PREFIX_LIBPNG}/bin:${PREFIX_PIXMAN}/bin:$PATH" 

########################
## building ios
#########################

buildIos() 
{
	ARCH="arm64"
	export XCODE_DEVELOPER=`xcode-select --print-path`

	export IOS_PLATFORM="iPhoneOS"	
	export IOS_PLATFORM_DEVELOPER="${XCODE_DEVELOPER}/Platforms/${IOS_PLATFORM}.platform/Developer"
	LATEST_SDK=`ls ${IOS_PLATFORM_DEVELOPER}/SDKs | sort -r | head -n1`
	export IOS_SDK="${IOS_PLATFORM_DEVELOPER}/SDKs/${LATEST_SDK}"
	HOST="arm-apple-darwin"

	export CXX="${XCODE_DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++"
	export CC="${XCODE_DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang"
	export CPPFLAGS="-isysroot ${IOS_SDK} -I${IOS_SDK}/usr/include -arch arm64 -mios-version-min=8.0"
	export CXXFLAGS="-stdlib=libc++ -isysroot ${IOS_SDK} -I${IOS_SDK}/usr/include -arch arm64  -mios-version-min=8.0"
	export LDFLAGS="-stdlib=libc++ -isysroot ${IOS_SDK} -L${FINAL_LIB_PATH} -L${IOS_SDK}/usr/lib -arch arm64 -mios-version-min=8.0"
	echo 'IOS_SDK =' ${IOS_SDK}

	buildLibPng $HOST

	export PNG_CFLAGS="-I${FINAL_INCLUDE_PATH}"
	export PNG_LIBS="-L${FINAL_LIB_PATH} -lpng -L${IOS_SDK}/usr/lib -lz"

	#export PIXMAN_CFLAGS_i386="${CFLAGS} -DPIXMAN_NO_TLS"
	#export PIXMAN_CXXFLAGS_i386="${CXXFLAGS} -DPIXMAN_NO_TLS"

  buildPixman $HOST

	export png_LIBS="-L${FINAL_LIB_PATH} -lpng"
	export png_CFLAGS="-I${FINAL_INCLUDE_PATH}" 
	export pixman_CFLAGS="-I${FINAL_INCLUDE_PATH}/pixman-1"
	export pixman_LIBS="-L${FINAL_LIB_PATH} -lpixman-1"
	export FREETYPE_LIBS="-L${CINDER_LIB_DIR} -lcinder"
	export FREETYPE_CFLAGS="-I${CINDER_FREETYPE_INCLUDE_PATH}"

	OPTIONS="--disable-quartz --disable-quartz-font --disable-quartz-image --without-x --disable-xlib --disable-xlib-xrender --disable-xcb --disable-xlib-xcb --disable-xcb-shm --enable-ft --disable-full-testing" 

	export CC="clang -Wno-enum-conversion -I${INCLUDEDIR}/pixman-1"
	#export LDFLAGS="-stdlib=libc++ -L${FINAL_LIB_PATH} -lpng -lpixman-1 -lfreetype -lfontconfig ${LDFLAGS}"

	buildCairo "${OPTIONS}" "${HOST}"
}

#########################
## building osx
#########################

buildOSX() 
{
	echo Setting up OSX environment...
	
  # On osx, i want to make sure the zlib version, we use
  downloadZlib
	buildZlib
	buildLibPng
	buildPixman 
	
	OPTIONS="--enable-quartz=yes --enable-quartz-surface=yes --enable-quartz-image=yes --without-x --disable-xlib --disable-xlib-xrender --disable-xcb --disable-xlib-xcb --disable-xcb-shm --enable-ft --disable-full-testing" 
  
	buildCairo "${OPTIONS}" 
}

#########################
## building linux
#########################

buildLinux() 
{
	echo Setting up Linux environment...
	
#	CFLAGS = -isysroot ${IOS_SDK} -I${IOS_SDK}/usr/include -arch ${ARCH} -miphoneos-version-min=5.0
#	CXXFLAGS = -stdlib=libc++ -isysroot ${IOS_SDK} -I${IOS_SDK}/usr/include -arch ${ARCH}  -miphoneos-version-min=5.0 

	buildLibPng
	
	export PNG_CFLAGS="-I${FINAL_INCLUDE_PATH}" 
	export PNG_LIBS="-L${FINAL_LIB_PATH} -lpng" 
	 
	buildPixman 
	
	export png_LIBS="-L${FINAL_LIB_PATH} -lpng"
	export png_CFLAGS="-I${FINAL_INCLUDE_PATH}" 
	export pixman_CFLAGS="-I${FINAL_INCLUDE_PATH}/pixman-1"
	export pixman_LIBS="-L${FINAL_LIB_PATH} -lpixman-1"
	export FREETYPE_LIBS="-L${CINDER_LIB_DIR} -lcinder"
	export FREETYPE_CFLAGS="-I${CINDER_FREETYPE_INCLUDE_PATH}"

	OPTIONS="--without-x --disable-xlib --disable-xlib-xrender --disable-xcb --disable-xlib-xcb --disable-xcb-shm --enable-ft --disable-full-testing" 

	export CC="gcc -I${INCLUDEDIR}/pixman-1"
#	export LDFLAGS="-stdlib=libc++ -L${FINAL_LIB_PATH} -lpng -lpixman-1 -lfreetype -lfontconfig ${LDFLAGS}"

	buildCairo $OPTIONS 
}

#########################
## downloading libs
#########################

downloadZlib()
{
	echo Downloading zlib...
	curl http://zlib.net/zlib-1.2.8.tar.gz -o zlib.tar.gz
	tar -xf zlib.tar.gz
	mv zlib-* zlib
	rm zlib.tar.gz
	echo Finished Downloading zlib...
}

downloadPkgConfig()
{
	echo Downloading pkg-config...
	curl https://pkg-config.freedesktop.org/releases/pkg-config-0.29.1.tar.gz -o pkgconfig.tar.gz 
	tar -xf pkgconfig.tar.gz
	mv pkg-config-* pkgconfig 
	rm pkgconfig.tar.gz
	echo Finished Downloading pkg-config...
}

downloadFreetype()
{
	echo Downloading freetype...
}

downloadLibPng() 
{
	echo Downloading libpng...
	curl ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng16/libpng-1.6.25.tar.gz -o libpng.tar.gz
	tar -xf libpng.tar.gz 
	mv libpng-* libpng
	rm libpng.tar.gz 
	echo Finished downloading libpng...
}

downloadLibPixman() 
{
	echo Downloading pixman...
	curl https://www.cairographics.org/releases/pixman-0.34.0.tar.gz -o pixman.tar.gz 
	tar -xf pixman.tar.gz
	mv pixman-* pixman 
	rm pixman.tar.gz 
	echo Finished downloading pixman...
}

downloadLibCairo() 
{
	echo Downloading cairo...
	curl https://www.cairographics.org/releases/cairo-1.14.6.tar.xz -o cairo.tar.xz
	tar -xf cairo.tar.xz 
	mv cairo-* cairo 
	rm cairo.tar.xz 
	echo Finished downloading cairo...
}

#########################
## building libs
#########################

buildZlib()
{
 	cd zlib
	echo "Building zlib, and installing $1"
	
	./configure

	make -j 6
	make install
	make clean

	cd ..	
}

buildLibPng()
{
	cd libpng
	echo "Building libpng, and installing $1"
 	PREFIX=$PREFIX_LIBPNG
	HOST=$1
	echo "Passed in $HOST"
	if [ -z "$HOST" ]; then 
		./configure --disable-shared --prefix=${PREFIX}
	else	
		echo Building with cross-compile
		./configure --host=${HOST} --disable-shared --prefix=${PREFIX} 
	fi

  make -j 6
	make install
	make clean
 	
	cp -r ${PREFIX}/include/* ${FINAL_INCLUDE_PATH}
	cp ${PREFIX}/lib/*.a ${FINAL_LIB_PATH}

	cd ..	
}

buildPixman()
{
	cd pixman
	echo "Building pixman, and installing $1"
	PREFIX=$PREFIX_PIXMAN
	HOST=$1
 	if [ -z "$HOST" ]; then 
		./configure --disable-shared --prefix=${PREFIX}
	else	
		./configure --host=${HOST} --disable-shared --prefix=${PREFIX} 
	fi

  make -j 6
	make install
	make clean
 	
	cp -r ${PREFIX}/include/* ${FINAL_INCLUDE_PATH}
	cp ${PREFIX}/lib/*.a ${FINAL_LIB_PATH}

	cd ..	
}

buildCairo()
{
	cd cairo

	echo "Building cairo, and installing ${PREFIX_CAIRO}, with options ${1}"
 	PREFIX=$PREFIX_CAIRO
	OPTIONS=$1
	HOST=$2
	if [ -z "$HOST" ]; then 
		./configure --disable-shared --enable-static --prefix=${PREFIX} ${OPTIONS}
	else	
		./configure --host=${HOST} --disable-shared --enable-static --prefix=${PREFIX} $OPTIONS
	fi
  
	make -j 6
	make install
	make clean
 		
	cp -r ${PREFIX}/include/* ${FINAL_INCLUDE_PATH}
	cp ${PREFIX}/lib/libcairo.a ${FINAL_LIB_PATH}

	cd ..	
}

#########################
## program
#########################

# Create working directory
rm -rf tmp 
mkdir tmp 
cd tmp

downloadFreetype
downloadPkgConfig
downloadLibPng
downloadLibPixman
downloadLibCairo

## Set up flags used by the builds
export PNG_CFLAGS="-I${FINAL_INCLUDE_PATH}" 
export PNG_LIBS="-L${FINAL_LIB_PATH} -lpng -lz" 
export png_LIBS="-L${FINAL_LIB_PATH} -lpng"
export png_CFLAGS="-I${FINAL_INCLUDE_PATH}" 
export pixman_CFLAGS="-I${FINAL_INCLUDE_PATH}/pixman-1"
export pixman_LIBS="-L${FINAL_LIB_PATH} -lpixman-1"
export FREETYPE_LIBS="-L${CINDER_LIB_DIR} -lcinder"
export FREETYPE_CFLAGS="-I${CINDER_FREETYPE_INCLUDE_PATH}"

echo "Building cairo for {$lower_case}"
if [ "${lower_case}" = "mac" ] || [ "${lower_case}" = "macosx" ];
then
  export CXX= "$(xcrun -find -sdk macosx clang++) -Wno-enum-conversion"
  export CC="$(xcrun -find -sdk macosx clang) -Wno-enum-conversion"
  export CFLAGS="-O3 -pthread ${CFLAGS}"
  export CXXFLAGS="-O3 -pthread ${CXXFLAGS}"
	export LDFLAGS="-stdlib=libc++ -framework CoreText -framework CoreFoundation -framework CoreGraphics  ${LDFLAGS}"
	buildOSX
elif [ "${lower_case}" = "linux" ];
then
	buildLinux
elif [ "${lower_case}" = "ios" ];
then
	buildIos
else
	echo "Unkown selection: ${1}"
	echo "usage: ./install.sh [platform]"
	echo "accepted platforms are macosx, linux, ios"
	exit 1
fi

