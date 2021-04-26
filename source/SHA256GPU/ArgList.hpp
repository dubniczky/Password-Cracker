#pragma once

#include <string>
#include <vector>

#include "LinkedNode.hpp"


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
	/**
	 * Creates and empty ArgList
	 */
	ArgList()
	{
		this->start = nullptr;
		this->length = 0;
	}

	/**
	 * Deconstructs ArgList
	 */
	~ArgList()
	{
		while (length > 0)
		{
			pop();
			length--;
		}
	}


	//Modify
	/**
	 * Adds an item to the end of the ArgList.
	 * @param item Item to add.
	 */
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

	/**
	 * Removes the first item in the ArgList if it's not empty.
	 */
	void pop()
	{
		if (start == nullptr) return;

		LinkedNode<std::string>* next = start->next;
		delete start;
		start = next;
		length--;
	}

	/**
	 * Fetches the property associated with the given marker
	 * @param marker Marker to search for
	 * @return Returns the property found by the chain or an empty string if not found, or a dash if the property was invalid or not found.
	 */
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
	/**
	 * Returns the first item in the ArgList or an empty string.
	 * @return First item
	 */
	std::string get() const
	{
		return (start == nullptr) ? std::string("") : start->item;
	}

	/**
	 * Returns the number of items in the list.
	 * @return Count.
	 */
	int count() const
	{
		return length;
	}

	/**
	 * Returns if the first item in the ArgList matches the given string.
	 * @param match String to match.
	 * @return True if the given string matches the first item, false if no match or empty list.
	 */
	bool first(std::string match) const
	{
		if (start == nullptr) return false;
		else return match == start->item;
	}

	/**
	 * Collects the items in the ArgList and adds them to the given vector.
	 * @param v Target vector to dump the items to.
	 */
	void dump(std::vector<std::string>& v) const
	{
		if (start == nullptr) return;

		start->dump(&v);
	}
};
