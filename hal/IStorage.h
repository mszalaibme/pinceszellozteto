#pragma once
#include "core/Configuration.h"
#include <optional>
namespace ventilation {
struct IStorage {
    virtual ~IStorage() = default;
    virtual std::optional<Configuration> load() = 0;
    virtual bool save(const Configuration& config) = 0;
};
}
