#if !defined(NULL)
    #define NULL (void *)0
#endif

/*
 * Every allocation needs an 8-byte header to store the allocation size while
 * staying 8-byte aligned. The address returned by "malloc" is the address
 * right after this header (i.e. the size occupies the 8 bytes before the
 * returned address).
 */
#define HEADER_SIZE 8

/*
 * The minimum allocation size is 16 bytes because we have an 8-byte header and
 * we need to stay 8-byte aligned.
 */
#define MIN_ALLOC_LOG2 4
#define MIN_ALLOC ((unsigned long)1 << MIN_ALLOC_LOG2)

/*
 * The maximum allocation size is currently set to 2gb. This is the total size
 * of the heap. It's technically also the maximum allocation size because the
 * heap could consist of a single allocation of this size. But of course real
 * heaps will have multiple allocations, so the real maximum allocation limit
 * is at most 1gb.
 */
#define MAX_ALLOC_LOG2 31
#define MAX_ALLOC ((unsigned long)1 << MAX_ALLOC_LOG2)

/*
 * Allocations are done in powers of two starting from MIN_ALLOC and ending at
 * MAX_ALLOC inclusive. Each allocation size has a bucket that stores the free
 * list for that allocation size.
 *
 * Given a bucket index, the size of the allocations in that bucket can be
 * found with "(unsigned long)1 << (MAX_ALLOC_LOG2 - bucket)".
 */
#define BUCKET_COUNT (MAX_ALLOC_LOG2 - MIN_ALLOC_LOG2 + 1)

/*
 * Free lists are stored as circular doubly-linked lists. Every possible
 * allocation size has an associated free list that is threaded through all
 * currently free blocks of that size. That means MIN_ALLOC must be at least
 * "sizeof(list_t)". MIN_ALLOC is currently 16 bytes, so this will be true for
 * both 32-bit and 64-bit.
 */
typedef struct list_t {
  struct list_t *prev, *next;
} list_t;

/*
 * Each bucket corresponds to a certain allocation size and stores a free list
 * for that size. The bucket at index 0 corresponds to an allocation size of
 * MAX_ALLOC (i.e. the whole address space).
 */
list_t buckets[BUCKET_COUNT];

/*
 * We could initialize the allocator by giving it one free block the size of
 * the entire address space. However, this would cause us to instantly reserve
 * half of the entire address space on the first allocation, since the first
 * split would store a free list entry at the start of the right child of the
 * root. Instead, we have the tree start out small and grow the size of the
 * tree as we use more memory. The size of the tree is tracked by this value.
 */
unsigned long bucket_limit;

/*
 * This array represents a linearized binary tree of bits. Every possible
 * allocation larger than MIN_ALLOC has a node in this tree (and therefore a
 * bit in this array).
 *
 * Given the index for a node, lineraized binary trees allow you to traverse to
 * the parent node or the child nodes just by doing simple arithmetic on the
 * index:
 *
 * - Move to parent:         index = (index - 1) / 2;
 * - Move to left child:     index = index * 2 + 1;
 * - Move to right child:    index = index * 2 + 2;
 * - Move to sibling:        index = ((index - 1) ^ 1) + 1;
 *
 * Each node in this tree can be in one of several states:
 *
 * - UNUSED (both children are UNUSED)
 * - SPLIT (one child is UNUSED and the other child isn't)
 * - USED (neither children are UNUSED)
 *
 * These states take two bits to store. However, it turns out we have enough
 * information to distinguish between UNUSED and USED from context, so we only
 * need to store SPLIT or not, which only takes a single bit.
 *
 * Note that we don't need to store any nodes for allocations of size MIN_ALLOC
 * since we only ever care about parent nodes.
 */
unsigned char node_is_split[(1 << (BUCKET_COUNT - 1)) / 8];

/*
 * This is the starting address of the address range for this allocator. Every
 * returned allocation will be an offset of this pointer from 0 to MAX_ALLOC.
 */
unsigned char *base_ptr;

/*
 * This is the maximum address that has ever been used by the allocator. It's
 * used to know when to call "brk" to request more memory from the kernel.
 */
unsigned char *max_ptr;

int update_max_ptr(unsigned char *new_value);
void list_init(list_t *list);
void list_push(list_t *list, list_t *entry);
void list_remove(list_t *entry);
list_t *list_pop(list_t *list);
unsigned char *ptr_for_node(unsigned long index, unsigned long bucket);
unsigned long node_for_ptr(unsigned char *ptr, unsigned long bucket);
int parent_is_split(unsigned long index);
void flip_parent_is_split(unsigned long index);
unsigned long bucket_for_request(unsigned long request);
int lower_bucket_limit(unsigned long bucket);
void *malloc(unsigned long request);
void free(void *ptr);
