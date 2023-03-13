find . -name _DEBUG_ -exec rm -rf {} \;
find . -name _RELEASE_ -exec rm -rf {} \;

find -name '*.log' -delete;