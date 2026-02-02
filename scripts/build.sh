# use this script, if you want build project. You can find executable files in /build

BUILD_DIR="../build"

#if build dir exist, then rebuild project
if [ -d "$BUILD_DIR" ]; then
    cmake --build $BUILD_DIR --clean-first -j$(nproc)
fi 

#build 
mkdir -p "$BUILD_DIR"
cmake -S .. -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" -j$(nproc)

#run tests
cd $BUILD_DIR 
ctest


