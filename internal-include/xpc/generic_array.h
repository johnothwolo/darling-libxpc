#ifndef _XPC_GENERIC_ARRAY_H_
#define _XPC_GENERIC_ARRAY_H_

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <pthread/pthread.h>

#define XPC_GENARR_APPEND SIZE_MAX

/**
 * Type-generic array.
 */

#define XPC_GENARR_STRUCT(name, type) \
	struct xpc_genarr_ ## name ## _s { \
		bool resize_by_factor; \
		size_t size; \
		size_t length; \
		type* array; \
		xpc_genarr_ ## name ## _item_destructor_f item_dtor; \
	}

#define XPC_GENARR_INITIALIZER(_resize_by_factor, _item_dtor) { \
		.resize_by_factor = _resize_by_factor, \
		.size = 0, \
		.length = 0, \
		.array = NULL, \
		.item_dtor = _item_dtor, \
	}

#define XPC_GENARR_DECL(name, type, static) \
	typedef struct xpc_genarr_ ## name ## _s xpc_genarr_ ## name ## _t; \
	typedef void (*xpc_genarr_ ## name ## _item_destructor_f)(type* item); \
	typedef bool (*xpc_genarr_ ## name ## _iterator_f)(void* context, size_t index, type* value); \
	static void xpc_genarr_ ## name ## _init(xpc_genarr_ ## name ## _t* genarr, bool resize_by_factor, xpc_genarr_ ## name ## _item_destructor_f item_dtor); \
	static void xpc_genarr_ ## name ## _destroy(xpc_genarr_ ## name ## _t* genarr); \
	static bool xpc_genarr_ ## name ## _get(xpc_genarr_ ## name ## _t* genarr, size_t index, type* value); \
	static bool xpc_genarr_ ## name ## _set(xpc_genarr_ ## name ## _t* genarr, size_t index, type const* value); \
	static bool xpc_genarr_ ## name ## _append(xpc_genarr_ ## name ## _t* genarr, type const* value); \
	static bool xpc_genarr_ ## name ## _remove(xpc_genarr_ ## name ## _t* genarr, size_t index); \
	static bool xpc_genarr_ ## name ## _insert(xpc_genarr_ ## name ## _t* genarr, size_t index, type const* value); \
	static size_t xpc_genarr_ ## name ## _length(xpc_genarr_ ## name ## _t* genarr); \
	static type* xpc_genarr_ ## name ## _data(xpc_genarr_ ## name ## _t* genarr); \
	static void xpc_genarr_ ## name ## _iterate(xpc_genarr_ ## name ## _t* genarr, void* context, xpc_genarr_ ## name ## _iterator_f iterator);

#define XPC_GENARR_SEARCH_DECL(name, type, static) \
	typedef struct xpc_genarr_ ## name ## _search_context_s { \
		size_t index; \
		type const* target; \
	} xpc_genarr_ ## name ## _search_context_t; \
	static bool xpc_genarr_ ## name ## _search_iterator(void* context, size_t index, type* value); \
	static size_t xpc_genarr_ ## name ## _find(xpc_genarr_ ## name ## _t* genarr, type const* target); \

#define XPC_GENARR_BLOCKS_DECL(name, type, static) \
	typedef bool (^xpc_genarr_ ## name ## _block_iterator_t)(size_t index, type* value); \
	static void xpc_genarr_ ## name ## _block_iterate(xpc_genarr_ ## name ## _t* genarr, xpc_genarr_ ## name ## _block_iterator_t block_iterator);


#define XPC_GENARR_DEF(name, type, static) \
	static void xpc_genarr_ ## name ## _init(xpc_genarr_ ## name ## _t* genarr, bool resize_by_factor, xpc_genarr_ ## name ## _item_destructor_f item_dtor) { \
		genarr->resize_by_factor = resize_by_factor; \
		genarr->size = 0; \
		genarr->length = 0; \
		genarr->array = NULL; \
		genarr->item_dtor = item_dtor; \
	}; \
	static void xpc_genarr_ ## name ## _destroy(xpc_genarr_ ## name ## _t* genarr) { \
		for (size_t i = 0; i < genarr->length; ++i) { \
			if (genarr->item_dtor) { \
				genarr->item_dtor(&genarr->array[i]); \
			} \
		} \
		if (genarr->array) {\
			free(genarr->array); \
		} \
		genarr->array = NULL; \
		genarr->length = 0; \
		genarr->size = 0; \
	}; \
	static bool xpc_genarr_ ## name ## _get(xpc_genarr_ ## name ## _t* genarr, size_t index, type* value) { \
		bool result = false; \
		if (index < genarr->length) { \
			if (value) { \
				memcpy(value, &genarr->array[index], sizeof(type)); \
			} \
			result = true; \
		} \
		return result; \
	}; \
	static bool xpc_genarr_ ## name ## _set(xpc_genarr_ ## name ## _t* genarr, size_t index, type const* value) { \
		bool result = false; \
		if (index < genarr->length) { \
			if (genarr->item_dtor) { \
				genarr->item_dtor(&genarr->array[index]); \
			} \
			memcpy(&genarr->array[index], value, sizeof(type)); \
			result = true; \
		} \
		return result; \
	}; \
	static bool xpc_genarr_ ## name ## _append(xpc_genarr_ ## name ## _t* genarr, type const* value) { \
		bool result = false; \
		if (genarr->size < genarr->length + 1) { \
			size_t new_size = 0; \
			type* new_array = NULL; \
			if (genarr->resize_by_factor) { \
				new_size = (genarr->size == 0) ? 1 : genarr->size * 2; \
			} else { \
				new_size = genarr->length + 1; \
			} \
			new_array = realloc(genarr->array, sizeof(type) * new_size); \
			if (new_array) { \
				genarr->array = new_array; \
				genarr->size = new_size; \
			} \
		} \
		if (genarr->size >= genarr->length + 1) { \
			if (value) { \
				memcpy(&genarr->array[genarr->length++], value, sizeof(type)); \
			} \
			result = true; \
		} \
		return result; \
	}; \
	static bool xpc_genarr_ ## name ## _remove(xpc_genarr_ ## name ## _t* genarr, size_t index) { \
		bool result = false; \
		size_t new_size = genarr->size; \
		if (index < genarr->length) { \
			if (genarr->item_dtor) { \
				genarr->item_dtor(&genarr->array[index]); \
			} \
			memmove(&genarr->array[index], &genarr->array[index + 1], (genarr->length - index - 1) * sizeof(type)); \
			--genarr->length; \
			result = true; \
		} \
		if (genarr->resize_by_factor && genarr->length <= genarr->size / 2) { \
			new_size = genarr->size / 2; \
		} else if (!genarr->resize_by_factor && genarr->length < genarr->size) { \
			new_size = genarr->length; \
		} \
		if (new_size != genarr->size) { \
			type* new_array = realloc(genarr->array, sizeof(type) * new_size); \
			if (new_array) { \
				genarr->array = new_array; \
				genarr->size = new_size; \
			} \
		} \
		return result; \
	}; \
	static bool xpc_genarr_ ## name ## _insert(xpc_genarr_ ## name ## _t* genarr, size_t index, type const* value) { \
		bool result = false; \
		if (index == genarr->length || index == XPC_GENARR_APPEND) { \
			result = xpc_genarr_ ## name ## _append(genarr, value); \
		} else if (index < genarr->length) { \
			result = xpc_genarr_ ## name ## _append(genarr, NULL); \
			if (result) { \
				memmove(&genarr->array[index + 1], &genarr->array[index], (genarr->length - index - 2) * sizeof(type)); \
				if (value) { \
					memcpy(&genarr->array[index], value, sizeof(type)); \
				} \
			} \
		} \
		return result; \
	}; \
	static size_t xpc_genarr_ ## name ## _length(xpc_genarr_ ## name ## _t* genarr) { \
		return genarr->length; \
	}; \
	static type* xpc_genarr_ ## name ## _data(xpc_genarr_ ## name ## _t* genarr) { \
		return genarr->array; \
	}; \
	static void xpc_genarr_ ## name ## _iterate(xpc_genarr_ ## name ## _t* genarr, void* context, xpc_genarr_ ## name ## _iterator_f iterator) { \
		for (size_t i = 0; i < genarr->length; ++i) { \
			if (!iterator(context, i, &genarr->array[i])) { \
				break; \
			} \
		} \
	};

#define XPC_GENARR_SEARCH_DEF(name, type, static) \
	static bool xpc_genarr_ ## name ## _search_iterator(void* context, size_t index, type* value) { \
		xpc_genarr_ ## name ## _search_context_t* search_context = context; \
		if (memcmp(value, search_context->target, sizeof(type)) == 0) { \
			search_context->index = index; \
			return false; \
		} \
		return true; \
	}; \
	static size_t xpc_genarr_ ## name ## _find(xpc_genarr_ ## name ## _t* genarr, type const* target) { \
		xpc_genarr_ ## name ## _search_context_t search_context = { \
			.index = SIZE_MAX, \
			.target = target, \
		}; \
		xpc_genarr_ ## name ## _iterate(genarr, &search_context, xpc_genarr_ ## name ## _search_iterator); \
		return search_context.index; \
	}; \

#define XPC_GENARR_BLOCKS_DEF(name, type, static) \
	static void xpc_genarr_ ## name ## _block_iterate(xpc_genarr_ ## name ## _t* genarr, xpc_genarr_ ## name ## _block_iterator_t block_iterator) { \
		for (size_t i = 0; i < genarr->length; ++i) { \
			if (!block_iterator(i, &genarr->array[i])) { \
				break; \
			} \
		} \
	};

#endif // _XPC_GENERIC_ARRAY_H_
