#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <string>
#include <fstream>
#include <sstream>

#include "print.hpp"

static void *get_module_base_address(std::string module_name) {
    std::ifstream file;
    file.open("/proc/self/maps");

    bool found = false;

    std::string line;
    std::string concat_line;
    while (std::getline(file, line)) {
        for (int i = 0; i < line.length(); i++) {
            concat_line = "";
            for (int h = 0; i + h < line.length() && h < module_name.length(); h++) {
                concat_line += line[i + h];
            }
            if (concat_line == module_name) {
                found = true;
                break;
            }
        }

        if (found) { break; }
    }

    concat_line = "";
    for (int i = 0; i <= line.length(); i++) {
        if (line[i] == '-') { break; }
        concat_line += line[i];
    }

    // Use stringstream for conversion
    std::stringstream ss;
    ss << std::hex << "0x" + concat_line;

    // Store the result
    unsigned long result;
    ss >> result;

    return reinterpret_cast<void *>(result);
}

#endif
