#include "httplib.h"
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <random>
#include <string>
#include <mutex>

std::atomic<long long> total_requests{0};
std::atomic<long long> total_latency_us{0};


std::mutex cout_mutex;


std::string randomKey(int tid) {
    static thread_local long long counter = 0;
    std::string key = "key_" + std::to_string(tid) + "_" + std::to_string(counter++);

    return key;
}


void putAllWorkload(httplib::Client& cli, int tid) {
    std::string key = randomKey(tid);
    std::string value = "val_" + std::to_string(rand() % 1000);

    // if (rand() % 2 == 0) {
    //     cli.Post("/create?key=" + key + "&value=" + value, "", "text/plain");
    // } else {
    //     cli.Delete("/delete?key=" + key);
    // }
    cli.Post("/create?key=" + key + "&value=" + value, "", "text/plain");
}

void getAllWorkload(httplib::Client& cli, int tid) {
    std::string key = randomKey(tid); 
    cli.Get("/read?key=" + key);
}

void getPopularWorkload(httplib::Client& cli, int tid) {
    std::vector<std::string> hotKeys;
    hotKeys.reserve(100);

    for (int i = 1; i <= 100; ++i) {
        hotKeys.push_back("hot" + std::to_string(i));
    }
    std::string key = hotKeys[rand() % hotKeys.size()];
    cli.Get("/read?key=" + key);
}


void clientThread(int tid, int duration, const std::string& workload) {
    httplib::Client cli("127.0.0.1", 8080);
    cli.set_connection_timeout(5, 0);
    cli.set_read_timeout(5, 0);
    cli.set_write_timeout(5, 0);

    auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < std::chrono::seconds(duration)) {
        auto req_start = std::chrono::steady_clock::now();

        try {
            if (workload == "putall") {
                putAllWorkload(cli, tid);
            } else if (workload == "getall") {
                getAllWorkload(cli, tid);
            } else if (workload == "getpopular") {
                getPopularWorkload(cli, tid);
            } else {
                // default: mixed workload
                int r = rand() % 3;
                if (r == 0) putAllWorkload(cli, tid);
                else if (r == 1) getAllWorkload(cli, tid);
                else getPopularWorkload(cli, tid);
            }
        } catch (...) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cerr << "Thread " << tid << " request failed\n";
        }

        auto req_end = std::chrono::steady_clock::now();
        long long latency = std::chrono::duration_cast<std::chrono::microseconds>(req_end - req_start).count();

        total_requests++;
        total_latency_us += latency;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " --workload [putall|getall|getpopular|mixed] --threads N --duration S\n";
        return 1;
    }

    std::string workload = argv[2];
    int threads = std::stoi(argv[4]);
    int duration = std::stoi(argv[6]);

    std::vector<std::thread> pool;
    for (int i = 0; i < threads; i++) {
        pool.emplace_back(clientThread, i, duration, workload);
    }
    for (auto& t : pool) t.join();

    double throughput = total_requests.load() / (double)duration;
    double avg_latency_ms = (total_latency_us.load() / (double)total_requests.load()) / 1000.0;

    std::cout << "Workload: " << workload << "\n";
    std::cout << "Total Requests: " << total_requests.load() << "\n";
    std::cout << "Threads: " << threads << "\n";
    std::cout << "Duration: " << duration << "s\n";
    std::cout << "Throughput: " << throughput << " req/s\n";
    std::cout << "Average Response Time: " << avg_latency_ms << " ms\n";

    return 0;
}
