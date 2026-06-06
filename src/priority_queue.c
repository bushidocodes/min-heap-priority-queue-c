#include "priority_queue.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>

/****************************
 * Private Helper Functions *
 ****************************/

/**
 * Adds a value to the end of the binary heap
 * @param self the priority queue
 * @param new_item the value we are adding
 * @return 0 on success. -1 when priority queue is full
 **/
static inline int
priority_queue_append(struct priority_queue *const self, void *new_item)
{
	assert(self != NULL);

	if (self->first_free >= PRIORITY_QUEUE_CAPACITY) return -1;

	self->items[self->first_free] = new_item;
	self->first_free++;
	return 0;
}

/**
 * Shifts an appended value upwards to restore heap structure property
 * @param self the priority queue
 */
static inline void
priority_queue_percolate_up(struct priority_queue *const self)
{
	assert(self != NULL);
	assert(self->get_key != NULL);

	for (size_t i = self->first_free - 1;
	     i / 2 != 0 && self->get_key(self->items[i]) < self->get_key(self->items[i / 2]); i /= 2) {
		void *temp         = self->items[i / 2];
		self->items[i / 2] = self->items[i];
		self->items[i]     = temp;
		// If percolated to highest priority, update highest priority
		if (i / 2 == 1) self->min_key = self->get_key(self->items[1]);
	}
}

/**
 * Returns the index of a node's smallest child
 * @param self the priority queue
 * @param parent_index
 * @returns the index of the smallest child
 */
static inline size_t
priority_queue_find_smallest_child(const struct priority_queue *const self, size_t parent_index)
{
	assert(self != NULL);
	assert(parent_index >= 1 && parent_index < self->first_free);
	assert(self->get_key != NULL);

	size_t left_child_index  = 2 * parent_index;
	size_t right_child_index = 2 * parent_index + 1;
	assert(self->items[left_child_index] != NULL);

	// If we don't have a right child or the left child is smaller, return it
	if (right_child_index == self->first_free) {
		return left_child_index;
	} else if (self->get_key(self->items[left_child_index])
	           < self->get_key(self->items[right_child_index])) {
		return left_child_index;
	} else {
		// Otherwise, return the right child
		return right_child_index;
	}
}

/**
 * Shifts the top of the heap downwards. Used after placing the last value at
 * the top
 * @param self the priority queue
 */
static inline void
priority_queue_percolate_down(struct priority_queue *const self)
{
	assert(self != NULL);
	assert(self->get_key != NULL);

	size_t parent_index     = 1;
	size_t left_child_index = 2 * parent_index;
	while (left_child_index >= 2 && left_child_index < self->first_free) {
		size_t smallest_child_index = priority_queue_find_smallest_child(self, parent_index);
		// Once the parent is equal to or less than its smallest child, break;
		if (self->get_key(self->items[parent_index])
		    <= self->get_key(self->items[smallest_child_index]))
			break;
		// Otherwise, swap and continue down the tree
		void *temp                        = self->items[smallest_child_index];
		self->items[smallest_child_index] = self->items[parent_index];
		self->items[parent_index]         = temp;

		parent_index     = smallest_child_index;
		left_child_index = 2 * parent_index;
	}
}

/*********************
 * Public API        *
 *********************/

/**
 * Initialized the Priority Queue Data structure
 * @param self the priority_queue to initialize
 * @param get_key pointer to a function that extracts the ordering key from an element
 **/
void
priority_queue_initialize(struct priority_queue *const self, priority_queue_get_key_t get_key)
{
	assert(self != NULL);
	assert(get_key != NULL);

	memset(self->items, 0, sizeof(void *) * PRIORITY_QUEUE_CAPACITY);
	self->first_free   = 1;
	self->get_key = get_key;

	self->min_key = UINT64_MAX;
}

/**
 * Removes all elements from the priority queue, preserving the get_key
 * callback so the queue can be reused without reinitializing.
 * @param self the priority queue to clear
 **/
void
priority_queue_clear(struct priority_queue *const self)
{
	assert(self != NULL);

	memset(self->items, 0, sizeof(void *) * PRIORITY_QUEUE_CAPACITY);
	self->first_free       = 1;
	self->min_key = UINT64_MAX;
}

/**
 * @param self the priority_queue
 * @returns the number of elements in the priority queue
 **/
size_t
priority_queue_length(const struct priority_queue *const self)
{
	assert(self != NULL);

	return self->first_free - 1;
}

/**
 * @param self the priority queue
 * @returns the minimum-priority element without removing it, or NULL if empty
 **/
void *
priority_queue_peek(const struct priority_queue *const self)
{
	assert(self != NULL);

	if (self->first_free == 1) return NULL;
	return self->items[1];
}

/**
 * @param self - the priority queue we want to add to
 * @param value - the value we want to add
 * @returns 0 on success. -1 when priority queue is full
 **/
int
priority_queue_enqueue(struct priority_queue *const self, void *value)
{
	assert(self != NULL);

	if (value == NULL) return -1;
	if (priority_queue_append(self, value) == -1) return -1;
	if (self->first_free > 2) {
		priority_queue_percolate_up(self);
	} else {
		// If this is the first element we add, update the highest priority
		self->min_key = self->get_key(value);
	}
	return 0;
}

/**
 * @param self - the priority queue we want to add to
 * @returns The head of the priority queue or NULL when empty
 **/
void *
priority_queue_dequeue(struct priority_queue *const self)
{
	assert(self != NULL);
	assert(self->get_key != NULL);
	// If first_free is 1, we're empty
	if (self->first_free == 1) return NULL;

	void *min                         = self->items[1];
	self->items[1]                    = self->items[self->first_free - 1];
	self->items[self->first_free - 1] = NULL;
	self->first_free--;
	assert(self->first_free == 1 || self->items[self->first_free - 1] != NULL);
	// Because of 1-based indices, first_free is 2 when there is only one element
	if (self->first_free > 2) priority_queue_percolate_down(self);

	if (self->first_free > 1) {
		self->min_key = self->get_key(self->items[1]);
	} else {
		self->min_key = UINT64_MAX;
	}
	return min;
}

/**
 * @param self the priority queue
 * @returns true if the priority queue contains no elements
 **/
bool
priority_queue_is_empty(const struct priority_queue *const self)
{
	assert(self != NULL);

	return self->first_free == 1;
}

/**
 * @param self the priority queue
 * @returns true if the priority queue has reached maximum capacity
 **/
bool
priority_queue_is_full(const struct priority_queue *const self)
{
	assert(self != NULL);

	return self->first_free >= PRIORITY_QUEUE_CAPACITY;
}
