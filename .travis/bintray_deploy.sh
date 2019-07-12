#! /bin/bash

user_info="$BINTRAY_USER:$BINTRAY_API_KEY"
build_dir="$TRAVIS_BUILD_DIR/Build/Release"
filename="ni-daq.dll"
repo="oe-plugins"
package="ni-daq"

version="0.5"

cd $build_dir
curl -T $filename --user $user_info https://api.bintray.com/content/$BINTRAY_USER/$repo/$package/$version/$filename