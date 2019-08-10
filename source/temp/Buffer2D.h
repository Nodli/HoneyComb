#ifndef BUFFER_H
#define BUFFER_H

// The allocation is row major
// The origin (0, 0) of the buffer is at the top left corner
template <typename T>
struct Buffer2D{

	// ---- contructor ---- //

	Buffer2D();
	Buffer2D(const unsigned int& sizeX, const unsigned int& sizeY);
	Buffer2D(const unsigned int& sizeX, const unsigned int& sizeY, const T& value);

	Buffer2D(const Buffer2D<T>& src);
	Buffer2D(Buffer2D<T>&& src);

    // conversion from another template type
	template<typename otherT>
	Buffer2D(const Buffer2D<otherT>& src);

	~Buffer2D();

	// ---- operator ---- //

	Buffer2D<T>& operator=(const Buffer2D<T>& src);
	Buffer2D<T>& operator=(Buffer2D<T>&& src);

    // conversion from another template type
	template<typename otherT>
	Buffer2D<T>& operator=(const Buffer2D<otherT>& src);

	// access a value using its index
	inline T operator[](const unsigned int& index) const;
	inline T& operator[](const unsigned int& index);

	// access a value using its 2D coordinates
	inline T operator()(const unsigned int& x, const unsigned int& y) const;
	inline T& operator()(const unsigned int& x, const unsigned int& y);

	// ---- storage related methods ---- //

	// set all data values to the provided value
	Buffer2D<T>& fill(const T& value);

    // The size variables and memory allocation are updated to reflect the new size
    // The memory is reallocated only if
    // sizeX * sizeY != previous_sizeX * previous_sizeY
    // but in the general case you should assume that the content is lost
	void resize(const unsigned int& sizeX, const unsigned int& sizeY);

	// ---- math operations ---- //

	T min() const;
	T max() const;

	// ---- data ---- //

	unsigned int sizeX;
	unsigned int sizeY;

	T* data;
};

#include "Buffer2D.inl"

#endif
