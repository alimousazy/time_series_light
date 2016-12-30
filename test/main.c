#include <assert.h>

// If unit testing is enabled override assert with mock_assert().
#if UNIT_TESTING
extern void mock_assert(const int result, const char* const expression, 
                        const char * const file, const int line);
#undef assert
#define assert(expression) \
    mock_assert((int)(expression), #expression, __FILE__, __LINE__);
#endif // UNIT_TESTING

void increment_value(int * const value) {
    assert(value);
    (*value) ++;
}

void decrement_value(int * const value) {
    if (value) {
        (*value) --;
    }
}
