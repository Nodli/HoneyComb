clear
clear

echo "$1"
if [ "$1" == "clean" ]; then
	rm -r build
fi

mkdir -p build
cd build
cmake ..
make -j
