#ifndef __UNITTEST_DUMMY_NAMESPACE_PROTOCOL_HPP__
#define __UNITTEST_DUMMY_NAMESPACE_PROTOCOL_HPP__

#include <map>
#include <set>
#include <string>
#include <vector>

#include "utils.hpp"
#include <boost/function.hpp>

#include "concurrency/fifo_checker.hpp"
#include "timestamps.hpp"

struct signal_t;

namespace unittest {

class dummy_protocol_t {

public:

    class region_t {

    public:
        bool contains(const region_t &r);
        bool overlaps(const region_t &r);
        region_t intersection(const region_t &r);

        std::set<std::string> keys;
    };

    class temporary_cache_t {
        /* Dummy protocol doesn't need to cache anything */
    };

    class read_response_t {

    public:
        std::map<std::string, std::string> values;
    };

    class read_t {

    public:
        region_t get_region();
        std::vector<read_t> shard(std::vector<region_t> regions);
        read_response_t unshard(std::vector<read_response_t> resps, temporary_cache_t *cache);

        region_t keys;
    };

    class write_response_t {

    public:
        std::map<std::string, std::string> old_values;
    };

    class write_t {

    public:
        region_t get_region();
        std::vector<write_t> shard(std::vector<region_t> regions);
        write_response_t unshard(std::vector<write_response_t> resps, temporary_cache_t *cache);

        std::map<std::string, std::string> values;
    };

    class store_t {

    public:
        region_t get_region();
        bool is_coherent();
        state_timestamp_t get_timestamp();

        read_response_t read(read_t read, order_token_t otok, signal_t *interruptor);
        write_response_t write(write_t write, transition_timestamp_t timestamp, order_token_t otok, signal_t *interruptor);

        bool is_backfilling();

        class backfill_request_t {

        public:
            region_t get_region();
            state_timestamp_t get_timestamp();

            region_t region;
            state_timestamp_t earliest_timestamp, latest_timestamp;
        };

        class backfill_chunk_t {

        public:
            std::string key, value;
            state_timestamp_t timestamp;
        };

        backfill_request_t backfillee_begin();
        void backfillee_chunk(backfill_chunk_t chunk);
        void backfillee_end(state_timestamp_t end);
        void backfillee_cancel();

        state_timestamp_t backfiller(backfill_request_t request,
            boost::function<void(backfill_chunk_t)> chunk_fun,
            signal_t *interruptor);

        /* This stuff isn't part of the protocol interface, but it's public for
        the convenience of the people using `dummy_protocol_t`. */

        store_t(region_t r);
        ~store_t();

        region_t region;

        bool coherent, backfilling;

        state_timestamp_t earliest_timestamp, latest_timestamp;

        order_sink_t order_sink;

        std::map<std::string, std::string> values;
        std::map<std::string, state_timestamp_t> timestamps;
    };
};

bool operator==(dummy_protocol_t::region_t a, dummy_protocol_t::region_t b);
bool operator!=(dummy_protocol_t::region_t a, dummy_protocol_t::region_t b);

}   /* namespace unittest */

#endif /* __UNITTEST_DUMMY_NAMESPACE_PROTOCOL_HPP__ */
