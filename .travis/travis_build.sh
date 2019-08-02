cd ..
git clone -b development https://github.com/open-ephys/plugin-GUI.git
cd plugin-GUI/Build
cmake -G "Visual Studio 15 2017 Win64" ..
MSBuild.exe open-ephys.vcxproj //p:Configuration=Release //p:Platform=x64
cd ../..
cd nidaq-plugin/Build
cmake -G "Visual Studio 15 2017 Win64" ..
MSBuild.exe INSTALL.vcxproj //p:Configuration=Release //p:Platform=x64