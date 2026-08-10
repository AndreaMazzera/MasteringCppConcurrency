// src/common/logger.h
#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <syncstream>
#include <thread>

#define LOG(msg) \
    std::osyncstream(std::cout) << "[Thread: " << std::this_thread::get_id() << "] " << msg << "\n"

#endif // LOGGER_H