#pragma once

namespace m{

	template <typename T>
	inline T* find(T* p, T* q, T elem)
	{
		T* needed = &elem;
		for (; p != q + 1; ++p)
			if (*needed == *p)
				needed = p;

		return needed == &elem ? nullptr : needed;
	}

	template <typename T>
	inline T* maxel(T* p, T* q)
	{
		T* max = p;
		for (; p != q + 1; ++p)
			if (*max <= *p)
				max = p;

		return max;
	}

	template <typename T>
	inline T* minel(T* p, T* q) {
		T* min = p;
		for (; p != q + 1; ++p)
			if (*min >= *p)
				return p;

		return nullptr;
	}

	template <typename T>
	inline T* lower_bound(T* p, T* q, T elem) {
		T* min = &elem;
		for (; p != q + 1; ++p)
			if (*min >= *p)
				return p;

		return nullptr;
	}

	template <typename T>
	inline T* uper_bound(T* p, T* q, T elem) {
		T* max = & elem;
		for (; p != q + 1; ++p)
			if (*max <= *p)
				max = p;

		return max == &elem ? nullptr : max;
	}



	template <typename T>
	inline void sort(T* left, T* right) {

		T* i = left, * j = right;
		T tmp;
		T pivot = left[(right - left) / 2];


		while (i <= j) {
			while (*i < pivot)
				i++;
			while (*j > pivot)
				j--;
			if (i <= j) {
				tmp = *i;
				*i = *j;
				*j = tmp;
				i++;
				j--;
			}
		};

		if (left < j)
			sort(left, j);
		if (i < right)
			sort(i, right);
	}


	template <typename T, typename F>
	inline void sort(T* left, T* right, F comp) {

		T* i = left, * j = right;
		T tmp;
		T pivot = left[(right - left) / 2];


		while (i <= j) {
			while (comp(*i, pivot))
				i++;
			while (comp(pivot, *j))
				j--;
			if (i <= j) {
				tmp = *i;
				*i = *j;
				*j = tmp;
				i++;
				j--;
			}
		};

		if (left < j)
			sort(left, j, comp);
		if (i < right)
			sort(i, right, comp);
	}

}