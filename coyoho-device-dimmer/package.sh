#!/bin/bash
PROJECT_DIR=coyoho-device-dimmer
VERSION=$(cat version.txt)
mkdir -p build
rm -f build/*.zip
cd ..
zip ${PROJECT_DIR}/build/coyoho-device-dimmer-$VERSION.zip ${PROJECT_DIR} -r \
	--exclude ${PROJECT_DIR}/build/\* \
	--exclude ${PROJECT_DIR}/eagle/\*.b\#? \
	--exclude ${PROJECT_DIR}/eagle/\*.s\#?
cd ${PROJECT_DIR}
