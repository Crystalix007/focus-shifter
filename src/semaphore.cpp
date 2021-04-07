#include "semaphore.hpp"

POSIXSemaphore::POSIXSemaphore(unsigned int initialValue) {
	sem_init(&semaphore, 0, initialValue);
}

POSIXSemaphore::~POSIXSemaphore() {
	sem_destroy(&semaphore);
}

void POSIXSemaphore::acquire() {
	sem_wait(&semaphore);
}

void POSIXSemaphore::release(unsigned int diff) {
	for (unsigned int i = 0; i < diff; i++) {
		sem_post(&semaphore);
	}
}
