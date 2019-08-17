#define TEST
#include "eval.h"
#include "vktest.h"
#include "util.h"

void test_int_calc() {
  VK_TEST_SECTION_BEGIN("integer calculation test")

  stack_t test_stack;
  init_stack(&test_stack);
  stack_enter_frame(&test_stack);

  stkobj_t *stack_a = stack_get(&test_stack, create_string("a"));
  stkobj_t *stack_b = stack_get(&test_stack, create_string("b"));

  stack_a->soid = SOID_INT;
  stack_a->value.ivalue = 1;

  stack_b->soid = SOID_INT;
  stack_b->value.ivalue = 2;

  heapobj_t *heapobj = heap_alloc_int(33);

  plvalue_t sum_ab = algebraic_calc(create_onstack(stack_a),
                                     create_onstack(stack_b), ALF_ADD);

  VK_ASSERT_EQUALS(ROC_TEMP, sum_ab.roc);
  VK_ASSERT_EQUALS(JT_INT,    sum_ab.type);
  VK_ASSERT_EQUALS(3,         sum_ab.value.ivalue);

  plvalue_t sum_abc = algebraic_calc(sum_ab, create_onheap(heapobj),
                                      ALF_ADD);

  VK_ASSERT_EQUALS(ROC_TEMP, sum_abc.roc);
  VK_ASSERT_EQUALS(JT_INT,    sum_abc.type);
  VK_ASSERT_EQUALS(36,        sum_abc.value.ivalue);

  close_stack(&test_stack);
  
  VK_TEST_SECTION_END("integer calculation test")
}

void test_str_add() {
  VK_TEST_SECTION_BEGIN("string add test")

  stack_t test_stack;
  init_stack(&test_stack);
  stack_enter_frame(&test_stack);

  stkobj_t *stack_a = stack_get(&test_stack, create_string("a"));
  stkobj_t *stack_b = stack_get(&test_stack, create_string("b"));

  stack_a->soid = SOID_STR;
  stack_a->value.svalue = create_string("Hello, ");

  stack_b->soid = SOID_STR;
  stack_b->value.svalue = create_string("world!");

  plvalue_t sum_ab = algebraic_calc(create_onstack(stack_a),
                                    create_onstack(stack_b), ALF_ADD);

  VK_ASSERT_EQUALS(ROC_TEMP, sum_ab.roc);
  VK_ASSERT_EQUALS(JT_STR,   sum_ab.type);
  VK_ASSERT_EQUALS(create_string("Hello, world!"), sum_ab.value.svalue);

  close_stack(&test_stack);

  VK_TEST_SECTION_END("string add test")
}

int main() {
  VK_TEST_BEGIN

  init_heap();

  test_int_calc();
  test_str_add();

  gc_start();
  gc_cleanup();

  VK_TEST_END
  return 0;
}
