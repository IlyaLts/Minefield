mkdir _RELEASE_
cd _RELEASE_

cmake -DCMAKE_BUILD_TYPE=Release .. -G "Unix Makefiles"
make
