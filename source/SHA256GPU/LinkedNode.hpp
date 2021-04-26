#pragma once

template<typename ItemType>
struct LinkedNode
{
public:
	ItemType item;
	LinkedNode<ItemType>* next;

	/**
	 * Creates an instance of LinkedNode with the following item.
	 */
	LinkedNode(ItemType item)
	{
		this->item = item;
		this->next = nullptr;
	}

	/**
	 * Calculates the length of the list chain from this node.
	 * @return The length.
	 */
	int calcLength()
	{
		if (next == nullptr) return 1;
		else return next->calcLength() + 1;
	}

	/**
	 * Creates a LinkedNode with the given item and adds it to the end of the chain of this LinkedNode.
	 * @param item Item to add.
	 */
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

	/**
	 * Adds the item in this node to the given vector and calls dump on the followin LinkedNode if not null.
	 * @param v Vector to dump the items to.
	 */
	void dump(std::vector<ItemType>* v) const
	{
		v->push_back(item);
		if (next != nullptr)
		{
			next->dump(v);
		}
	}

	/**
	 * Search the LinkedNode with the given marker and remove the marker from the chain with it's property.
	 * @param previous Previous LinkedNode.
	 * @param marker Marker to search for.
	 * @return Returns the property found by the chain or an empty string if not found, or a dash if the property was invalid or not found.
	 */
	ItemType fetchProperty(LinkedNode<ItemType>* previous, ItemType marker)
	{
		if (item == marker) //Found
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

				ItemType res = this->next->item;

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