/*
*	The Buddy Page Allocator
*	SKELETON IMPLEMENTATION TO BE FILLED IN FOR TASK 2
*/

#include <infos/mm/page-allocator.h> #include <infos/mm/mm.h>
#include <infos/kernel/kernel.h> #include <infos/kernel/log.h> #include <infos/util/math.h> #include <infos/util/printf.h>

using namespace infos::kernel; using namespace infos::mm; using namespace infos::util;
#define MAX_ORDER	18

/**
*	A buddy page allocation algorithm.
*/
class BuddyPageAllocator : public PageAllocatorAlgorithm
{
private:
PageDescriptor *_free_areas[MAX_ORDER+1]; int location[262144] = {-1};

unsigned int log2(uint64_t x){ unsigned int result = 0; while (x >>= 1) result++; return result;
}

/**
*	Inserts a block into the free list of the given order. The block is inserted in ascending order.
*	@param pgd The page descriptor of the block to insert.
*	@param order The order in which to insert the block.
*	@return Returns the slot (i.e. a pointer to the pointer that points to the block) that the block
*	was inserted into.
*/
PageDescriptor **insert_block(PageDescriptor *pgd, int order)
{
assert(pgd);
// Starting from the _free_area array, find the slot in which the page descriptor
// should be inserted.
PageDescriptor **slot = &_free_areas[order]; location[sys.mm().pgalloc().pgd_to_pfn(pgd)] = order;

// Iterate whilst there is a slot, and whilst the page descriptor pointer is numerically
// greater than what the slot is pointing to.
if(*slot == nullptr || pgd < *slot){
pgd->next_free = _free_areas[order];
if(_free_areas[order]) _free_areas[order]->prev_free = pgd;
_free_areas[order] = pgd; return &_free_areas[order];
 
}
else{
 


while((*slot)->next_free && pgd > (*slot)->next_free){ slot = &((*slot)->next_free);
 
}

pgd->prev_free = *slot;
pgd->next_free = (*slot)->next_free;
if(pgd->next_free) pgd->next_free->prev_free = pgd; (*slot)->next_free = pgd;

return &((*slot)->next_free);
}
}

void print_free_areas(int order){ mm_log.messagef(LogLevel::DEBUG, "order = %d", order); PageDescriptor **slot = &_free_areas[order]; while((*slot)->next_free) slot = &(*slot)->next_free;
//PageDescriptor *p = _free_areas[order];
while(*slot){
mm_log.messagef(LogLevel::DEBUG, "pfn = %d", sys.mm().pgalloc().pgd_to_pfn(*slot));
//p = p->next_free;
slot = &(*slot)->prev_free;
}
}

/**
*	Removes a block from the free list of the given order. The block MUST be present in the free-list, otherwise
*	the system will panic.
*	@param pgd The page descriptor of the block to remove.
 
*	@param order The order in which to remove the block from.
*/
void remove_block(PageDescriptor *pgd, int order)
{
PageDescriptor **slot = &_free_areas[order]; location[sys.mm().pgalloc().pgd_to_pfn(pgd)] = -1;

// Starting from the _free_area array, iterate until the block has been located in the linked-list.
while (*slot && pgd != *slot) { slot = &(*slot)->next_free;
}

// Make sure the block actually exists. Panic the system if it does not.
assert(*slot == pgd);
mm_log.messagef(LogLevel::DEBUG, "Evil removal xxxx %d",sys.mm().pgalloc().pgd_to_pfn(pgd));

// Remove the block from the free list.
if(*slot) (*slot)->prev_free = pgd->prev_free;
*slot = pgd->next_free; pgd->next_free = NULL; pgd->prev_free = NULL;
}

/** Given a page descriptor, and an order, returns the buddy PGD. The buddy could either be
*	to the left or the right of PGD, in the given order.
*	@param pgd The page descriptor to find the buddy for.
*	@param order The order in which the page descriptor lives.
*	@return Returns the buddy of the given page descriptor, in the given order.
*/
PageDescriptor *buddy_of(PageDescriptor *pgd, int order)
{
// TODO: Implement me!
// wrong parameter
if(order >= MAX_ORDER) return nullptr; assert(pgd);
uint64_t num_of_pages = (1<<order);
uint64_t pfn = sys.mm().pgalloc().pgd_to_pfn(pgd);

if( !(pfn % (1<<(order+1))) ){
if(pgd->next_free && pgd->next_free == pgd + num_of_pages){ return (pgd + num_of_pages);
}
}
else if(pgd->prev_free && pgd->prev_free == pgd - num_of_pages){ return (pgd - num_of_pages);
}
mm_log.messagef(LogLevel::DEBUG, "FAIL");

return nullptr;
}

/**
*	Given a pointer to a block of free memory in the order "source_order", this function will
*	split the block in half, and insert it into the order below.
*	@param block_pointer A pointer to a pointer containing the beginning of a block of free memory.
*	@param source_order The order in which the block of free memory exists. Naturally,
*	the split will insert the two new blocks into the order below.
*	@return Returns the left-hand-side of the new block.
*/
PageDescriptor *split_block(PageDescriptor **block_pointer, int source_order)
{
// TODO: Implement me!
// Unable to split if it is order zero, or there is no free_area
if(!source_order){
mm_log.messagef(LogLevel::DEBUG, "Cannot split a block of 1 page"); return nullptr;
}
if(_free_areas[source_order] == nullptr){
mm_log.messagef(LogLevel::DEBUG, "Cannot split! This order is empty"); return nullptr;
}
assert(*block_pointer);
uint64_t pfn = sys.mm().pgalloc().pgd_to_pfn(*block_pointer); uint64_t num_of_pages = (1<<(source_order-1));

remove_block(*block_pointer, source_order);
PageDescriptor **block_B = insert_block(*block_pointer + num_of_pages, source_order - 1); block_pointer = insert_block(*block_pointer, source_order - 1);

return *block_pointer;
}

/**
*	Takes a block in the given source order, and merges it (and its buddy) into the next order.
*	@param block_pointer A pointer to a pointer containing a block in the pair to merge.
*	@param source_order The order in which the pair of blocks live.
 
*	@return Returns the new slot that points to the merged block.
*/
PageDescriptor **merge_block(PageDescriptor **block_pointer, int source_order)
{
// TODO: Implement me!
// Unable to merge if it is MAX_ORDER, or wrong order
if(source_order >= MAX_ORDER){
mm_log.messagef(LogLevel::DEBUG, "Cannot merge! unfeasible order for merging"); return nullptr;
}
if(_free_areas[source_order] == nullptr){
mm_log.messagef(LogLevel::DEBUG, "Cannot merge! This order is free"); return nullptr;
}
assert(*block_pointer);
uint64_t num_of_pages = (1<<source_order);
uint64_t pfn = sys.mm().pgalloc().pgd_to_pfn(*block_pointer);

// pgd is not the leftmost one
*block_pointer = *block_pointer - (pfn % num_of_pages);

PageDescriptor *Buddy = buddy_of(*block_pointer, source_order); if(Buddy == nullptr) return nullptr;

remove_block(Buddy, source_order); remove_block(*block_pointer, source_order);
PageDescriptor *insertion = (Buddy < *block_pointer)? Buddy : *block_pointer;
//
if(sys.mm().pgalloc().pgd_to_pfn(*block_pointer) == 0){
mm_log.messagef(LogLevel::DEBUG, "Focus here	= %d", sys.mm().pgalloc().pgd_to_pfn(Buddy));

}
return insert_block(insertion, source_order + 1);
}

 
public:
 

/**
*	Allocates 2^order number of contiguous pages
*	@param order The power of two, of the number of contiguous pages to allocate.
*	@return Returns a pointer to the first page descriptor for the newly allocated page range, or NULL if
*	allocation failed.
*/
PageDescriptor *allocate_pages(int order) override
{
// TODO: Implement me!
int counter = order;
while(counter < MAX_ORDER && !_free_areas[counter]){ counter++;
 
}

PageDescriptor* Split = _free_areas[counter]; while(counter > order){
Split = split_block(&Split, counter); counter--;
}
remove_block(Split, counter);

return Split;
}

 
/**
 

*	Frees 2^order contiguous pages.
*	@param pgd A pointer to an array of page descriptors to be freed.
*	@param order The power of two number of contiguous pages to free.
*/
 
void free_pages(PageDescriptor *pgd, int order) override
{
// TODO: Implement me!
if(location[sys.mm().pgalloc().pgd_to_pfn(pgd)] == -1){ insert_block(pgd, order);
}
PageDescriptor **Merged; while(order < MAX_ORDER){
Merged = merge_block(&pgd, order); if(Merged == nullptr) break; order++;
}

}

/**
*	Marks a range of pages as available for allocation.
*	@param start A pointer to the first page descriptors to be made available.
*	@param count The number of page descriptors to make available.
*/
virtual void insert_page_range(PageDescriptor *start, uint64_t count) override
 
{
// TODO: Implement me!
int order;
uint64_t num_of_pages; PageDescriptor *pgd = start; while(count > 0){
for(order = log2(count); order >= 0; --order){ if(order > 18) order = 18; num_of_pages = (1<<order);
if(!(sys.mm().pgalloc().pgd_to_pfn(pgd) % num_of_pages)){
PageDescriptor** trash = insert_block(pgd, order); //insert_block add a loop to make the next_free and
 
prev_free according to num_of_pages



}
 

count = count - num_of_pages; pgd = pgd + num_of_pages; break;
 
count--;
}

}
// For testing insert_page function
// while(count > 0){
// PageDescriptor** trash = insert_block(start + count, 0);
// count--;
// }
}

/**
*	Marks a range of pages as unavailable for allocation.
*	@param start A pointer to the first page descriptors to be made unavailable.
*	@param count The number of page descriptors to make unavailable.
*/
virtual void remove_page_range(PageDescriptor *start, uint64_t count) override
{
// TODO: Implement me!
PageDescriptor *pgd = start; uint64_t pfn, num_of_pages; while(count > 0){
pfn = sys.mm().pgalloc().pgd_to_pfn(pgd);

for(int order = MAX_ORDER; order >= 0; --order){ num_of_pages = (1<<order);

if(!order || location[pfn - pfn % num_of_pages] == order){
// This is the head, and the count is ok with removing the whole block
if(!order || (!(pfn % num_of_pages) && count >= num_of_pages)){ remove_block(pgd, order);
print_free_areas(order); count = count - num_of_pages; pgd = pgd + num_of_pages; break;
}
// Keep spliting until you can remove
else{
PageDescriptor *Head = pgd - pfn % num_of_pages; split_block(&Head, order);
}
}
}
}
}

/**
*	Initialises the allocation algorithm.
*	@return Returns TRUE if the algorithm was successfully initialised, FALSE otherwise.
*/
bool init(PageDescriptor *page_descriptors, uint64_t nr_page_descriptors) override
{
//TODO: Implement me!
for(auto &pgd: _free_areas ) pgd = NULL;
return true;
}

/**
*	Returns the friendly name of the allocation algorithm, for debugging and selection purposes.
*/
const char* name() const override { return "buddy"; }

/**
*	Dumps out the current state of the buddy system
*/
void dump_state() const override
{
// Print out a header, so we can find the output in the logs.
mm_log.messagef(LogLevel::DEBUG, "BUDDY STATE:");
 
// Iterate over each free area.
for (unsigned int i = 0; i < ARRAY_SIZE(_free_areas); i++) { char buffer[256];
snprintf(buffer, sizeof(buffer), "[%d] ", i);

// Iterate over each block in the free area.
PageDescriptor *pg = _free_areas[i]; while (pg) {
// Append the PFN of the free block to the output buffer.
snprintf(buffer, sizeof(buffer), "%s%lx ", buffer, sys.mm().pgalloc().pgd_to_pfn(pg)); pg = pg->next_free;
}

mm_log.messagef(LogLevel::DEBUG, "%s", buffer);
}
}
};

/* --- DO NOT CHANGE ANYTHING BELOW THIS LINE --- */

/*
* Allocation algorithm registration framework
*/
RegisterPageAllocator(BuddyPageAllocator);
