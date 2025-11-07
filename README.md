###  Installation

To install required dependencies
```bash
sudo apt update
sudo apt install g++ cmake libmysqlcppconn-dev libssl-dev mysql-server
```

### Database Setup

Start MySQL and create the required database and table:

```sql
CREATE DATABASE kvstore;
USE kvstore;
CREATE TABLE kv_table (k VARCHAR(64) PRIMARY KEY, v TEXT);
```

### Build Instructions

Use CMake to build the project:

```bash
mkdir build && cd build
cmake ..
make
```

### Running the Server

From the `build` directory run:

```bash
./kv_server
```

### Create a key-value pair

```bash
curl -X POST http://localhost:8080/create -d "key=key&value=val"
```

### Read a value by key

```bash
curl "http://localhost:8080/read?key=key"
```

### Delete a key-value pair

```bash
curl -X DELETE "http://localhost:8080/delete?key=key"
```

### MySQL Credentials

If your MySQL username or password differs, update them in `db_connector.cpp`:

```cpp
user = "your_user";
password = "your_password";
```
