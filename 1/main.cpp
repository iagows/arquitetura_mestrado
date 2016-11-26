#include <algorithm>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include <cstdlib>

int main(int argc, char** argv)
{
    //thread por CPU
    (void)argc;
    (void)argv;
    unsigned num_cpus = std::thread::hardware_concurrency();
    std::cout << "Lançando " << num_cpus << " threads\n";

    //Um mutex para acessar o std::cout de muitas threads
    std::mutex iomutex;
    std::vector<std::thread> threads(num_cpus);
    for(unsigned i=0; i<num_cpus; ++i)
    {
        threads[i] = std::thread([&iomutex, i]
        {
            {
                //travando só para o momento do uso do cout
                std::lock_guard<std::mutex> iolock(iomutex);
                std::cout << "Thread #" << i << " está rodando\n";
            }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        });
    }

    for(auto& t:threads)
    {
        t.join();
    }

    return EXIT_SUCCESS;
}
