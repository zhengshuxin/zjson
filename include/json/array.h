#ifndef	ARRAY_INCLUDE_H
#define	ARRAY_INCLUDE_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "json_define.h"
#include "iterator.h"

/**
 * 动态数组类型定义
 */
typedef	struct ARRAY ARRAY;

struct ARRAY{
	int     capacity;	/**< items 数组空间大小 */
	int     count;		/**< items 中含有元素的个数 */
	void  **items;		/**< 动态数组 */

	/* for iterator */

	/* 取迭代器头函数 */
	void *(*iter_head)(ITER*, struct ARRAY*);
	/* 取迭代器下一个函数 */
	void *(*iter_next)(ITER*, struct ARRAY*);
	/* 取迭代器尾函数 */
	void *(*iter_tail)(ITER*, struct ARRAY*);
	/* 取迭代器上一个函数 */
	void *(*iter_prev)(ITER*, struct ARRAY*);
};

/**
 * 创建一个动态数组
 * @param init_size {int} 动态数组的初始大小
 * @return {ARRAY*} 动态数组指针
 */
JSON_API ARRAY *array_create(int init_size);

/**
 * 释放掉动态数组内的成员变量，但并不释放动态数组对象
 * @param a {ARRAY*} 动态数组指针
 * @param free_fn {void (*)(void*)} 用于释放动态数组内成员变量的释放函数指针
 */
JSON_API void array_clean(ARRAY *a, void (*free_fn)(void *));

/**
 * 释放掉动态数组内的成员变量，并释放动态数组对象，当数组对象创建 dbuf 对象
 * 时，则该数组对象的释放将会在释放 dbuf 时被释放
 * @param a {ARRAY*} 动态数组指针
 * @param free_fn {void (*)(void*)} 用于释放动态数组内成员变量的释放函数指针
 */
JSON_API void array_free(ARRAY *a, void (*free_fn)(void *));

/**
 * 向动态数组尾部添加动态成员变量
 * @param a {ARRAY*} 动态数组指针
 * @param obj {void*} 动态成员变量
 * @return {int} >=0: 成功, 返回值为该元素在数组中的下标位置；-1: 失败
 */
JSON_API int array_append(ARRAY *a, void *obj);

/**
 * 从动态数组中的某个下标位置取出动态对象
 * @param a {ARRAY*} 动态数组指针
 * @param idx {int} 下标位置，不能越界，否则返回-1
 * @return {void*} != NULL: 成功；== NULL: 不存在或失败
 */
JSON_API void *array_index(const ARRAY *a, int idx);

/**
 * 获得当前动态数组中动态对象的个数
 * @param a {ARRAY*} 动态数组指针
 * @return {int} 动态数组中动态对象的个数
 */
JSON_API int array_size(const ARRAY *a);

#ifdef  __cplusplus
}
#endif

#endif
