#pragma once
#include <string>

#include "types.h"

std::string pr_str(const std::shared_ptr<MalType>& mal_type,
                   bool readably = true);
