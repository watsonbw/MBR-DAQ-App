#pragma once

#include <fstream>

#include "helpers/tempfile.hpp"

namespace mbr::tests::helpers {

struct backend_json_fixture {
    backend_json_fixture() {
        std::ofstream f{temp.path};
        f << R"({
              "fields": [
                { "key": "T",  "name": "Timestamp",   "required": true,  "plot": false },
                { "key": "E",  "name": "Engine RPM",  "required": false, "plot": true, "unit": "RPM",  "group": "rpm" },
                { "key": "W",  "name": "Wheel RPM",   "required": false, "plot": true, "unit": "RPM",  "group": "rpm" }
              ]
            })";
    }

    tempfile temp{"MBR_data.json"};
};

} // namespace mbr::tests
