#!/bin/bash


lower_case=$(echo "$1" | tr '[:upper:]' '[:lower:]')

if [ -z $1 ]; then 
	echo Need to provide platform. Possible platforms are linux, macosx, ios. Exiting!
	exit 
fi 

rm -rf tmp 
mkdir tmp 

CINDER_DIR=`pwd`/../../../ 
CINDER_LIB_DIR=${CINDER_DIR}/lib/${lower_case}/Release
CINDER_FREETYPE_INCLUDE_PATH=${CINDER_DIR}/freetype

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

FINAL_PATH=`pwd`/..
FINAL_LIB_PATH=${FINAL_PATH}/lib/${lower_case}
rm -rf ${FINAL_LIB_PATH}
mkdir -p ${FINAL_LIB_PATH}

FINAL_INCLUDE_PATH=${FINAL_PATH}/include/${lower_case}
rm -rf ${FINAL_INCLUDE_PATH}
mkdir -p ${FINAL_INCLUDE_PATH}

export PATH="${PREFIX_LIBPNG}/bin:${PREFIX_PIXMAN}/bin:$PATH" 

cd tmp

buildIos() 
{
	ARCH="arm64"
	export XCODE_DEVELOPER=`xcode-select --print-path`

	export IOS_PLATFORM="iPhoneOS"	
	export IOS_PLATFORM_DEVELOPER="${XCODE_DEVELOPER}/Platforms/${IOS_PLATFORM}.platform/Developer"
	LATEST_SDK=`ls ${IOS_PLATFORM_DEVELOPER}/SDKs | sort -r | head -n1`
	export IOS_SDK="${IOS_PLATFORM_DEVELOPER}/SDKs/${LATEST_SDK}"
	export HOST="arm-apple-darwin"

	export CXX="${XCODE_DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++"
	export CC="${XCODE_DEVELOPER}/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang"
	export CFLAGS="-isysroot ${IOS_SDK} -I${IOS_SDK}/usr/include -arch ${ARCH} -miphoneos-version-min=5.0"
	export CXXFLAGS="-stdlib=libc++ -isysroot ${IOS_SDK} -I${IOS_SDK}/usr/include -arch ${ARCH}  -miphoneos-version-min=5.0"
	export LDFLAGS="-stdlib=libc++ -isysroot ${IOS_SDK} -L${FINAL_LIB_PATH} -L${IOS_SDK}/usr/lib -arch ${ARCH} -miphoneos-version-min=5.0"

	export PIXMAN_CFLAGS_armv7="${CFLAGS}"
	export PIXMAN_CFLAGS_armv7s="${CFLAGS}"
	export PIXMAN_CFLAGS_i386="${CFLAGS} -DPIXMAN_NO_TLS"
	export PIXMAN_CXXFLAGS_armv7="${CXXFLAGS}"
	export PIXMAN_CXXFLAGS_armv7s="${CXXFLAGS}"
	export PIXMAN_CXXFLAGS_i386="${CXXFLAGS} -DPIXMAN_NO_TLS"
}

buildOSX() 
{
	echo Setting up OSX environment...
	
#	CFLAGS = -isysroot ${IOS_SDK} -I${IOS_SDK}/usr/include -arch ${ARCH} -miphoneos-version-min=5.0
#	CXXFLAGS = -stdlib=libc++ -isysroot ${IOS_SDK} -I${IOS_SDK}/usr/include -arch ${ARCH}  -miphoneos-version-min=5.0 

	downloadZlib
	buildZlib
		
	buildLibPng $PREFIX_LIBPNG
	
	export PNG_CFLAGS="-I${FINAL_INCLUDE_PATH}" 
	export PNG_LIBS="-L${FINAL_LIB_PATH} -lpng -lz" 
	
	rm -rf $PREFIX_PIXMAN
	mkdir -p $PREFIX_PIXMAN 
	buildPixman $PREFIX_PIXMAN 
	
	export png_LIBS="-L${FINAL_LIB_PATH} -lpng"
	export png_CFLAGS="-I${FINAL_INCLUDE_PATH}" 
	export pixman_CFLAGS="-I${FINAL_INCLUDE_PATH}/pixman-1"
	export pixman_LIBS="-L${FINAL_LIB_PATH} -lpixman-1"
	export FREETYPE_LIBS="-L${CINDER_LIB_DIR} -lcinder"
	export FREETYPE_CFLAGS="-I${CINDER_FREETYPE_INCLUDE_PATH}"

	OPTIONS="--disable-quartz --disable-quartz-font --disable-quartz-image --without-x --disable-xlib --disable-xlib-xrender --disable-xcb --disable-xlib-xcb --disable-xcb-shm --enable-ft --disable-full-testing" 

	export CC="clang -Wno-enum-conversion -I${INCLUDEDIR}/pixman-1"
#	export LDFLAGS="-stdlib=libc++ -L${FINAL_LIB_PATH} -lpng -lpixman-1 -lfreetype -lfontconfig ${LDFLAGS}"

	rm -rf $PREFIX_CAIRO 
	mkdir -p $PREFIX_CAIRO
	buildCairo $PREFIX_CAIRO $OPTIONS 
}

buildLinux() 
{
	echo Setting up Linux environment...
	
#	CFLAGS = -isysroot ${IOS_SDK} -I${IOS_SDK}/usr/include -arch ${ARCH} -miphoneos-version-min=5.0
#	CXXFLAGS = -stdlib=libc++ -isysroot ${IOS_SDK} -I${IOS_SDK}/usr/include -arch ${ARCH}  -miphoneos-version-min=5.0 

	buildLibPng $PREFIX_LIBPNG
	
	export PNG_CFLAGS="-I${FINAL_INCLUDE_PATH}" 
	export PNG_LIBS="-L${FINAL_LIB_PATH} -lpng" 
	
	rm -rf $PREFIX_PIXMAN
	mkdir -p $PREFIX_PIXMAN 
	buildPixman $PREFIX_PIXMAN 
	
	export png_LIBS="-L${FINAL_LIB_PATH} -lpng"
	export png_CFLAGS="-I${FINAL_INCLUDE_PATH}" 
	export pixman_CFLAGS="-I${FINAL_INCLUDE_PATH}/pixman-1"
	export pixman_LIBS="-L${FINAL_LIB_PATH} -lpixman-1"
	export FREETYPE_LIBS="-L${CINDER_LIB_DIR} -lcinder"
	export FREETYPE_CFLAGS="-I${CINDER_FREETYPE_INCLUDE_PATH}"

	OPTIONS="--without-x --disable-xlib --disable-xlib-xrender --disable-xcb --disable-xlib-xcb --disable-xcb-shm --enable-ft --disable-full-testing" 

	export CC="gcc -I${INCLUDEDIR}/pixman-1"
#	export LDFLAGS="-stdlib=libc++ -L${FINAL_LIB_PATH} -lpng -lpixman-1 -lfreetype -lfontconfig ${LDFLAGS}"

	rm -rf $PREFIX_CAIRO 
	mkdir -p $PREFIX_CAIRO
	buildCairo $PREFIX_CAIRO $OPTIONS 
}

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

buildZlib()
{
 	cd zlib
	echo "Building zlib, and installing $1"
	PREFIX=$1
	if [ -z $2 ]; then
		./configure
	else
		./configure --host=${HOST} --disable-shared
	fi

	make -j 6
	make install
	make clean

	cd ..	
}

buildLibPng()
{
	cd libpng
	echo "Building libpng, and installing $1"
 	PREFIX=$1
	if [ -z $2 ]; then 
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

buildPixman()
{
	cd pixman
	echo "Building pixman, and installing $1"
	PREFIX=$1
 	if [ -z $2 ]; then 
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

	echo "Building libpng, and installing $1"
 	PREFIX=$1
	if [ -z $3 ]; then 
		./configure --disable-shared --enable-static --prefix=${PREFIX} $OPTIONS 
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

downloadFreetype
downloadPkgConfig
downloadLibPng
downloadLibPixman
downloadLibCairo

declare -a config_settings=("debug" "release")
declare -a config_paths=("/Debug" "/Release")

echo "Building cairo for {$lower_case}"
if [ "${lower_case}" = "mac" ] || [ "${lower_case}" = "macosx" ];
then
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

