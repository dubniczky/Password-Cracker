#pragma once

#include <string>
#include <vector>


template<typename T>
struct LinkedNode
{
public:
	T arg;
	LinkedNode<T>* next;

	LinkedNode(T arg)
	{
		this->arg = arg;
		this->next = nullptr;
	}

	void add(T item)
	{
		if (next == nullptr)
		{
			next = new LinkedNode(item);
		}
		else
		{
			next->add(item);
		}
	}

	void dump(std::vector<T>* v) const
	{
		v->push_back(arg);
		if (next != nullptr)
		{
			next->dump(v);
		}
	}
};

template<typename T>
struct LinkedList
{
private:
	int length;
	LinkedNode<T>* start;
public:
	//Construct
	LinkedList()
	{
		this->start = nullptr;
		this->length = 0;
	}
	~LinkedList()
	{
		while (length > 0)
		{
			pop();
			length--;
		}
	}


	//Modify
	void add(T item)
	{
		if (start == nullptr)
		{
			start = new LinkedNode<T>(item);
		}
		else
		{
			start->add(item);
		}
		length++;
	}
	void pop()
	{
		if (start == nullptr) return;

		LinkedNode<T>* next = start->next;
		delete start;
		start = next;
		length--;
	}


	//Query
	T get() const
	{
		return (start == nullptr) ? T("") : start->arg;
	}
	int count() const
	{
		return length;
	}
	bool first(std::string s) const
	{
		if (start == nullptr) return false;
		else return s == start->arg;
	}
	void dump(std::vector<T>& v) const
	{
		if (start == nullptr) return;

		start->dump(&v);
	}
};
