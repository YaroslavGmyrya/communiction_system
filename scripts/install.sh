sudo apt install -y doxygen graphviz pre-commit
sudo apt install libsdl2-dev libgl1-mesa-dev libglew-dev # for ImGui

# Install zmqpp (C++ version zmq)

cd ../third_party
sudo apt install -y git cmake build-essential

git clone https://github.com/zeromq/zmqpp.git
cd zmqpp
mkdir build && cd build
cmake ..
make -j
sudo make install
sudo ldconfig
cd ../
rm -rf zmqpp


pre-commit install