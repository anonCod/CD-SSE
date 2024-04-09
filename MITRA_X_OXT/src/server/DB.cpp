#include "../../include/server/DB.hpp"

DB::DB(std::string dbpath) {
    rocksdb::Options options;
    rocksdb::Status status;
    
    options.create_if_missing = true;
    status = rocksdb::DB::Open(options, dbpath, &this->db);
    assert(status.ok());
}

DB::~DB() {
    delete this->db;
}

std::string DB::get(std::string key) {
    std::string value;
    rocksdb::Status s;

    s = this->db->Get(rocksdb::ReadOptions(), key, &value);
    if (!s.ok())
        throw new MITRAXOXTException(s.ToString());
    return value;
}

bool DB::put(std::string key, std::string value) {
    rocksdb::Status s;
    
    s = this->db->Put(rocksdb::WriteOptions(), key, value);
    if (!s.ok())
        return false;
    return true;
}

bool DB::del(std::string key) {
    rocksdb::Status s;
    
    s = this->db->Delete(rocksdb::WriteOptions(), key);
    if (!s.ok())
        return false;
    return true;
}