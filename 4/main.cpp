#include <algorithm>
#include <chrono>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <thread>
#include <vector>

#include <cstdlib>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    //Configurando a afinidade com a CPU por código
    constexpr unsigned num_threads = 4;

    //mutex para acessar o cout
    std::mutex iomutex;
    std::vector<std::thread> threads(num_threads);
    for(unsigned i =0; i<num_threads; ++i)
    {
        threads[i] = std::thread([&iomutex, i]
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            while(1)
            {
                {
                    //travando o mutex só enquanto usa o cout
                    std::lock_guard<std::mutex> iolock(iomutex);
                    std::cout <<"Thread #" << i << ": na CPU " << sched_getcpu() << "\n";
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(900));
            }
        });
        //criando um objeto cpu_set_t representando os CPUs. Limpando a lista
        //e marcando só CPU i como o 'set' (conjunto)
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(i, &cpuset);
        int rc = pthread_setaffinity_np(threads[i].native_handle(),
                                        sizeof(cpu_set_t), &cpuset);
        if(rc != 0)
        {
            std::cerr << "Erro ao criar a pthread_setaffinity_np: " << rc << "\n";
        }
    }

    for(auto& t: threads)
    {
        t.join();
    }
    std::cout << "\nA thread fica sempre ligada ao mesmo CPU especificado";
    return EXIT_SUCCESS;
}
