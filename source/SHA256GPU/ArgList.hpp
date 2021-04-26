#pragma once

#include <string>
#include <vector>


template<typename ItemType>
struct LinkedNode
{
public:
	ItemType arg;
	LinkedNode<ItemType>* next;

	LinkedNode(ItemType arg)
	{
		this->arg = arg;
		this->next = nullptr;
	}

	int calcLength()
	{
		if (next == nullptr) return 1;
		else return next->calcLength() + 1;
	}

	void add(ItemType item)
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
	void dump(std::vector<ItemType>* v) const
	{
		v->push_back(arg);
		if (next != nullptr)
		{
			next->dump(v);
		}
	}
	ItemType fetchProperty(LinkedNode<ItemType>* previous, ItemType marker)
	{
		if (arg == marker) //Found
		{
			if (next == nullptr) //No value
			{
				previous->next = nullptr;
				delete this;
				return "-";
			}
			else //Return value
			{
				previous->next = this->next->next;

				ItemType res = this->next->arg;

				delete this->next;
				delete this;

				return res;
			}
		}
		else if (next != nullptr) //Move next
		{
			return next->fetchProperty(this, marker);
		}
		else //Not found
		{
			return "";
		}
	}
};

struct ArgList
{
private:
	int length;
	LinkedNode<std::string>* start;

	int calcLength()
	{
		return start->calcLength();
	}
public:
	//Construct
	ArgList()
	{
		this->start = nullptr;
		this->length = 0;
	}
	~ArgList()
	{
		while (length > 0)
		{
			pop();
			length--;
		}
	}


	//Modify
	void add(std::string item)
	{
		if (start == nullptr)
		{
			start = new LinkedNode<std::string>(item);
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

		LinkedNode<std::string>* next = start->next;
		delete start;
		start = next;
		length--;
	}
	std::string fetchProperty(std::string marker)
	{
		if (marker == "" || start == nullptr || start->next == nullptr)
		{
			return "";
		}

		std::string res = start->next->fetchProperty(start, marker);
		length = calcLength();
		return res;
	}


	//Query
	std::string get() const
	{
		return (start == nullptr) ? std::string("") : start->arg;
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
	void dump(std::vector<std::string>& v) const
	{
		if (start == nullptr) return;

		start->dump(&v);
	}
};
