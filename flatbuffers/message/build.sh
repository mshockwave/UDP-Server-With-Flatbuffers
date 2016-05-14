#!/bin/bash

INCLUDE_SCHEMAS_PATH=../include/schemas

if ! [ -d "${INCLUDE_SCHEMAS_PATH}/message" ];then
  mkdir -p ${INCLUDE_SCHEMAS_PATH}/message
fi

for f in $(ls ./*.fbs);do
  flatc --no-includes -o ${INCLUDE_SCHEMAS_PATH}/message/ -c $f 
done
