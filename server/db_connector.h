#pragma once
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <mutex>
#include <queue>
#include <memory>
#include <condition_variable>

class DBConnector {
public:
    DBConnector(size_t pool_size = 8);
    void put(const std::string& key, const std::string& value);
    bool get(const std::string& key, std::string& value);
    bool remove(const std::string& key);

private:
    sql::Driver* driver;
    std::string db_url, user, password, schema;

    std::mutex pool_mutex;
    std::condition_variable pool_cv;
    std::queue<std::unique_ptr<sql::Connection>> connection_pool;

    std::unique_ptr<sql::Connection> create_connection();
    std::unique_ptr<sql::Connection> acquire_connection();
    void release_connection(std::unique_ptr<sql::Connection>&& conn);
};
