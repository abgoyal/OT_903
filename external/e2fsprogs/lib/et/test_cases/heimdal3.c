
#include <stdlib.h>

static const char * const text[] = {
	"Test message 1",
	"Test message 2",
    0
};

struct error_table {
    char const * const * msgs;
    long base;
    int n_msgs;
};
struct et_list {
    struct et_list *next;
    const struct error_table * table;
};
extern struct et_list *_et_list;

const struct error_table et_h3test_error_table = { text, 43787520L, 2 };

static struct et_list link = { 0, 0 };

void initialize_h3test_error_table_r(struct et_list **list);
void initialize_h3test_error_table(void);

void initialize_h3test_error_table(void) {
    initialize_h3test_error_table_r(&_et_list);
}

/* For Heimdal compatibility */
void initialize_h3test_error_table_r(struct et_list **list)
{
    struct et_list *et, **end;

    for (end = list, et = *list; et; end = &et->next, et = et->next)
        if (et->table->msgs == text)
            return;
    et = malloc(sizeof(struct et_list));
    if (et == 0) {
        if (!link.table)
            et = &link;
        else
            return;
    }
    et->table = &et_h3test_error_table;
    et->next = 0;
    *end = et;
}
