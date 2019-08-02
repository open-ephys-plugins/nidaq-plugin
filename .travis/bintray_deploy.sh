#! /bin/bash

user_info="$BINTRAY_USER:$BINTRAY_API_KEY"
build_dir="$TRAVIS_BUILD_DIR/Build/Release"
filename="nidaq-plugin.dll"
repo="open-ephys-plugins"
package="ni-daq"

version="0.1"

cd $build_dir
curl -T $filename --user $user_info https://api.bintray.com/content/$BINTRAY_USER/$repo/$package/$version/$filename