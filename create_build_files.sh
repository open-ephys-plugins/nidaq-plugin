#!/bin/bash

#!/usr/bin/env bash

if [ "$(uname)" == "Darwin" ]; then
    # Do something under Mac OS X platform    
    echo "Detected Mac OS X";    
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    # Do something under GNU/Linux platform
    echo "Detected Linux"; 
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
    # Do something under 32 bits Windows NT platform
    echo "Detected Win32"; 
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW64_NT" ]; then
    # Do something under 64 bits Windows NT platform
    echo "Detected Win64"; 

    if [ -d VS2013 ]; then
		rm -rf VS2013
	fi
	mkdir VS2013 && cd VS2013
	cmake -G "Visual Studio 12 2013 Win64" ..
fi
