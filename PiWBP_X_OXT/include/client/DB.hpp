#ifndef DBHPP
# define DBHPP

#include <assert.h>

#include "rocksdb/db.h"

#include "exception.hpp"

class DB {
    public:
        DB(std::string dbpath);
        ~DB();
        std::string get(std::string key);
        bool put(std::string key, std::string value);
        bool del(std::string key);
    private:
        rocksdb::DB* db;
};

#endif /* DBHPP */