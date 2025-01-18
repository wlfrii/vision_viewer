#ifndef H_WLF_A6599508_DED1_433F_9E85_A0EDE711A1DA
#define H_WLF_A6599508_DED1_433F_9E85_A0EDE711A1DA
#include <atomic>

template <typename T>
class TripleBuffer
{
public:
	/**
	 * @brief Construct a new Triple Buffer< T> object
	 */
	TripleBuffer<T>();

	/**
	 * @brief Construct a new Triple Buffer< T> object
	 * 
	 * @param init Initial object.
	 */
	TripleBuffer<T>(const T& init);

	/**
	 * @brief Non-copyable.
	 */ 
	TripleBuffer<T>(const TripleBuffer<T>&) = delete;
	
	/**
	 * @brief Non-assignment.
	 */
	TripleBuffer<T>& operator=(const TripleBuffer<T>&) = delete;

	/**
	 * @brief wrapper to update with a new element (write + flipWriter)
	 */
	void update(T newT); 

	/**
	 * @brief wrapper to read the last available element (newSnap + snap)
	 */
	T& readNewest(); 
	
	/**
	 * @brief Clear buffer 
	 */
	void clear();

	/**
	 * @brief Get the index of the newest object, which is snap.
	 */
	int getNewestIndex();

		/**
	 * @brief Get the index of the oldest object, which is dirty.
	 */
	int getOldestIndex();

private:
	// Swap to the latest value, if any
	bool newSnap(); 
	// Check if the newWrite bit is 1
	bool hasNewWrite(uint_fast8_t flags); 
	// Implicitly set newWrite bit to 0 while swaping snap with clean indices
	uint_fast8_t swapSnapWithClean(uint_fast8_t flags); 
	// Set newWrite to 1 while swaping clean and dirty indices.
	uint_fast8_t newWriteSwapCleanWithDirty(uint_fast8_t flags); 

	// 8 bit flags are (unused) (new write) (2x dirty) (2x clean) (2x snap)
	// newWrite   = (flags & 0x40)
	// dirtyIndex = (flags & 0x30) >> 4
	// cleanIndex = (flags & 0xC) >> 2
	// snapIndex  = (flags & 0x3)
	mutable std::atomic_uint_fast8_t flags;

	T buffer[3];
};

/* include implementation in header since it is a template */

template <typename T>
TripleBuffer<T>::TripleBuffer(){

	T dummy = T();

	buffer[0] = dummy;
	buffer[1] = dummy;
	buffer[2] = dummy;

	flags.store(0x6, std::memory_order_relaxed); // initially dirty = 0, clean = 1 and snap = 2
}

template <typename T>
TripleBuffer<T>::TripleBuffer(const T& init){

	buffer[0] = init;
	buffer[1] = init;
	buffer[2] = init;

	flags.store(0x6, std::memory_order_relaxed); // initially dirty = 0, clean = 1 and snap = 2
}

template <typename T>
void TripleBuffer<T>::update(T newT){
	// write into dirty index
	buffer[(flags.load(std::memory_order_consume) & 0x30) >> 4] = newT; 

	// change dirty/clean buffer positions for the next update
	uint_fast8_t flagsnow(flags.load(std::memory_order_consume));
	while(!flags.compare_exchange_weak(flagsnow,
			  newWriteSwapCleanWithDirty(flagsnow),
			  std::memory_order_release,
			  std::memory_order_consume));
}

template <typename T>
T& TripleBuffer<T>::readNewest(){
	newSnap(); // get most recent value
	return buffer[flags.load(std::memory_order_consume) & 0x3]; // read snap index
}

template <typename T>
void TripleBuffer<T>::clear(){
	T dummy = T();
	this->update(dummy);
	this->update(dummy);
	this->update(dummy);
}

template <typename T>
int TripleBuffer<T>::getNewestIndex() {
	newSnap();
	return flags.load(std::memory_order_consume) & 0x3;
}

template <typename T>
int TripleBuffer<T>::getOldestIndex() {
	newSnap();
	return (flags.load(std::memory_order_consume) & 0x30) >> 4;
}

template <typename T>
bool TripleBuffer<T>::newSnap(){
	uint_fast8_t flagsnow(flags.load(std::memory_order_consume));
	do {
		// nothing new, no need to swap
		if(!hasNewWrite(flagsnow)) {
			return false;
		}
	} while(!flags.compare_exchange_weak(flagsnow,
			    swapSnapWithClean(flagsnow),
			    std::memory_order_release,
			    std::memory_order_consume));

	return true;
}

template <typename T>
bool TripleBuffer<T>::hasNewWrite(uint_fast8_t flags){
	// check if the newWrite bit is 1
	return ((flags & 0x40) != 0);
}

template <typename T>
uint_fast8_t TripleBuffer<T>::swapSnapWithClean(uint_fast8_t flags){
	// implicitly set new Write bit to 0 while swaping snap with clean
	return (flags & 0x30) | ((flags & 0x3) << 2) | ((flags & 0xC) >> 2);
}

template <typename T>
uint_fast8_t TripleBuffer<T>::newWriteSwapCleanWithDirty(uint_fast8_t flags){
	// set newWrite bit to 1 while swaping clean with dirty
	return 0x40 | ((flags & 0xC) << 2) | ((flags & 0x30) >> 2) | (flags & 0x3);
}

#endif /* H_WLF_A6599508_DED1_433F_9E85_A0EDE711A1DA */
