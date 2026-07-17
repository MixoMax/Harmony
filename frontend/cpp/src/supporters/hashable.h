#pragma once

#ifndef HASHABLE_H
#define HASHABLE_H


class Hashable
{
public:
	virtual int toHash() = 0;
};

#endif // !HASHABLE_H