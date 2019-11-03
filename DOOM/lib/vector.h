#pragma once

#include <cassert>
#include <memory.h>
#include <malloc.h>
#include <cstddef>
#include <math.h>

#define MAX(a, b) (a > b? a : b)

namespace m{

	namespace vector
	{
		template <typename T>
		class vector
		{
			T* arr{ nullptr };
			size_t _size{ 0 };
			size_t _capacity{ 0 };

			void copy(T* first, T* second) {
				for (int i = 0; i < _size; ++i)
					first[i] = second[i];
			}

			void grow(size_t const& new_capacity = 0, bool unspecific = true) { // false if i want concret size
				assert(new_capacity > 0);
				if (new_capacity < _size && new_capacity >= 0) {
					_size = new_capacity;
					return;
				}

				size_t new_cap = 0;
				if (unspecific)
					new_cap = MAX(_capacity * 2 + 1, new_capacity);
				else
					new_cap = new_capacity;

				assert(_capacity < new_cap);
				if (arr) {
					T* new_arr = new T[new_cap];
					copy(new_arr, arr);
					delete[] arr;
					arr = new_arr;
				}
				else {
					arr = new T[new_cap];
				}
				_capacity = new_cap;
			}

		public:

			inline vector() = default;

			inline vector(T* arr, size_t size) : _size(size), _capacity(size) {
				this->arr = new T[size];
				copy(this->arr, arr);
			}

			inline vector(size_t const& capacity) {
				grow(capacity, false);
			}

			inline vector(size_t const& size, T elem) {
				_size = size;
				grow(_size);
				fill(elem);
			}

			inline vector(vector&& other) {
				delete[] arr;
				_size = other._size;
				_capacity = other._capacity;
				arr = other.arr;
				other.arr = nullptr;
				other._capacity = 0;
				other._size = 0;
			}

			inline vector& operator = (vector&& other) {
				delete[] arr;
				arr = other.arr;
				_size = other._size;
				_capacity = other._capacity;
				other.arr = nullptr;
				other._capacity = 0;
				other._size = 0;
				return*this;
			}

			inline vector(vector const& other) {
				_size = other._size;
				_capacity = other._capacity;
				delete[] arr;
				arr = new T[_capacity];
				copy(arr, other.arr);
			}

			~vector() {
				delete[] arr;
			}

			inline vector& operator = (vector const& other) {
				if (this == &other)
					return *this;
				_size = other._size;
				_capacity = other._capacity;
				delete[] arr;
				if (other._size != 0) // improvization remove func if vector<vector<int>> a;  a[0] = T();
				{
					arr = new T[_capacity];
					copy(arr, other.arr);
				}
				return *this;
			}

			inline void operator << (T const& elem) {  // use it instead push_back if u watch anime
				push_back(elem);
			}

			inline void push_back(T elem) {
				if (_capacity <= _size)
					grow(_size + 1);
				arr[_size++] = elem;
			}

			inline void remove(size_t const& pos) {
				assert(pos >= 0 && pos < _size);
				arr[pos] = T();                 // very bad move but qiuck
				if (pos != _size - 1)
					memmove(arr + pos, arr + pos + 1, sizeof(T) * (_size - pos));
				_size -= 1;
			}

			inline void resize(size_t new_size, T elem) {
				grow(new_size);
				if (new_size > _size)
					fill(elem, _size, new_size - 1);
				else
					fill(elem, 0, new_size - 1);

				_size = new_size;
			}

			inline void reserve(size_t const& new_capacity) {
				grow(new_capacity);
			}

			inline void clear() {
				_size = 0;
			}

			inline T& operator [] (size_t const& i) {
				assert(i >= 0 && i <= _size);
				return arr[i];
			}

			inline T& At(size_t const& i) {
				return operator[](i);
			}

			inline void insert(int place, T elem) {
				assert(place >= 0 && place <= _size);
				if (++_size > _capacity)
					grow(_size);

				memmove(arr + place + 1, arr + place, sizeof(T) * (_size - place));
				arr[place] = elem;
			}

			inline void fill(T elem, size_t start = -1, size_t end = -1) {
				if (start == -1 && end == -1) {
					for (int i = 0; i < _size; i++) {
						arr[i] = elem;
					}
					return;
				}

				assert(start >= 0 && start < _capacity && end >= 0 && end < _capacity && start <= end);
				for (size_t i = start; i < end + 1; i++) {
					arr[i] = elem;
				}
			}

			inline T& front() {
				return At(0);
			}

			inline T& back() {
				return At(_size - 1);
			}

			inline T* begin() {
				return arr;
			}

			inline T* end() {
				return arr + _size - 1;
			}

			inline size_t size() {
				return _size;
			}

			inline size_t capacity() {
				return _capacity;
			}

			inline bool empty() {
				return _size == 0 ? true : false;
			}

			// stack

			inline T pop() {
				return At(--_size);
			}

			inline T& peak() {
				return At(_size - 1);
			}

			inline void push(T const& elem) {
				push_back(elem);
			}

		};
	}
}