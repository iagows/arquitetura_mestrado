#include <cstdlib>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <random>
#include <thread>
#include <vector>

//tipo "função de carga de trabalho". Recebe um vetor e uma referência para um tipo FLOAT que será o resultado
using WorkLoadFunc = std::function<void(const std::vector<float>&, float&)>;

//reduzindo as chamadas do cronômetro
using hires_clock = std::chrono::high_resolution_clock;
using duration_ms = std::chrono::duration<double, std::milli>;

std::mutex iomutex;

void workload_fpchurn(const std::vector<float>& data, float& result)
{
    constexpr size_t NUM_ITERS = 10*1000*1000;
    auto t1 = hires_clock::now();
    float rt = 0;
    for(size_t i = 0; i < NUM_ITERS; ++i)
    {
        float item = data[i];
        float l = std::log(item);
        if(l>rt)
        {
            l = std::sin(l);
        }
        else
        {
            l = std::cos(l);
        }

        if(l>rt - 2.0)
        {
            l = std::exp(l);
        }
        else
        {
            l = std::exp(l+1.0);
        }

        rt++;
    }
    result = rt;
    {
        auto t2 = hires_clock::now();

        std::lock_guard<std::mutex> iolock(iomutex);
        std::cout << __func__ << " [CPU " << sched_getcpu() << "]:\n";
        std::cout << " elapsed: " << duration_ms(t2-t1).count() << " ms\n";
    }
}

void workload_sin(const std::vector<float>& data, float& result)
{
    auto t1 = hires_clock::now();
    float rt = 0;
    for(size_t i=0, total = data.size(); i<total; ++i)
    {
        rt+= std::sin(data[i]);
    }
    result = rt;

    {
        auto t2 = hires_clock::now();
        std::lock_guard<std::mutex> iolock(iomutex);
        std::cout << __func__ << " [cpu " << sched_getcpu() << "]:\n";
        std::cout << " itens processados: " << data.size() << "\n";
        std::cout << " tempo passado: " << duration_ms(t2-t1).count() << " ms\n";
        std::cout << " resultado: " << result << "\n";
    }
}

void workload_accum(const std::vector<float>& data, float& result)
{
    auto t1 = hires_clock::now();
    float rt = 0;
    for(size_t i=0, total = data.size(); i< total; ++i)
    {
        /*
         * Num x86-64 isso pode gerar um loop de ADDs de data.size comprimento,
         * tudo somando no mesmo registrador xmm. Se compilado com -0fast
         * (-ffast-math), o compilador vai tentar realizar otimizações FP inseguras e
         * vai vetorizar o loop em um de data.size/4 ADDs.
         * Isso pode mudar a ordem em que os floats são adicionados, o que é inseguro
         * visto que adição FP é não associativa
        */
        rt+= data[i];
    }
    result = rt;

    {
        auto t2 = hires_clock::now();
        std::lock_guard<std::mutex> iolock(iomutex);
        std::cout << __func__ << " [cpu " << sched_getcpu() << "]\n";
        std::cout << " itens processados: " << data.size() << "\n";
        std::cout << " tempo passado: " << duration_ms(t2-t1).count() << " ms\n";
        std::cout << " resultado: " << result << "\n";
    }
}

void workload_unrollaccum4(const std::vector<float>& data, float& result)
{
    auto t1 = hires_clock::now();
    if(data.size() % 4 != 0)
    {
        std::cerr << "ERRO em " << __func__ << ": data.size " << data.size() << "\n";
    }

    float rt0 = 0;
    float rt1 = 0;
    float rt2 = 0;
    float rt3 = 0;
    for(size_t i = 0; i<data.size(); i+=4)
    {
        /*
         * Esse UNROLL é faz uma quebra manual das dependências de um
         * único registrador xmm (o da função anterior). Deve ser mais rápido
         * porque ADDs distintos farão suas somas em registradores separados.
         * Mas também é inseguro pelo mesmo problema da função anterior
         */
        rt0 += data[i];
        rt1 += data[i+1];
        rt2 += data[i+2];
        rt3 += data[i+3];
    }
    result = rt0+rt1+rt2+rt3;

    {
        auto t2 = hires_clock::now();
        std::lock_guard<std::mutex> iolock(iomutex);
        std::cout << __func__ << " [cpu " << sched_getcpu() << "]:\n";
        std::cout << " itens processados: " << data.size() << "\n";
        std::cout << " tempo passado: " << duration_ms(t2-t1).count() << " ms\n";
        std::cout << " resultado: " << result << "\n";
    }
}

void workload_unrollaccum8(const std::vector<float>& data, float& result) {
    auto t1 = hires_clock::now();
    if (data.size() % 8 != 0) {
        std::cerr
                << "ERROR in " << __func__ << ": data.size " << data.size() << "\n";
    }
    float rt0 = 0;
    float rt1 = 0;
    float rt2 = 0;
    float rt3 = 0;
    float rt4 = 0;
    float rt5 = 0;
    float rt6 = 0;
    float rt7 = 0;
    for (size_t i = 0; i < data.size(); i += 8) {
        rt0 += data[i];
        rt1 += data[i + 1];
        rt2 += data[i + 2];
        rt3 += data[i + 3];
        rt4 += data[i + 4];
        rt5 += data[i + 5];
        rt6 += data[i + 6];
        rt7 += data[i + 7];
    }
    result = rt0 + rt1 + rt2 + rt3 + rt4 + rt5 + rt6 + rt7;

    {
        auto t2 = hires_clock::now();
        std::lock_guard<std::mutex> iolock(iomutex);
        std::cout << __func__ << " [cpu " << sched_getcpu() << "]:\n";
        std::cout << "  itens processados: " << data.size() << "\n";
        std::cout << "  tempo passado: " << duration_ms(t2 - t1).count() << " ms\n";
        std::cout << "  resultado: " << result << "\n";
    }
}

void workload_stdaccum(const std::vector<float>& data, float& result)
{
    auto t1 = hires_clock::now();
    result = std::accumulate(data.begin(), data.end(), 0.0f);

    {
        auto t2 = hires_clock::now();
        std::lock_guard<std::mutex> iolock(iomutex);
        std::cout << __func__ << " [cpu " << sched_getcpu() << "\n";
        std::cout << " itens processados: " << data.size() << "\n";
        std::cout << " tempo passado: " << duration_ms(t2-t1).count() << " ms\n";
        std::cout << "resultado: " << result << "\n";
    }
}

/**
 * @brief make_input_array: Cria um vetor preenchido com N floats
 * distribuídos uniformemente (-1.0, 1.0)
 * @param N
 * @return
 */
std::vector<float> make_input_array(int N)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-1.0f, 1.0f);

    std::vector<float> vf(N);
    for(size_t i=0, total = vf.size(); i < total; ++i)
    {
        vf[i] = dis(gen);
    }
    return vf;
}

/**
 * @brief do_not_optimize: Essa função pode ser usada para marcar memória
 * que não deve ser otimizada, ainda assim não vai gerar código
 * @param p
 */
void do_not_optimize(void *p)
{
    asm volatile("" : : "g"(p):"memory");
}

void pin_thread_to_cpu(std::thread& t, int cpu_num)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_num, &cpuset);

    int rc = pthread_setaffinity_np(t.native_handle(), sizeof(cpu_set_t), &cpuset);
    if(rc !=0)
    {
        std::cerr << "Erro ao chamar pthread_setaffinity_np: " << rc << "\n";
    }
}

int main(int argc, char *argv[])
{
    //separando números grandes com vírgula
    std::cout.imbue(std::locale(""));

    //chamando da linha de comando:
    //argv[0] nome do programa
    //argv[1] nome da workload e cpu no argv seguinte (argv[2])
    //argv[3] nome da workload e cpu no argv seguinte (argv[4])
    //etc

    int num_workloads = argc /2;

    std::vector<float> results(num_workloads);
    do_not_optimize(results.data());

    std::vector<std::thread> threads(num_workloads);

    constexpr size_t INPUT_SIZE = 100*1000*1000;
    auto t1 = hires_clock::now();

    /*
     * Alocando e inicializando um array de entrada extra - não usado pelos workloads.
     * Isso é para garantir que nenhum dos dados de trabalho (working) fique na cache L3
     * (que é bem grandinha), o que poderia dar uma vantagem injusta  para um dos workloads.
     * As camadas mais baixas de cache são muito pequenas e o tempo da vantagem de pré-carregamento
     * pode ser descartado
     */
    std::vector<std::vector<float>> inputs(num_workloads+1);
    for(int i=0; i<num_workloads; ++i)
    {
        inputs[i] = make_input_array(INPUT_SIZE);
    }

    std::cout << "Criados " << num_workloads +1 << " arrays de entrada"
              << "; em: " <<duration_ms(hires_clock::now()-t1).count() << " ms\n";
    for(int i=1; i<argc; i+=2)
    {
        WorkLoadFunc func;
        std::string workload_name = argv[i];
        if(workload_name == "fpchurn")
        {
            func = workload_fpchurn;
        }
        else if(workload_name == "sin")
        {
            func = workload_sin;
        }
        else if(workload_name == "accum")
        {
            func = workload_accum;
        }
        else if(workload_name == "unrollaccum4")
        {
            func = workload_unrollaccum4;
        }
        else if(workload_name == "unrollaccum8")
        {
            func = workload_unrollaccum8;
        }
        else if(workload_name == "stdaccum")
        {
            func = workload_stdaccum;
        }
        else
        {
            std::cerr << "Workload desconhecida: " << argv[i] << "\n";
            return EXIT_FAILURE;
        }

        int cpu_num;
        if(i+1 >= argc)
        {
            cpu_num = 0;
        }
        else
        {
            cpu_num = std::atoi(argv[i+1]);
        }

        {
            std::lock_guard<std::mutex> iolock(iomutex);
            std::cout << "Chamando workload '" << workload_name << "' na CPU "
                      << cpu_num << "\n";
        }

        int nworkload = i/2;
        threads[nworkload] = std::thread(func, std::cref(inputs[nworkload]),
                                         std::ref(results[nworkload]));
        pin_thread_to_cpu(threads[nworkload], cpu_num);
    }

    /*
     * Todas as threads foram lançadas ao mesmo tempo, agora é só esperar acabar.
     */
    for(auto& t:threads)
    {
        t.join();
    }

    return EXIT_SUCCESS;
}
