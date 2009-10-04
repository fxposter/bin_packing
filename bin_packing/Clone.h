#ifndef CLONE_H
#define CLONE_H

#include <cstddef>

template <class T> T* clone(const T* originalVector, size_t length)
{
	T* vector = new T[length];
	for (size_t i = 0; i < length; ++i) {
		vector[i] = originalVector[i];
	}
	return vector;
}


template <class T> T** clone(const T * const * originalMatrix, size_t rows, size_t columns)
{
	T** matrix = new T*[rows];
	for (size_t i = 0; i < rows; ++i) {
		matrix[i] = new T[columns];
		for (size_t j = 0; j < columns; ++j)
			matrix[i][j] = originalMatrix[i][j];
	}
	return matrix;
}

#endif // CLONE_H
