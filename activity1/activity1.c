#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <time.h>

#define SHM_NAME "/activity1_shm"
#define SEM1_NAME "/activity1_sem1"
#define SEM2_NAME "/activity1_sem2"

int main(int argc, char** argv) {
    srand(time(NULL));
    // Create and initialize semaphores
    sem_t* sem1 = sem_open(SEM1_NAME, O_CREAT, 0644, 1);
    if (sem1 == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    sem_t* sem2 = sem_open(SEM2_NAME, O_CREAT, 0644, 0);
    if (sem2 == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    // Create and initialize shared memory
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0644);
    if (fd == -1) {
        perror("shm_open");
        exit(1);
    }
    if (ftruncate(fd, sizeof(int)) == -1) {
        perror("ftruncate");
        exit(1);
    }
    int* shared_num = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_num == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    *shared_num = rand() % 20 + 11; // Generate a random number between 11 and 20

    // Create child process
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) { // Child process
        for (;;) {
            sem_wait(sem2); // Wait for parent's turn
            if (*shared_num == 0) {
                break;
            }
            (*shared_num)--;
            printf("child (pid = %d) bounce %d.\n", getpid(), *shared_num);
            sem_post(sem1); // Notify parent's turn
        }
        printf("child (pid = %d) ends.\n", getpid());
        sem_close(sem1);
        sem_close(sem2);
        exit(0);
    } else { // Parent process
        printf("parent (pid = %d) begins.\n", getpid());
        for (;;) {
            if (*shared_num == 0) {
                break;
            }
            (*shared_num)--;
            printf("parent (pid = %d) bounce %d.\n", getpid(), *shared_num);
            sem_post(sem2); // Notify child's turn
            sem_wait(sem1); // Wait for child's turn
        }

        printf("parent (pid = %d) ends.\n", getpid());
        sem_post(sem2);
        sem_close(sem1);
        sem_close(sem2);
        munmap(shared_num, sizeof(int));
        close(fd);
        shm_unlink(SHM_NAME);
    }

    exit(0);
}