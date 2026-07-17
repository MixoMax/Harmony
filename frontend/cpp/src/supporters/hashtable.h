#pragma once

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <type_traits>
#include "hashable.h"

template <typename T,
	typename = std::enable_if_t<std::is_base_of_v<Hashable, T>>>
class HashTable
{
public:
	struct Element{
		int idx = 0;
		T val;
	};

	Element EMPTY{-1};

	Element *entries;
	int tableLength;
	int elementCount;

	HashTable(int length) {
		tableLength = length;
		elementCount = 0;
		entries = new Element[tableLength];
		for (int i = 0; i < tableLength;i++) {
			*(entries + i) = EMPTY;
		}
	};

	~HashTable() {
		delete[] entries;
	};

	HashTable* add(T& elem) {
		if (elementCount >= tableLength) {
			std::cout << "Hash table is full" << std::endl;
			return this;
		}

		Element newElement;
		newElement.val = elem;
		newElement.idx = elem.toHash() % tableLength;
	
		while (entries[newElement.idx].idx != -1) {
			newElement.idx++;
			newElement.idx %= tableLength;
		}

		entries[newElement.idx] = newElement;

		++elementCount;

		return this;
	};

	HashTable* operator+=(const T& elem) {
		add(*elem);
		return this;
	};

	friend std::ostream& operator<<(std::ostream& os, const HashTable& hashtable) {
		os << "(" << std::endl;
		for (int i = 0; i < hashtable.tableLength; i++) {
			Element elem = hashtable.entries[i];
			if (elem.idx == -1) {
				continue;
			}
			os << elem.idx << " " << elem.val << std::endl;
		}
		os << ")";
		return os;
	};
};

#endif // !HASHTABLE_H