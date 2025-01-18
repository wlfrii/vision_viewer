#ifndef H_WLF_C413C79E_222E_4F4B_90FD_A18AE2448A3A
#define H_WLF_C413C79E_222E_4F4B_90FD_A18AE2448A3A
#include <semaphore.h>

/**
 * @brief A wrapper class of semaphore.
 * 
 */
class CSemaphore{
public:
    /**
     * @brief Construct a new CSemaphore object。
     */
    CSemaphore(){
         sem_init(&sem, 0, 0);
    }

    /**
     * @brief Destroy the CSemaphore object。
     */
    ~CSemaphore(){
        sem_destroy(&sem);
    }

    /**
     * @brief Take the thread until semaphore is released.
     */
    void take(){
        sem_wait(&sem);
    }

    /**
     * @brief Release a semaphore.
     */
    void release(){
        sem_post(&sem);
    }

private:
    sem_t sem;  ///< The semaphore.
};

#endif /* H_WLF_C413C79E_222E_4F4B_90FD_A18AE2448A3A */
