cd ..
git clone -b cmakeBuild https://github.com/aacuevas/plugin-GUI.git
cd plugin-GUI/Build
cmake "Visual Studio 15 2017" Win64 ..
MSBuild.exe open-ephys.vcxproj 
cd ../..
cd ni-daq/Build
cmake "Visual Studio 15 2017" Win64 ..
MSBuild.exe INSTALL.vcxproj 