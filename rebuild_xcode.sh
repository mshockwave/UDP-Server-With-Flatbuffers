#!/bin/bash

cd build

cmake $@ -G "Xcode" ..

cd -
