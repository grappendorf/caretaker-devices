#!/bin/bash
PROJECT_DIR=coyoho-xbeepandaadapter
VERSION=$(cat version.txt)
mkdir -p build
rm -f build/*.zip
cd ..
zip ${PROJECT_DIR}/build/coyoho-xbeepandaadapter-$VERSION.zip ${PROJECT_DIR} -r \
	--exclude ${PROJECT_DIR}/build/\* \
	--exclude ${PROJECT_DIR}/\*.b\#? \
	--exclude ${PROJECT_DIR}/\*.s\#?
cd ${PROJECT_DIR}
