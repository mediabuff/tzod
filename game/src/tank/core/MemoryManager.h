// MemoryManager.h
///////////////////////////////////////////

#pragma once

#include <stdlib.h>
#include <vector>

///////////////////////////////////////////////////////////////////

template
<
	class T,
	size_t block_size = 128
>
class MemoryManager1
{
	union BlankObject
	{
		BlankObject *pNext;
		char object[sizeof(T)];
	};

	std::vector<BlankObject *> _memoryBlocks;
	BlankObject *_freeBlock;

#ifdef _DEBUG
	size_t _allocatedCount;
	size_t _allocatedPeak;
#endif

	void Grow()
	{
		BlankObject *begin = (BlankObject *) malloc(sizeof(BlankObject) * block_size);
		BlankObject *end = begin + block_size;
		_memoryBlocks.push_back(begin);

		// link together newly allocated blanks
		do {
			begin->pNext = begin+1;
		} while( ++begin != end );

		(--end)->pNext = _freeBlock;
		_freeBlock = _memoryBlocks.back();
	}

public:
	MemoryManager1()
	  : _freeBlock(NULL)
#ifdef _DEBUG
	  , _allocatedCount(0)
	  , _allocatedPeak(0)
#endif
	{
	}

	~MemoryManager1()
	{
		for( size_t i = 0; i < _memoryBlocks.size(); i++ )
			::free(_memoryBlocks[i]);

#ifdef _DEBUG
		_ASSERT(0 == _allocatedCount);
	//	TRACE("MemoryManager<%s>: peak allocation is %u\n", typeid(T).name(), _allocatedPeak);
#endif
	}

	T* Alloc(void)
	{
#ifdef _DEBUG
		if( ++_allocatedCount > _allocatedPeak )
			_allocatedPeak = _allocatedCount;
#endif

		if( !_freeBlock ) Grow();
		BlankObject* tmp = _freeBlock;
		_freeBlock = _freeBlock->pNext;
		return (T*) tmp;
	}

	void Free(T* p)
	{
		_ASSERT(_allocatedCount--);
		((BlankObject*) p)->pNext = _freeBlock;
		_freeBlock = (BlankObject*) p;
	}
};

///////////////////////////////////////////////////////////////////////////////


template
<
	class T,
	size_t block_size = 128
>
class MemoryPool
{
	struct Block;

	struct BlankObject
	{
		union
		{
			BlankObject *next;
			char data[sizeof(T)];
		};
		Block *block;
	};

	struct Block
	{
		Block *_prev;
		Block *_next;
		Block *_prevFree;
		Block *_nextFree;
		BlankObject *_blanks;
		BlankObject *_free;
		size_t _used;

		Block()
		  : _prev(NULL)
		  , _next(NULL)
		  , _prevFree(NULL)
		  , _nextFree(NULL)
		  , _blanks((BlankObject *) malloc(sizeof(BlankObject) * block_size))
		  , _free(_blanks)
		  , _used(0)
		{
			// link together newly allocated blanks
			BlankObject *tmp(_blanks);
			BlankObject *end(_blanks + block_size - 1);
			while( tmp != end )
			{
				tmp->block = this;
				tmp->next = tmp + 1;
				++tmp;
			}
			end->block = this;
			end->next = NULL;
		}

		~Block()
		{
			free(_blanks);
		}

		BlankObject* Alloc()
		{
			_ASSERT(_free);
			BlankObject *tmp = _free;
			_free = _free->next;
			++_used;
			return tmp;
		}

		void Free(BlankObject *p)
		{
			_ASSERT(this == p->block);
			p->next = _free;
			_free = p;
			--_used;
		}
	};


	Block *_blocks;
	Block *_freeBlock;

#ifdef _DEBUG
	size_t _allocated_count;
	size_t _allocated_peak;
#endif

	void Grow()
	{
		if( _blocks )
		{
			_ASSERT(!_blocks->_prev);
			_blocks->_prev = new Block();
			_blocks->_prev->_next = _blocks;
			_blocks = _blocks->_prev;
		}
		else
		{
			_blocks = new Block();
		}

		if( _freeBlock )
		{
			_ASSERT(!_freeBlock->_prevFree);
			_blocks->_nextFree = _freeBlock;
			_freeBlock->_prevFree = _blocks;
		}
		_freeBlock = _blocks;
	}

public:
	MemoryPool()
	  : _blocks(NULL)
	{
#ifdef _DEBUG
		_allocated_count = 0;
		_allocated_peak  = 0;
#endif
	}

	~MemoryPool()
	{
		while( _blocks )
		{
			Block *tmp = _blocks;
			_blocks = _blocks->_next;
			delete tmp;
		}

#ifdef _DEBUG
		_ASSERT(0 == _allocated_count);
		printf("MemoryPool<%s>: peak allocation is %u\n", typeid(T).name(), _allocated_peak);
#endif
	}

	T* Alloc()
	{
#ifdef _DEBUG
		if( ++_allocated_count > _allocated_peak )
			_allocated_peak = _allocated_count;
#endif

		if( !_freeBlock )
		{
			Grow();
		}

		BlankObject *result = _freeBlock->Alloc();
		_ASSERT(_freeBlock == result->block);

		if( !_freeBlock->_free )
		{
			// no more free blanks in this block
			Block *tmp = _freeBlock;
			_freeBlock = _freeBlock->_nextFree;
			tmp->_nextFree = NULL;
			tmp->_prevFree = NULL;
			_ASSERT(!tmp->_prevFree);
		}

		return (T *) result->data;
	}

	void Free(void* p)
	{
		_ASSERT(_allocated_count--);

		Block *block = ((BlankObject*) p)->block;
		if( !block->_free )
		{
			// block just became free
			_ASSERT(!block->_prevFree);
			_ASSERT(!block->_nextFree);

			if( _freeBlock )
			{
				_ASSERT(!_freeBlock->_prevFree);
				_freeBlock->_prevFree = block;
				block->_nextFree = _freeBlock;
			}
			_freeBlock = block;
		}

		block->Free((BlankObject *) p);

		if( 0 == block->_used )
		{
			if( block == _blocks )
				_blocks = _blocks->_next;
			if( block->_prev )
				block->_prev->_next = block->_next;
			if( block->_next )
				block->_next->_prev = block->_prev;

			if( block == _freeBlock )
				_freeBlock = _freeBlock->_nextFree;
			if( block->_prevFree )
				block->_prevFree->_nextFree = block->_nextFree;
			if( block->_nextFree )
				block->_nextFree->_prevFree = block->_prevFree;

			delete block;
		}
	}
};


#define MemoryManager	MemoryPool

///////////////////////////////////////////////////////////////////////////////
// end of file
