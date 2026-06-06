#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "vendor/unity.h"
#include "priority_queue.h"

struct sandbox_request {
	uint64_t absolute_deadline;
};

struct sandbox_request *
sandbox_request_allocate(uint64_t absolute_deadline)
{
	struct sandbox_request *sandbox_request = malloc(sizeof(struct sandbox_request));
	if (sandbox_request == NULL) abort();
	sandbox_request->absolute_deadline = absolute_deadline;
	return sandbox_request;
}

uint64_t
sandbox_request_get_key(void *element_raw)
{
	struct sandbox_request *element = (struct sandbox_request *)element_raw;
	return element->absolute_deadline;
}

struct priority_queue pq;

void
setUp(void)
{
	priority_queue_initialize(&pq, sandbox_request_get_key);
}

void
tearDown(void)
{
}

void
initialize_should_set_first_free_to_1(void)
{
	TEST_ASSERT_EQUAL_UINT(1, pq.first_free);
}

void
initialize_should_set_min_key_to_UINT64_MAX(void)
{
	TEST_ASSERT_EQUAL_UINT64(UINT64_MAX, pq.min_key);
}

void
length_should_be_one_less_than_first_free(void)
{
	TEST_ASSERT_EQUAL_UINT(pq.first_free - 1, priority_queue_length(&pq));
}

void
enqueue_should_increment_first_free_and_length(void)
{
	struct sandbox_request *sandbox_one = sandbox_request_allocate(10);
	TEST_ASSERT_EQUAL_INT(0, priority_queue_enqueue(&pq, sandbox_one));
	TEST_ASSERT_EQUAL_UINT(2, pq.first_free);
	TEST_ASSERT_EQUAL_UINT(1, priority_queue_length(&pq));
	free(sandbox_one);
}

void
enqueue_first_call_should_set_min_key(void)
{
	struct sandbox_request *sandbox_one = sandbox_request_allocate(10);
	TEST_ASSERT_EQUAL_INT(0, priority_queue_enqueue(&pq, sandbox_one));
	TEST_ASSERT_EQUAL_INT32(10, pq.min_key);
	free(sandbox_one);
}

void
enqueue_first_call_should_set_index_1(void)
{
	struct sandbox_request *sandbox_one = sandbox_request_allocate(10);
	TEST_ASSERT_EQUAL_INT(0, priority_queue_enqueue(&pq, sandbox_one));
	TEST_ASSERT_EQUAL_PTR(sandbox_one, pq.items[1]);
	free(sandbox_one);
}

void
enqueue_returns_neg1_on_null(void)
{
	TEST_ASSERT_EQUAL_INT(-1, priority_queue_enqueue(&pq, NULL));
	TEST_ASSERT_EQUAL_UINT(0, priority_queue_length(&pq));
}

void
enqueue_returns_neg1_on_full(void)
{
	struct sandbox_request *sandbox_one = sandbox_request_allocate(10);

	// Fill up the priority queue up to the max
	// This is one less than PRIORITY_QUEUE_CAPACITY because a min heap does not use the 0th element
	for (size_t i = 0; i < PRIORITY_QUEUE_CAPACITY - 1; i++) TEST_ASSERT_EQUAL_INT32(0, priority_queue_enqueue(&pq, sandbox_one));

	// And then add one more
	TEST_ASSERT_EQUAL_INT32(-1, priority_queue_enqueue(&pq, sandbox_one));

	free(sandbox_one);
}

void
dequeue_on_empty_returns_null(void)
{
	TEST_ASSERT_EQUAL_PTR(NULL, priority_queue_dequeue(&pq));
}

void
dequeue_last_element_should_set_UINT64_MAX(void)
{
	TEST_ASSERT_EQUAL_UINT64(UINT64_MAX, pq.min_key);
	struct sandbox_request *sandbox_one = sandbox_request_allocate(10);
	TEST_ASSERT_EQUAL_INT(0, priority_queue_enqueue(&pq, sandbox_one));
	TEST_ASSERT_LESS_THAN_UINT64(UINT64_MAX, pq.min_key);
	priority_queue_dequeue(&pq);
	free(sandbox_one);
	TEST_ASSERT_EQUAL_UINT64(UINT64_MAX, pq.min_key);
}

void
dequeue_of_one(void)
{
	struct sandbox_request *sandbox_one = sandbox_request_allocate(10);
	TEST_ASSERT_EQUAL_INT(0, priority_queue_enqueue(&pq, sandbox_one));
	TEST_ASSERT_EQUAL_PTR(sandbox_one, priority_queue_dequeue(&pq));
	free(sandbox_one);
}

void
dequeue_should_return_in_priority_order(void)
{
	struct sandbox_request *sandbox_7 = sandbox_request_allocate(7);
	TEST_ASSERT_EQUAL_INT(0, priority_queue_enqueue(&pq, sandbox_7));
	TEST_ASSERT_EQUAL_PTR(sandbox_7, pq.items[1]);
	TEST_ASSERT_EQUAL_UINT64(7, pq.min_key);

	struct sandbox_request *sandbox_9 = sandbox_request_allocate(9);
	TEST_ASSERT_EQUAL_INT(0, priority_queue_enqueue(&pq, sandbox_9));
	TEST_ASSERT_EQUAL_PTR(sandbox_7, pq.items[1]);
	TEST_ASSERT_EQUAL_PTR(sandbox_9, pq.items[2]);
	TEST_ASSERT_EQUAL_UINT64(7, pq.min_key);

	struct sandbox_request *sandbox_5 = sandbox_request_allocate(5);
	TEST_ASSERT_EQUAL_INT(0, priority_queue_enqueue(&pq, sandbox_5));
	TEST_ASSERT_EQUAL_PTR(sandbox_5, pq.items[1]);
	TEST_ASSERT_EQUAL_PTR(sandbox_9, pq.items[2]);
	TEST_ASSERT_EQUAL_PTR(sandbox_7, pq.items[3]);
	TEST_ASSERT_EQUAL_UINT64(5, pq.min_key);

	struct sandbox_request *sandbox_11 = sandbox_request_allocate(11);
	TEST_ASSERT_EQUAL_INT(0, priority_queue_enqueue(&pq, sandbox_11));
	TEST_ASSERT_EQUAL_PTR(sandbox_5, pq.items[1]);
	TEST_ASSERT_EQUAL_PTR(sandbox_9, pq.items[2]);
	TEST_ASSERT_EQUAL_PTR(sandbox_7, pq.items[3]);
	TEST_ASSERT_EQUAL_PTR(sandbox_11, pq.items[4]);
	TEST_ASSERT_EQUAL_UINT64(5, pq.min_key);

	struct sandbox_request *sandbox_2 = sandbox_request_allocate(2);
	TEST_ASSERT_EQUAL_INT(0, priority_queue_enqueue(&pq, sandbox_2));

	struct sandbox_request *state1[] = { NULL, sandbox_2, sandbox_5, sandbox_7, sandbox_11, sandbox_9 };
	TEST_ASSERT_EQUAL_PTR_ARRAY(state1, pq.items, 6);
	TEST_ASSERT_EQUAL_UINT64(2, pq.min_key);

	TEST_ASSERT_EQUAL_PTR(sandbox_2, priority_queue_dequeue(&pq));
	free(sandbox_2);
	struct sandbox_request *state2[] = { NULL, sandbox_5, sandbox_9, sandbox_7, sandbox_11 };
	TEST_ASSERT_EQUAL_PTR_ARRAY(state2, pq.items, 5);
	TEST_ASSERT_EQUAL_UINT64(5, pq.min_key);

	TEST_ASSERT_EQUAL_PTR(sandbox_5, priority_queue_dequeue(&pq));
	free(sandbox_5);
	struct sandbox_request *state3[] = { NULL, sandbox_7, sandbox_9, sandbox_11 };
	TEST_ASSERT_EQUAL_PTR_ARRAY(state3, pq.items, 4);
	TEST_ASSERT_EQUAL_UINT64(7, pq.min_key);

	TEST_ASSERT_EQUAL_PTR(sandbox_7, priority_queue_dequeue(&pq));
	free(sandbox_7);
	TEST_ASSERT_EQUAL_PTR(sandbox_9, pq.items[1]);
	TEST_ASSERT_EQUAL_PTR(sandbox_11, pq.items[2]);
	TEST_ASSERT_EQUAL_UINT64(9, pq.min_key);

	TEST_ASSERT_EQUAL_PTR(sandbox_9, priority_queue_dequeue(&pq));
	free(sandbox_9);
	TEST_ASSERT_EQUAL_PTR(sandbox_11, pq.items[1]);
	TEST_ASSERT_EQUAL_UINT64(11, pq.min_key);

	TEST_ASSERT_EQUAL_PTR(sandbox_11, priority_queue_dequeue(&pq));
	free(sandbox_11);
	TEST_ASSERT_EQUAL_PTR(NULL, pq.items[1]);
	TEST_ASSERT_EQUAL_UINT64(UINT64_MAX, pq.min_key);

	TEST_ASSERT_EQUAL_PTR(NULL, priority_queue_dequeue(&pq));
}

void
peek_on_empty_returns_null(void)
{
	TEST_ASSERT_NULL(priority_queue_peek(&pq));
}

void
peek_returns_min_without_removing(void)
{
	struct sandbox_request *sandbox_one = sandbox_request_allocate(10);
	struct sandbox_request *sandbox_two = sandbox_request_allocate(5);
	TEST_ASSERT_EQUAL_INT(0, priority_queue_enqueue(&pq, sandbox_one));
	TEST_ASSERT_EQUAL_INT(0, priority_queue_enqueue(&pq, sandbox_two));

	TEST_ASSERT_EQUAL_PTR(sandbox_two, priority_queue_peek(&pq));
	TEST_ASSERT_EQUAL_UINT(2, priority_queue_length(&pq));

	free(sandbox_one);
	free(sandbox_two);
}

void
is_empty_returns_true_on_empty_queue(void)
{
	TEST_ASSERT_TRUE(priority_queue_is_empty(&pq));
}

void
is_empty_returns_false_after_enqueue(void)
{
	struct sandbox_request *sandbox_one = sandbox_request_allocate(10);
	TEST_ASSERT_EQUAL_INT(0, priority_queue_enqueue(&pq, sandbox_one));
	TEST_ASSERT_FALSE(priority_queue_is_empty(&pq));
	free(sandbox_one);
}

void
is_full_returns_false_on_empty_queue(void)
{
	TEST_ASSERT_FALSE(priority_queue_is_full(&pq));
}

void
is_full_returns_true_when_at_capacity(void)
{
	struct sandbox_request *sandbox_one = sandbox_request_allocate(10);
	for (size_t i = 0; i < PRIORITY_QUEUE_CAPACITY - 1; i++) (void)priority_queue_enqueue(&pq, sandbox_one);
	TEST_ASSERT_TRUE(priority_queue_is_full(&pq));
	free(sandbox_one);
}

void
clear_empties_queue(void)
{
	struct sandbox_request *sandbox_one = sandbox_request_allocate(10);
	struct sandbox_request *sandbox_two = sandbox_request_allocate(5);
	TEST_ASSERT_EQUAL_INT(0, priority_queue_enqueue(&pq, sandbox_one));
	TEST_ASSERT_EQUAL_INT(0, priority_queue_enqueue(&pq, sandbox_two));
	TEST_ASSERT_FALSE(priority_queue_is_empty(&pq));

	priority_queue_clear(&pq);

	TEST_ASSERT_TRUE(priority_queue_is_empty(&pq));
	TEST_ASSERT_EQUAL_UINT64(UINT64_MAX, pq.min_key);
	free(sandbox_one);
	free(sandbox_two);
}

void
clear_preserves_get_key_callback(void)
{
	priority_queue_clear(&pq);
	TEST_ASSERT_EQUAL_PTR(sandbox_request_get_key, pq.get_key);
}

void
clear_allows_reuse(void)
{
	struct sandbox_request *sandbox_one = sandbox_request_allocate(10);
	TEST_ASSERT_EQUAL_INT(0, priority_queue_enqueue(&pq, sandbox_one));
	priority_queue_clear(&pq);

	struct sandbox_request *sandbox_two = sandbox_request_allocate(5);
	TEST_ASSERT_EQUAL_INT(0, priority_queue_enqueue(&pq, sandbox_two));
	TEST_ASSERT_EQUAL_PTR(sandbox_two, priority_queue_peek(&pq));
	TEST_ASSERT_EQUAL_UINT(1, priority_queue_length(&pq));

	free(sandbox_one);
	free(sandbox_two);
}

int
main(void)
{
	UnityBegin("priority_queue_test.c");
	RUN_TEST(initialize_should_set_first_free_to_1);
	RUN_TEST(initialize_should_set_min_key_to_UINT64_MAX);
	RUN_TEST(length_should_be_one_less_than_first_free);
	RUN_TEST(enqueue_first_call_should_set_min_key);
	RUN_TEST(enqueue_should_increment_first_free_and_length);
	RUN_TEST(enqueue_first_call_should_set_index_1);
	RUN_TEST(enqueue_returns_neg1_on_null);
	RUN_TEST(enqueue_returns_neg1_on_full);
	RUN_TEST(dequeue_on_empty_returns_null);
	RUN_TEST(dequeue_last_element_should_set_UINT64_MAX);
	RUN_TEST(dequeue_of_one);
	RUN_TEST(dequeue_should_return_in_priority_order);
	RUN_TEST(peek_on_empty_returns_null);
	RUN_TEST(peek_returns_min_without_removing);
	RUN_TEST(is_empty_returns_true_on_empty_queue);
	RUN_TEST(is_empty_returns_false_after_enqueue);
	RUN_TEST(is_full_returns_false_on_empty_queue);
	RUN_TEST(is_full_returns_true_when_at_capacity);
	RUN_TEST(clear_empties_queue);
	RUN_TEST(clear_preserves_get_key_callback);
	RUN_TEST(clear_allows_reuse);

	return UnityEnd();
}
