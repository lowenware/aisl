#!/bin/bash

# -----------------------------------------------------------------------------
#
# CMake Wrapper v.1.0 for Linux
# (c) Copyright Löwenware Ltd. (https://lowenware.com/)
#
# -----------------------------------------------------------------------------

ABSOLUTE_PATH="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

PROJECT="aisl"
PROJECT_VERSION=$(cat ${ABSOLUTE_PATH}/version | sed  's/\([0-9]\{1,5\}.[0-9]\{1,5\}.[0-9]\{1,5\}\).*/\1/')

PREFIX="/usr"
SYSCONF="/etc"
DIR_BUILD="build"
DIR_ROOT="root"

# -----------------------------------------------------------------------------

function project_clean {
    echo "Cleaning..."

    if [ -d ./$DIR_BUILD ]; then
        rm -Rf ./$DIR_BUILD/*
    else
        mkdir ./$DIR_BUILD
    fi

    if [ -d ./$DIR_ROOT ]; then
        rm -Rf ./$DIR_ROOT/*
    else
        mkdir ./$DIR_ROOT
    fi

}

# -----------------------------------------------------------------------------

function project_compile {

    CMAKE="cmake"

    if [[ "$OSTYPE" == "darwin"* ]]; then
        ov="1.0.2n"
        ov_p="-DOPENSSL_INCLUDE_DIRS=/usr/local/Cellar/openssl/${ov}/include -DOPENSSL_CRYPTO_LIBRARY=/usr/local/Cellar/openssl/${ov}/lib/libcrypto.dylib -DOPENSSL_SSL_LIBRARY=/usr/local/Cellar/openssl/${ov}/lib/libssl.dylib -DOPENSSL_LIBRARY_DIRS=/usr/local/Cellar/openssl/${ov}/lib"
        CMAKE="cmake ${ov_p}"
    fi

    echo ${CMAKE}

    ${CMAKE} -B./$DIR_BUILD -H./ -DCMAKE_INSTALL_PREFIX=$PREFIX -DCMAKE_DEBUG=1
    cd $DIR_BUILD/
    make
    make DESTDIR=../$DIR_ROOT install
    cd ..
}

# -----------------------------------------------------------------------------

case $1 in
    clean)
        project_clean
    ;;
    compile)
        project_compile
    ;;
    build)
        project_clean
        project_compile
    ;;
    install)
        cmake -DWITH_EVERYTHING=1 -B./$DIR_BUILD -H./ -DCMAKE_INSTALL_PREFIX=$PREFIX
        cd $DIR_BUILD
        sudo make install
        cd ..
    ;;
    deploy)
      DEPLOY_PATH="${PROJECT}-${PROJECT_VERSION}"
      mkdir ${2}${DEPLOY_PATH}
      cp -R ${ABSOLUTE_PATH}/{include,library,LICENSE,AUTHORS,version,README.md,cmake*,CMakeLists.txt,cStuff} ${2}${DEPLOY_PATH}
      rm ${2}${DEPLOY_PATH}/cStuff/.git
      CUR_DIR=$(pwd)
      cd $2
      tar cfz ${DEPLOY_PATH}.tar.gz  ${DEPLOY_PATH}
      cd $CUR_DIR
      rm -Rf ${2}${DEPLOY_PATH}
      echo "_version ${PROJECT_VERSION}"
    ;;
    *)
        echo "Usage: ./project.sh (compile|build|clean|install)"
    ;;
esac


# -----------------------------------------------------------------------------
