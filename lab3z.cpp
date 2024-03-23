#include <iostream>
#include <queue>
#include <future>
#include <thread>
#include <chrono>
#include <cmath>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <random>
#include <fstream>

std::mutex mut;

std::condition_variable condition;

class Server
{

public:

    Server() : isRunning(true)
    {
        std::cout << "Server start" << std::endl;
        worker = std::jthread(&Server::start, this);

    }
    template <typename T>
    size_t add_task(std::future<T>&& task)
    {
        std::unique_lock lock_res(mut, std::defer_lock);

        lock_res.lock();

        tasks.push({ id_task, std::move(task) });
        condition.notify_one();

        lock_res.unlock();

        return id_task++;
    }

    void stop()
    {
        std::cout << "Server ends work\n";
        condition.notify_one();
        isRunning = false;
    }

    double request_result(size_t id_res)
    {
        //std::unique_lock lock_res(mut, std::defer_lock);
        //double r;
        /*if (results.find(id_res) != results.end())
            r = results[id_res];
        else
            r = -1;*/
        return (results.find(id_res) != results.end()) ? results[id_res] : -1;
    }

private:

    std::unordered_map<size_t, double> results;
    size_t id_task = 1;
    std::jthread worker;
    bool isRunning;
    std::queue<std::pair<size_t, std::future<double>>> tasks;

    void start() {
        std::unique_lock lock_res(mut, std::defer_lock);
        while (true)
        {
            lock_res.lock();

            condition.wait(lock_res, [this] { return !tasks.empty() || !isRunning; });
            if (!tasks.empty())
            {
                results.insert({ tasks.front().first, tasks.front().second.get() });
                tasks.pop();
            }
            else
            {
                break;
            }

            lock_res.unlock();

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
};

template <typename T>
T fun_sin(T arg)
{
    return std::sin(arg);
}

template <typename T>
T fun_sqrt(T arg)
{
    return std::sqrt(arg);
}

template <typename T>
T fun_pow(T x, T y)
{
    return std::pow(x, y);
}


void add_task_threads_pow(Server& server)
{
    std::default_random_engine re(std::random_device{}());
    std::uniform_real_distribution<double> unif(1, 10);

    std::unique_lock lock_res{ mut, std::defer_lock };

    std::uniform_real_distribution<double> unify(5, 1000);

    int N = unify(re);

    std::ofstream out_pow;
    out_pow.open("out_pow.txt");

    for (int i = 0; i < N; i++)
    {
        std::future<double> result = std::async(std::launch::deferred,
            [&unif, &re]() -> double
            {
                int rand1 = unif(re);
                int rand2 = unif(re);
                return fun_pow(rand1, rand2);
            });

        size_t id_task = server.add_task(std::move(result));

        bool ready_result = false;

        while (!ready_result)
        {

            lock_res.lock();
            auto res = server.request_result(id_task);
            if (res != -1)
            {
                std::cout << "POWER task_thread " << id_task << " result:\t" << res << '\n';
                out_pow << res << " " << id_task << std::endl;
                ready_result = true;
            }
            lock_res.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    out_pow.close();
}

void add_task_threads_sinus(Server& server)
{

    std::default_random_engine re(std::random_device{}());
    std::uniform_real_distribution<double> unif(0, 1000);

    std::unique_lock lock_res{ mut, std::defer_lock };

    std::uniform_real_distribution<double> unify(5, 1000);

    int N = unify(re);

    std::ofstream out_sinus;
    out_sinus.open("out_sinus.txt");

    for (int i = 0; i < N; i++)
    {
        std::future<double> result = std::async(std::launch::deferred,
            [&unif, &re]() -> double
            {
                double rand1 = unif(re);
                return fun_sin(rand1);
            });

        size_t id_task = server.add_task(std::move(result));

        bool ready_result = false;

        while (!ready_result)
        {

            lock_res.lock();
            auto res = server.request_result(id_task);
            if (res != -1)
            {
                std::cout << "SIN task_thread " << id_task << " result:\t" << res << '\n';
                out_sinus << res << " " << id_task << std::endl;
                ready_result = true;
            }
            lock_res.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    out_sinus.close();
}


void add_task_threads_sqrt(Server& server)
{
    std::default_random_engine re(std::random_device{}());
    std::uniform_real_distribution<double> unif(0, 1000);

    std::unique_lock lock_res{ mut, std::defer_lock };

    std::uniform_real_distribution<double> unify(5, 1000);

    int N = unify(re);

    std::ofstream out_sqrt;
    out_sqrt.open("out_sqrt.txt");

    for (int i = 0; i < N; i++)
    {


        std::future<double> result = std::async(std::launch::deferred,
            [&unif, &re]() -> double
            {
                int rand1 = unif(re);
                return fun_sqrt(rand1);
            });

        result.get();

        size_t id_task = server.add_task(std::move(result));

        bool ready_result = false;

        while (!ready_result)
        {

            lock_res.lock();
            auto res = server.request_result(id_task);
            if (res != -1)
            {
                std::cout << "SQRT task_thread " << id_task << " result:\t" << res << '\n';
                out_sqrt << res << " " << id_task << std::endl;
                ready_result = true;
            }
            lock_res.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    out_sqrt.close();
}



int main()
{

    Server server;

    std::thread power(add_task_threads_pow, std::ref(server));
    std::thread sinux(add_task_threads_sinus, std::ref(server));
    std::thread sqrt(add_task_threads_sqrt, std::ref(server));

    sinux.join();
    sqrt.join();
    power.join();

    server.stop();

    return 0;
}