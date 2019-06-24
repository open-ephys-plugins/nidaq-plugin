cd ..
git clone -b cmakeBuild https://github.com/aacuevas/plugin-GUI.git
cd plugin-GUI/Build
cmake ..
MSBuild.exe open-ephys.vcxproj 
cd ../..
cd ni-daq/
sh ./create_build_files.sh
cd Build
MSBuild.exe NI-DAQmx.vcxproj