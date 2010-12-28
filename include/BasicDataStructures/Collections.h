#ifndef _Collections_h
#define _Collections_h

template <typename Type, typename CollectionState> struct Collection
{

};

template <typename CollectionState> struct Clearable
{
	void (*mpfClear)(CollectionState* in_pCollectionState);
};

template <typename Type, typename CollectionState> struct Set
{
	bool (*mpfContains)(CollectionState* in_pCollectionState, const Type* in_pcItem);
};

template <typename Type, typename CollectionState, typename IteratorState> struct List
{
	void (*mpfAppendToEnd)(CollectionState* state, Type t);
};

#endif
