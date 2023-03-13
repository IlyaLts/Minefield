mkdir _DEBUG_
cd _DEBUG_

cmake -DCMAKE_BUILD_TYPE=Debug .. -G "Unix Makefiles"
make
