#ifndef BUFFER_INL
#define BUFFER_INL

#include <cstring> // memcpy
#include <cassert> // assert

// The memory is allocated using new instead of malloc because the class is
// templated and we want to call the constructor of T in the general case.

template<typename T>
Buffer2D<T>::Buffer2D(): sizeX(0), sizeY(0), data(nullptr){
}

template<typename T>
Buffer2D<T>::Buffer2D(const unsigned int& i_sizeX, const unsigned int& i_sizeY)
: sizeX(i_sizeX), sizeY(i_sizeY){

	if(sizeX * sizeY){
		data = new T[sizeX * sizeY];
	}else{
		data = nullptr;
	}
}

template<typename T>
Buffer2D<T>::Buffer2D(const unsigned int& i_sizeX, const unsigned int& i_sizeY, const T& value)
: sizeX(i_sizeX), sizeY(i_sizeY){

	if(sizeX * sizeY){
		data = new T[sizeX * sizeY];

		for(unsigned int ivalue = 0; ivalue != sizeX * sizeY; ++ivalue){
			data[ivalue] = value;
		}

	}else{
		data = nullptr;
	}
}

template<typename T>
Buffer2D<T>::Buffer2D(const Buffer2D<T>& src)
: sizeX(src.sizeX), sizeY(src.sizeY){

	if(src.data){
		data = new T[sizeX * sizeY];
		std::memcpy(data, src.data, sizeof(T) * sizeX * sizeY);

	}else{
		data = nullptr;
	}
}

template<typename T>
Buffer2D<T>::Buffer2D(Buffer2D<T>&& src)
: sizeX(src.sizeX), sizeY(src.sizeY), data(src.data){

	src.data = nullptr;
}

template<typename T>
template<typename otherT>
Buffer2D<T>::Buffer2D(const Buffer2D<otherT>& src)
: sizeX(src.sizeX), sizeY(src.sizeY){

	if(src.data){
		data = new T[sizeX * sizeY];

		for(unsigned int iT = 0; iT != sizeX * sizeY; ++iT){
			data[iT] = src.data[iT];
		}

	}else{
		data = nullptr;
	}
}

template<typename T>
Buffer2D<T>::~Buffer2D(){
	if(data){
		delete[] data;
	}
}

template<typename T>
Buffer2D<T>& Buffer2D<T>::operator=(const Buffer2D<T>& src){

	// modifying allocation if needed
	if((sizeX * sizeY) != (src.sizeX * src.sizeY)){
		if(data){
			delete[] data;
		}

		if(src.sizeX * src.sizeY){
			data = new T[src.sizeX * src.sizeY];
		}else{
			data = nullptr;
		}
	}

	if(data){
		std::memcpy(data, src.data, sizeof(T) * src.sizeX * src.sizeY);
	}

	sizeX = src.sizeX;
	sizeY = src.sizeY;

	return *this;
}

template<typename T>
Buffer2D<T>& Buffer2D<T>::operator=(Buffer2D<T>&& src){

	// switching data
	T* temp = data;
	data = src.data;
	src.data = temp;

	sizeX = src.sizeX;
	sizeY = src.sizeY;

	return *this;
}

template<typename T>
template<typename otherT>
Buffer2D<T>& Buffer2D<T>::operator=(const Buffer2D<otherT>& src){

	// modifying allocation if needed
	if((sizeX * sizeY) != (src.sizeX * src.sizeY)){
		if(data){
			delete[] data;
		}

		if(src.sizeX * src.sizeY){
			data = new T[src.sizeX * src.sizeY];
		}else{
			data = nullptr;
		}
	}

	if(data){
		for(unsigned int iT = 0; iT != sizeX * sizeY; ++iT){
			data[iT] = src.data[iT];
		}
	}

	sizeX = src.sizeX;
	sizeY = src.sizeY;

	return *this;
}

template<typename T>
inline T Buffer2D<T>::operator[](const unsigned int& index) const{
	assert(index < sizeX * sizeY);
	return data[index];
}

template<typename T>
inline T& Buffer2D<T>::operator[](const unsigned int& index){
	assert(index < sizeX * sizeY);
	return data[index];
}

template<typename T>
inline T Buffer2D<T>::operator()(const unsigned int& x, const unsigned int& y) const{
	assert(x < sizeX && y < sizeY);
	return data[y * sizeX + x];
}

template<typename T>
inline T& Buffer2D<T>::operator()(const unsigned int& x, const unsigned int& y){
	assert(x < sizeX && y < sizeY);
	return data[y * sizeX + x];
}

template<typename T>
Buffer2D<T>& Buffer2D<T>::fill(const T& value){
	for(unsigned int iT = 0; iT != sizeX * sizeY; ++iT){
		data[iT] = value;
	}

	return *this;
}

template<typename T>
void Buffer2D<T>::resize(const unsigned int& i_sizeX, const unsigned int& i_sizeY){

	// modifying alocation if needed
	if((i_sizeX * i_sizeY) != (sizeX * sizeY)){
		if(data){
			delete[] data;
		}

		if(i_sizeX * i_sizeY){
			data = new T[i_sizeX * i_sizeY];
		}else{
			data = nullptr;
		}
	}

	sizeX = i_sizeX;
	sizeY = i_sizeY;
}

template<typename T>
T Buffer2D<T>::min() const{
	assert(data != nullptr);

	T min_value = data[0];

	for(unsigned int iT = 1; iT != sizeX * sizeY; ++iT){
		if(data[iT] < min_value){
			min_value = data[iT];
		}
	}

	return min_value;
}

template<typename T>
T Buffer2D<T>::max() const{
	assert(data != nullptr);

	T max_value = data[0];

	for(unsigned int iT = 1; iT != sizeX * sizeY; ++iT){
		if(data[iT] > max_value){
			max_value = data[iT];
		}
	}

	return max_value;
}

#endif
