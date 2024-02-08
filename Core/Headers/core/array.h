#pragma once

#include "core/csharp.h"
#include "memory.h"
#include "pointer.h"

// TEMPLATE
#define _EXPAND_STRUCT_NAME(type) _array_##type

#define _ARRAY_DEFINE_STRUCT(type) struct _EXPAND_STRUCT_NAME(type)\
{\
	/* The pointer to the backing array */\
	type* Values;\
	/* The size, in bytes, of the backing array*/\
	size_t Size;\
	/* The size in bytes between elements of the array\
	// This would typically be the size of the element stored */\
	size_t ElementSize;\
	/* the size in elements that this array can store\
	this is the same as Size/ElementSize */\
	size_t Capacity;\
	/* The number of elements stored in the array */\
	size_t Count;\
	/* The typeid of the element stored in the array */\
	size_t TypeId;\
	/* Whether or not this object was constructed on the stack */\
	bool StackObject;\
}; \
typedef struct _array_##type* type##_array;

#define stack_array(type,count, ...) &(struct _EXPAND_STRUCT_NAME(type))\
{\
	.Values = (type*) __VA_ARGS__,\
	.Size = sizeof(type) * count,\
	.ElementSize = sizeof(type),\
	.Capacity = count,\
	.Count = count,\
	.TypeId = 0,\
	.StackObject = true\
}

#define stack_string(string) stack_array(char, sizeof(string) - 1, string)

#define _EXPAND_array(type) type##_array
#define _EXPAND_Arrays(type) type##_array##Arrays

// Creates an array of the given type, remember to define the type if this fails to compile
#define array(type) _EXPAND_array(type)
// Convenience methods that can be used with an array(type)
#define Arrays(type) _EXPAND_Arrays(type)

_ARRAY_DEFINE_STRUCT(void);
typedef array(void) Array;

struct _arrayMethods
{
	Array(*Create)(size_t elementSize, size_t count, size_t typeId);
	void (*AutoResize)(Array);
	void (*Resize)(Array, size_t newCount);
	// Appends the given item to the end of the array
	void (*Append)(Array, void*);
	// Removes the given index, moving all contents to the left
	// in its place
	void (*RemoveIndex)(Array, size_t index);
	// Swaps the positions of two elements
	void (*Swap)(Array, size_t firstIndex, size_t secondIndex);
	// Insertion sorts given the provided comparator Func
	void (*InsertionSort)(Array, bool(comparator)(void* leftMemoryBlock, void* rightMemoryBlock));
	// Gets a pointer to the value contained at index
	void* (*At)(Array, size_t index);
	// Appends the given value array to the end of the given array
	void (*AppendArray)(Array array, Array appendedValue);
	void (*Clear)(Array array);
	void (*Foreach)(Array, void(*method)(void*));
	void (*ForeachWithContext)(Array, void* context, void(*method)(void* context, void* item));
	void (*Dispose)(Array);
};

extern const struct _arrayMethods Arrays;

// TEMPLATE FOR METHODS
#define DEFINE_ARRAY(type) _ARRAY_DEFINE_STRUCT(type)\
DEFINE_TYPE_ID(type##_array); \
private array(type) _array_##type##_Create(size_t count)\
{\
REGISTER_TYPE(type##_array); \
return (array(type))Arrays.Create(sizeof(type), count, type##_arrayTypeId); \
}\
private void _array_##type##_AutoResize(array(type) array)\
{\
Arrays.AutoResize((Array)array); \
}\
private void _array_##type##_Resize(array(type) array, size_t newCount)\
{\
Arrays.Resize((Array)array, newCount); \
}\
private void _array_##type##_Append(array(type)array, type value)\
{\
Arrays.Append((Array)array, &value); \
}\
private void _array_##type##_RemoveIndex(array(type) array, size_t index)\
{\
Arrays.RemoveIndex((Array)array, index); \
}\
private void _array_##type##_Swap(array(type) array, size_t firstIndex, size_t secondIndex)\
{\
Arrays.Swap((Array)array, firstIndex, secondIndex); \
}\
private void _array_##type##_InsertionSort(array(type) array, bool(comparator)(type* leftMemoryBlock, type* rightMemoryBlock))\
{\
Arrays.InsertionSort((Array)array, comparator); \
}\
private type* _array_##type##_At(array(type) array, size_t index)\
{\
return (type*)Arrays.At((Array)array, index); \
}\
private void _array_##type##_AppendArray(array(type) array, array(type) appendedValue)\
{\
Arrays.AppendArray((Array)array, (Array)appendedValue); \
}\
private void _array_##type##_Clear(array(type)array)\
{\
Arrays.Clear((Array)array); \
}\
private void _array_##type##_Foreach(array(type)array, void(*method)(type*))\
{\
Arrays.Foreach((Array)array, method); \
}\
private void _array_##type##_ForeachWithContext(array(type)array, void* context, void(*method)(void* context, type* item))\
{\
Arrays.ForeachWithContext((Array)array, context, method);\
}\
private void _array_##type##_Dispose(array(type)array)\
{\
Arrays.Dispose((Array)array); \
}\
const static struct _array_##type##_methods\
{\
array(type) (*Create)(size_t count); \
void (*AutoResize)(array(type)); \
void (*Resize)(array(type), size_t newCount); \
void (*Append)(array(type), type); \
void (*RemoveIndex)(array(type), size_t index); \
void (*Swap)(array(type), size_t firstIndex, size_t secondIndex); \
void (*InsertionSort)(array(type), bool(comparator)(type* left, type* right)); \
type* (*At)(array(type), size_t index); \
void (*AppendArray)(array(type), array(type) appendedValue); \
void (*Clear)(array(type));\
void (*Foreach)(array(type), void(*method)(type*));\
void (*ForeachWithContext)(array(type), void* context, void(*method)(void*, type*));\
void (*Dispose)(array(type)); \
} type##_array##Arrays = \
{\
.Create = _array_##type##_Create, \
.AutoResize = _array_##type##_AutoResize, \
.Resize = _array_##type##_Resize, \
.Append = _array_##type##_Append, \
.RemoveIndex = _array_##type##_RemoveIndex, \
.Swap = _array_##type##_Swap, \
.InsertionSort = _array_##type##_InsertionSort, \
.At = _array_##type##_At, \
.AppendArray = _array_##type##_AppendArray, \
.Clear = _array_##type##_Clear,\
.Foreach = _array_##type##_Foreach,\
.ForeachWithContext = _array_##type##_ForeachWithContext,\
.Dispose = _array_##type##_Dispose\
};

// define common types
DEFINE_ARRAY(bool);
DEFINE_ARRAY(char);
DEFINE_ARRAY(int);
DEFINE_ARRAY(float);
DEFINE_ARRAY(double);
DEFINE_ARRAY(size_t);

DEFINE_ARRAY(char_array);

#define DEFINE_TUPLE_FULL(left,leftname,right,rightname) struct tuple_##left##_##right{ left leftname;right rightname;}
#define DEFINE_TUPLE(left,right) DEFINE_TUPLE_FULL(left,First,right,Second)
#define tuple(left,right) struct tuple_##left##_##right

#define DEFINE_TUPLE_BOTH_WAYS(T1,T2) DEFINE_TUPLE(T1,T2); DEFINE_TUPLE(T2,T1);
#define DEFINE_TUPLE_ALL(major,T1,T2,T3,T4,T5,T6) DEFINE_TUPLE(major,major);DEFINE_TUPLE(major, T1);DEFINE_TUPLE(major, T2);DEFINE_TUPLE(major, T3);DEFINE_TUPLE(major, T4);DEFINE_TUPLE(major, T5);DEFINE_TUPLE(major, T6);

DEFINE_TUPLE_ALL(bool, char, int, long, size_t, float, double)
DEFINE_TUPLE_ALL(char, bool, int, long, size_t, float, double)
DEFINE_TUPLE_ALL(int, bool, char, long, size_t, float, double)
DEFINE_TUPLE_ALL(long, bool, char, int, size_t, float, double)
DEFINE_TUPLE_ALL(size_t, bool, char, int, long, float, double)
DEFINE_TUPLE_ALL(float, bool, char, int, long, size_t, double)
DEFINE_TUPLE_ALL(double, bool, char, int, long, size_t, float)

#define DEFINE_TUPLE_ALL_INTRINSIC(type)\
DEFINE_TUPLE_BOTH_WAYS(type,bool);\
DEFINE_TUPLE_BOTH_WAYS(type,char);\
DEFINE_TUPLE_BOTH_WAYS(type,int);\
DEFINE_TUPLE_BOTH_WAYS(type,long);\
DEFINE_TUPLE_BOTH_WAYS(type,size_t);\
DEFINE_TUPLE_BOTH_WAYS(type,float);\
DEFINE_TUPLE_BOTH_WAYS(type,double);\

#define DEFINE_CONTAINERS(type) __pragma(warning(disable:4113)); DEFINE_ARRAY(type); DEFINE_TUPLE(type,type); DEFINE_TUPLE_ALL_INTRINSIC(type); DEFINE_POINTER(type); __pragma(warning(default:4113))

#pragma warning(disable:4113)
DEFINE_POINTER(_Bool);
DEFINE_POINTER(char);
DEFINE_POINTER(int);
DEFINE_POINTER(short);
DEFINE_POINTER(long);
DEFINE_POINTER(size_t);
DEFINE_POINTER(float);
DEFINE_POINTER(double);
#pragma warning(default: 4113)
