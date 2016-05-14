#!/bin/bash

INCLUDE_SCHEMAS_PATH=include/schemas

#Build fbs in current path
if ! [ -d "${INCLUDE_SCHEMAS_PATH}/core" ];then
  mkdir -p ${INCLUDE_SCHEMAS_PATH}/core
fi

for f in $(ls ./*.fbs);do
  flatc --no-includes -o ${INCLUDE_SCHEMAS_PATH}/core/ -c $f 
done

#Invoke build scripts in individual path
for d in $(ls -d */);do
  cd $d
  if [ -f "build.sh" ];then
    echo "Found build script for ${d}"
    . build.sh
  fi
  cd -
done

#Generate forwarding header files
INCLUDE_SCHEMAS_PATH=include/schemas
cd $INCLUDE_SCHEMAS_PATH
for d in $(ls);do
  if [ -d "$d" ];then
    header_file=${d}.h
    if [ "$d" != "core" ];then
      echo "#include \"core.h\"" > $header_file
    else
      echo "#include <flatbuffers/flatbuffers.h>" > $header_file
    fi
    #Put first level header on the top
    for f in $(ls ${d}/*.h | grep -e "^_");do      
      echo "#include \"${f}\"" >> $header_file
    done
    for f in $(ls ${d}/*.h | grep -e "^[^_]");do
      echo "#include \"${f}\"" >> $header_file
    done
  fi
done

cd -
