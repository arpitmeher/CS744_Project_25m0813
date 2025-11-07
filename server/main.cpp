#include "httplib.h"
#include "kv_cache.h"
#include "db_connector.h"
#include "stats.h"
#include <regex>

using namespace std;

KVCache cache(1000);
DBConnector db;
Stats stats;

bool is_valid_key(const std::string& key) {
    return !key.empty() && key.size() <= 64;
}

bool is_valid_value(const std::string& value) {
    return !value.empty() && value.size() <= 4096;
}

int main() {
    httplib::Server svr;

    svr.new_task_queue = []() {
        unsigned int num_threads = std::thread::hardware_concurrency();
        return new httplib::ThreadPool(num_threads > 0 ? num_threads : 4);
    };

    svr.Post("/create", [](const httplib::Request& req, httplib::Response& res) {
        stats.total_puts++;
        std::string key = req.get_param_value("key");
        std::string value = req.get_param_value("value");
     
        if (!is_valid_key(key) || !is_valid_value(value)) {
            stats.invalid_puts++;
            res.status = 400;
            res.set_content("Invalid key or value", "text/plain");
            return;
        }

        stats.valid_puts++;
        cache.put(key, value);
        db.put(key, value);
        res.set_content("Created", "text/plain");
    });

    svr.Get("/read", [](const httplib::Request& req, httplib::Response& res) {
        std::string key = req.get_param_value("key");
        std::string value;

        if (cache.get(key, value)) {
            res.set_content("Cache Hit: " + value, "text/plain");
        } else if (db.get(key, value)) {
            cache.put(key, value);
            res.set_content("Cache Miss: " + value, "text/plain");
        } else {
            res.status = 404;
            res.set_content("Not Found", "text/plain");
        }
    });

   svr.Delete("/delete", [](const httplib::Request& req, httplib::Response& res) {
    std::string key = req.get_param_value("key");
    cache.remove(key);
    if (db.remove(key)) {
        res.set_content("Deleted", "text/plain");
    } else {
        res.status = 404;
        res.set_content("Key not found", "text/plain");
    }
});


    svr.Get("/stats", [](const httplib::Request&, httplib::Response& res) {
        std::string report = "Cache Hits: " + std::to_string(cache.hits) +
                             "\nCache Misses: " + std::to_string(cache.misses) +
                             "\nTotal PUTs: " + std::to_string(stats.total_puts.load()) +
                             "\nValid PUTs: " + std::to_string(stats.valid_puts.load()) +
                             "\nInvalid PUTs: " + std::to_string(stats.invalid_puts.load());
        res.set_content(report, "text/plain");
    });

    if (!svr.listen("0.0.0.0", 8080)) {
        std::cerr << "Failed to start server on port 8080\n";
        return 1;
    }

    return 0;
}
