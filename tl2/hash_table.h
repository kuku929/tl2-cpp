#pragma once
#include <unordered_map>
#include "version_lock.h"
#include "types.h"

static std::unordered_map<uint *, VersionLock> hashtbl;