#include "db_connector.h"

DBConnector::DBConnector(size_t pool_size) {
    driver = get_driver_instance();
    db_url = "tcp://127.0.0.1:3306";
    user = "root";
    password = "pass";
    schema = "kvstore";

    for (size_t i = 0; i < pool_size; ++i) {
        auto conn = create_connection();
        connection_pool.push(std::move(conn));
    }
}

std::unique_ptr<sql::Connection> DBConnector::create_connection() {
    auto conn = std::unique_ptr<sql::Connection>(
        driver->connect(db_url, user, password));
    conn->setSchema(schema);
    return conn;
}

std::unique_ptr<sql::Connection> DBConnector::acquire_connection() {
    std::unique_lock<std::mutex> lock(pool_mutex);
    pool_cv.wait(lock, [this]() { return !connection_pool.empty(); });

    auto conn = std::move(connection_pool.front());
    connection_pool.pop();
    return conn;
}

void DBConnector::release_connection(std::unique_ptr<sql::Connection>&& conn) {
    std::lock_guard<std::mutex> lock(pool_mutex);
    connection_pool.push(std::move(conn));
    pool_cv.notify_one();
}

void DBConnector::put(const std::string& key, const std::string& value) {
    auto conn = acquire_connection();

    {
    auto stmt = std::unique_ptr<sql::PreparedStatement>(
        conn->prepareStatement("INSERT INTO kv_table (k, v) VALUES (?, ?) ON DUPLICATE KEY UPDATE v = VALUES(v)")
    );


    stmt->setString(1, key);
    stmt->setString(2, value);
    stmt->execute();
    }
    release_connection(std::move(conn));
}

bool DBConnector::get(const std::string& key, std::string& value) {
    auto conn = acquire_connection();
    bool found = false;
    {
    auto stmt = std::unique_ptr<sql::PreparedStatement>(
        conn->prepareStatement("SELECT v FROM kv_table WHERE k = ?")
    );

    stmt->setString(1, key);
    std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
    if (res->next()) {
        value = res->getString("v");
        found = true;
    }
}
    release_connection(std::move(conn));
    return found;
}

bool DBConnector::remove(const std::string& key) {
    auto conn = acquire_connection();
    int affected = 0;
    {
    std::unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement("DELETE FROM kv_table WHERE k = ?")
    );
    stmt->setString(1, key);
    int affected = stmt->executeUpdate();
    }
    release_connection(std::move(conn));
    return affected > 0;
}

