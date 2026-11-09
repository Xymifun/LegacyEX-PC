#include <stddef.h>

typedef void (*dfpt)(void);

typedef struct node_type
{
    struct node_type* next;
    void* object;
    dfpt dtor;
} node_type;

static node_type* list_head;

extern "C" void __call_dtors_single_list(node_type** node_list)
{
    while (*node_list != NULL)
    {
        node_type* node = *node_list;
        void* object = node->object;
        *node_list = node->next;
        if (object != NULL)
        {
            (*(void (*)(void*, int))node->dtor)(object, 2);
        }
        else
        {
            (*node->dtor)();
        }
    }
}

void __call_dtors(void)
{
    __call_dtors_single_list(&list_head);
}

extern "C" void __record_needed_destruction(node_type* node)
{
    if (node->next == NULL && node != list_head)
    {
        node->next = list_head;
        list_head = node;
    }
}
