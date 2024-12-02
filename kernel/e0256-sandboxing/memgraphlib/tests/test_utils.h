#ifndef __TEST_UTILS_H_INCLUDED__
#define __TEST_UTILS_H_INCLUDED__

static unsigned long tc_data_size = 0;

static std::vector<std::vector<unsigned long>> test_cases;
static std::vector<unsigned long> test_cases_results;

static std::vector<std::vector<unsigned long>> tc_edge_listings;
static std::vector<std::vector<unsigned long>> tc_libcall_listings;

enum tc_data_type {
    TC_DATA_NODE_COUNT          = 0,
    TC_DATA_EDGE_LISTING        = 1,
    TC_DATA_TEST_CASES_LISTING  = 2,
};

void cleanup_test_data() {
    tc_data_size = 0;
    test_cases.clear();
    tc_edge_listings.clear();
    tc_libcall_listings.clear();
}

bool read_test_data(const char *filename) {
    cleanup_test_data();
    std::ifstream infile(filename);
    enum tc_data_type type = TC_DATA_NODE_COUNT;
    if (!infile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(infile, line)) {
        // std::cout << line << std::endl;
        if (line == "##") {
            type = TC_DATA_EDGE_LISTING;
            continue;
        } else if (line == "--") {
            type = TC_DATA_TEST_CASES_LISTING;
            continue;
        }

        if (type == TC_DATA_NODE_COUNT) {
            tc_data_size = std::stoul(line);
            type = TC_DATA_EDGE_LISTING;
        } else if (type == TC_DATA_EDGE_LISTING) {
            std::istringstream iss(line);
            unsigned long src_node_id, successor_count, libcall_id, next_node_id;
            if (!(iss >> src_node_id >> successor_count)) {
                std::cerr << "Invalid edge listing: " << line << std::endl;
                continue;
            }
            std::vector <unsigned long> edge_listing , libcalls_listing;

            // std::cerr << "Edge listing: " << src_node_id << " -> " << successor_count << std::endl;
            for (int i = 0 ; i < successor_count ; i++) {
                if (!(iss >> next_node_id >> libcall_id)) {
                    std::cerr << "Invalid edge listing: " << line << std::endl;
                    continue;
                }
                edge_listing.push_back(next_node_id);
                libcalls_listing.push_back(libcall_id);
            }
            tc_edge_listings.push_back(edge_listing);
            tc_libcall_listings.push_back(libcalls_listing);

            // tc_data_size++;
        } else if (type == TC_DATA_TEST_CASES_LISTING) {
            std::istringstream iss(line);
            std::vector<unsigned long> numbers;
            unsigned long number, result;
            iss >> result;
            while (iss >> number) {
                numbers.push_back(number);
            }
            test_cases_results.push_back(result);
            test_cases.push_back(numbers);
        }
    }

    infile.close();
    return true;
}


#endif // __TEST_UTILS_H_INCLUDED__
