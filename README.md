# Project 4: Fast, Efficient Redundancy Elimination

## Producer/Consumer Implementation

Our producer/consumer implementation is built using a lock-based concurrent queue. It features a push and a pop operation that are used by the producer and consumer, respectively.

## Cache Implementation

Our cache is built with an array of node structures that each contain a packetholder and a next pointer. Our packetholders contain a valid bit, a 2400-byte char array for the packet data, an unsigned hash integer for the hashed payload, an unsigned bytes holder for the number of bytes read, and an integer to keep track of how many windows we have read up to the current point (for level 2). 
The next pointer is used for separate chaining, which is implemented so that if two payloads hash to the same bucket in our cache but don't have the same exact data, they can each be placed in that bucket in the form of a linked list. The hashing function that we used was the Hash Little hashlittle() implementation, as detailed in the FAQ's for the assignment.

## Eviction Strategy

For level 2, we used a First In, First Out eviction method to keep our cache under 64 MB. If one of the small packet windows maps to a bucket that already contains a linked list of data, then the head of that list is removed and freed, and the new packet window is placed at the end of the list. If the small packet window had mapped to a bucket that didn't contain any data, then we would search from the beginning of the cache until we found a bucket with data and eliminate that bucket's first node.

## Level 1

Overall, our process for level 1 was to loop through each file, and for each packet in the file we would hash bytes 52-onward and place it into a bucket in our cache. Separate chaining was used to keep track of packets that hashed to the same bucket but didn't have the same exact data.

## Level 2

Overall, our process was to loop through each file, and for each packet in the file we would hash bytes 52-115, 53-116, 54-117, etc. until we reached the end of the file. Each of these packet windows would be hashed, and we would check our cache for duplicate data. Only if the packet window was at least 32 bytes away from the last point of cache insertion would we insert the new packet window into our cache. We also ran a check for every cache insertion to make sure our data structure never exceeded 64 MB, and if it did, we ran our eviction strategy described above. 