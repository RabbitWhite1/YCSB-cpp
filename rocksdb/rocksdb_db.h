//
//  rocksdb_db.h
//  YCSB-cpp
//
//  Copyright (c) 2020 Youngjae Lee <ls4154.lee@gmail.com>.
//

#ifndef YCSB_C_ROCKSDB_DB_H_
#define YCSB_C_ROCKSDB_DB_H_

#include <string>
#include <mutex>

#include "core/db.h"
#include "core/properties.h"

#include <rocksdb/db.h>
#include <rocksdb/options.h>

namespace ycsbc {

class RocksdbDB : public DB {
 public:
  RocksdbDB() {}
  ~RocksdbDB() {
    if (db_) {
      const std::lock_guard<std::mutex> lock(mu_);
      delete db_;
      db_ = nullptr;
    }
  }

  void Init();

  void Cleanup();

  Status Read(const std::string &table, const std::string &key,
              const std::vector<std::string> *fields, std::vector<Field> &result) {
    return (this->*(method_read_))(table, key, fields, result);
  }

  Status Scan(const std::string &table, const std::string &key, int len,
              const std::vector<std::string> *fields, std::vector<std::vector<Field>> &result) {
    return (this->*(method_scan_))(table, key, len, fields, result);
  }

  Status Update(const std::string &table, const std::string &key, std::vector<Field> &values) {
    return (this->*(method_update_))(table, key, values);
  }

  Status Insert(const std::string &table, const std::string &key, std::vector<Field> &values) {
    return (this->*(method_insert_))(table, key, values);
  }

  Status Delete(const std::string &table, const std::string &key) {
    return (this->*(method_delete_))(table, key);
  }

  void GetOrPrintDBStatus(std::map<std::string, std::string>* status_map) {
    std::map<std::string, std::string> cache_status;
    db_->GetMapProperty(rocksdb::DB::Properties::kBlockCacheEntryStats, &cache_status);
    std::shared_ptr<rocksdb::Statistics> statistics = db_->GetOptions().statistics;

    uint64_t hit_count = statistics->getTickerCount(rocksdb::BLOCK_CACHE_DATA_HIT);
    uint64_t miss_count = statistics->getTickerCount(rocksdb::BLOCK_CACHE_DATA_MISS);
    double hit_ratio = (double)hit_count / (hit_count + miss_count) * 100;

    if (status_map == nullptr) {
      printf("Block cache entry stats(count,bytes,percent): "
             "DataBlock(%s, %s, %s), IndexBlock(%s, %s, %s), FilterBlock(%s, %s, %s), Misc(%s, %s, %s)\n",
             cache_status["count.data-block"].c_str(), cache_status["bytes.data-block"].c_str(), cache_status["percent.data-block"].c_str(),
             cache_status["count.index-block"].c_str(), cache_status["bytes.index-block"].c_str(), cache_status["percent.index-block"].c_str(),
             cache_status["count.filter-block"].c_str(), cache_status["bytes.filter-block"].c_str(), cache_status["percent.filter-block"].c_str(),
             cache_status["count.other"].c_str(), cache_status["bytes.other"].c_str(), cache_status["percent.other"].c_str());
      printf("Block cache %s capacity: %s\n", cache_status["id"].c_str(), cache_status["capacity"].c_str());
      printf("block cache hit ratio: %lu/%lu = %.2lf%%\n", hit_count, hit_count+miss_count, hit_ratio);
    } else {
      (*status_map)["data_block_percent"] = cache_status["percent.data-block"];
      (*status_map)["block_cache.hit_ratio"] = std::to_string(hit_ratio);
      (*status_map)["block_cache.hit_count"] = std::to_string(hit_count);
      (*status_map)["block_cache.miss_count"] = std::to_string(miss_count);
    }
  }

 private:
  enum RocksFormat {
    kSingleRow,
  };
  RocksFormat format_;

  void GetOptions(const utils::Properties &props, rocksdb::Options *opt,
                  std::vector<rocksdb::ColumnFamilyDescriptor> *cf_descs);
  static void SerializeRow(const std::vector<Field> &values, std::string &data);
  static void DeserializeRowFilter(std::vector<Field> &values, const char *p, const char *lim,
                                   const std::vector<std::string> &fields);
  static void DeserializeRowFilter(std::vector<Field> &values, const std::string &data,
                                   const std::vector<std::string> &fields);
  static void DeserializeRow(std::vector<Field> &values, const char *p, const char *lim);
  static void DeserializeRow(std::vector<Field> &values, const std::string &data);

  Status ReadSingle(const std::string &table, const std::string &key,
                    const std::vector<std::string> *fields, std::vector<Field> &result);
  Status ScanSingle(const std::string &table, const std::string &key, int len,
                    const std::vector<std::string> *fields,
                    std::vector<std::vector<Field>> &result);
  Status UpdateSingle(const std::string &table, const std::string &key,
                      std::vector<Field> &values);
  Status MergeSingle(const std::string &table, const std::string &key,
                     std::vector<Field> &values);
  Status InsertSingle(const std::string &table, const std::string &key,
                      std::vector<Field> &values);
  Status DeleteSingle(const std::string &table, const std::string &key);

  Status (RocksdbDB::*method_read_)(const std::string &, const std:: string &,
                                    const std::vector<std::string> *, std::vector<Field> &);
  Status (RocksdbDB::*method_scan_)(const std::string &, const std::string &,
                                    int, const std::vector<std::string> *,
                                    std::vector<std::vector<Field>> &);
  Status (RocksdbDB::*method_update_)(const std::string &, const std::string &,
                                      std::vector<Field> &);
  Status (RocksdbDB::*method_insert_)(const std::string &, const std::string &,
                                      std::vector<Field> &);
  Status (RocksdbDB::*method_delete_)(const std::string &, const std::string &);

  int fieldcount_;

  static rocksdb::DB *db_;
  static int ref_cnt_;
  static std::mutex mu_;
};

DB *NewRocksdbDB();

} // ycsbc

#endif // YCSB_C_ROCKSDB_DB_H_

