#include <algorithm>
#include <chrono>
#include <iostream>
#include <mutex>
#include <sched.h>
#include <thread>
#include <vector>

#include <cstdlib>

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    //thread associada a um CPU

    constexpr unsigned num_threads = 4;
    //acesso exclusivo ao cout
    std::mutex iomutex;
    std::vector<std::thread> threads(num_threads);
    for(unsigned i=0; i<num_threads; ++i)
    {
        threads[i] = std::thread([&iomutex, i]
        {
            while(1)
            {
                {
                    //protegendo a área do cout
                    std::lock_guard<std::mutex> iolock(iomutex);
                    std::cout << "Thread #" << i << ": na CPU " << sched_getcpu() << "\n";
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(900));
            }
        });

    }

    for(auto& t: threads)
    {
        t.join();
    }

    return EXIT_SUCCESS;
}
