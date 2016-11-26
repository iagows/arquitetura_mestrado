#include <cstdlib>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <thread>
#include <vector>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    //vendo ID das threads e os controladores nativos (OS)

    std::mutex iomutex;
    std::thread t = std::thread([&iomutex]
    {
        {
            std::lock_guard<std::mutex> iolock(iomutex);
            std::cout << "Thread: minha ID = " << std:: this_thread::get_id() << "\n"
                      << "PThread: minha ID= " << pthread_self() << "\n";
        }
    });
    {
        std::lock_guard<std::mutex> iolock(iomutex);
        std::cout << "Lançou t: id = " << t.get_id() << "\n"
                  << "Controlador nativo = " << t.native_handle() << "\n";
    }

    t.join();

    std::cout << "\nO id é o mesmo. Então podemos localizar a thread"
              << "\ntanto pelo ID da thread (c++) como pelo ID da pthread (SO)." << std::endl;
    return EXIT_SUCCESS;
}
