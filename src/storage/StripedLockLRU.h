#ifndef AFINA_STORAGE_STRIPED_SIMPLE_LRU_H
#define AFINA_STORAGE_STRIPED_SIMPLE_LRU_H

#include <map>
#include <mutex>
#include <string>
#include <memory>
#include <vector>

#include "afina/Storage.h"
#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

/**
 * # StripedRU 
 *
 *
 */
class StripedLockLRU : public Afina::Storage{
public:
    StripedLockLRU(size_t max_size = 1024, std::size_t size = 4) : _size_stripes(size), _max_size(max_size)
    {
        shards = std::vector<std::unique_ptr<SimpleLRU>>();
        for(int i = 0; i < _size_stripes; i++)
        {
            shards.push_back(std::unique_ptr<Afina::Backend::SimpleLRU>(new SimpleLRU(_max_size)));
        }
    }
    ~StripedLockLRU() {}

    // see SimpleLRU.h
    bool Put(const std::string &key, const std::string &value) override {
        // TODO: sinchronization
        return shards[_hash_for_key(key) % _size_stripes]->Put(key, value);
    }

    // see SimpleLRU.h
    bool PutIfAbsent(const std::string &key, const std::string &value) override {
        // TODO: sinchronization
        return shards[_hash_for_key(key) % _size_stripes]->PutIfAbsent(key, value);
    }

    // see SimpleLRU.h
    bool Set(const std::string &key, const std::string &value) override {
        // TODO: sinchronization
        return shards[_hash_for_key(key) % _size_stripes]->Set(key, value);
    }

    // see SimpleLRU.h
    bool Delete(const std::string &key) override {
        // TODO: sinchronization
        return shards[_hash_for_key(key) % _size_stripes]->Delete(key);
    }

    // see SimpleLRU.h
    bool Get(const std::string &key, std::string &value) override {
        // TODO: sinchronization
        return shards[_hash_for_key(key) % _size_stripes]->Get(key, value);
    }

private:
    std::hash<std::string> _hash_for_key;
    std::size_t _size_stripes;
    std::size_t _max_size;
    std::vector<std::unique_ptr<SimpleLRU>> shards;
};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_THREAD_SAFE_SIMPLE_LRU_H