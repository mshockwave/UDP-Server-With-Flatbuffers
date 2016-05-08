#!/bin/bash

INCLUDE_SCHEMAS_PATH=../include/schemas

if ! [ -d "${INCLUDE_SCHEMAS_PATH}/account" ];then
  mkdir -p ${INCLUDE_SCHEMAS_PATH}/account
fi

for f in $(ls ./*.fbs);do
  flatc --no-includes -o ${INCLUDE_SCHEMAS_PATH}/account/ -c $f 
done
