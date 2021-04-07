#pragma once

// This is a utility header file written because glibc semaphores were not
// cooperating.

#include <semaphore.h>

class POSIXSemaphore {
public:
	explicit POSIXSemaphore(unsigned int initialValue);
	virtual ~POSIXSemaphore();

	void acquire();
	void release(unsigned int diff = 1);

protected:
	sem_t semaphore;
};

namespace std {
	using binary_semaphore = POSIXSemaphore;
} // namespace std
