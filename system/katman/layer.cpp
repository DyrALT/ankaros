#include "layer.h"

#include <algorithm>

namespace ankaros {

bool validLayerName(std::string_view name)
{
    if (name.empty() || name.size() > 32 || name.front() == '-')
        return false;
    return std::all_of(name.begin(), name.end(), [](char c) {
        return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-';
    });
}

std::string layerRoot(std::string_view name)
{
    return std::string(layersDirectory) + "/" + std::string(name) + "/kok";
}

std::vector<SharedPath> sharedPaths(const std::string &home, const std::string &runtimeDir)
{
    std::vector<SharedPath> paths = {
        {"/dev", "/dev"},
        {"/proc", "/proc"},
        {"/sys", "/sys"},
        {"/tmp", "/tmp"},
        {"/etc/resolv.conf", "/etc/resolv.conf", true},
    };
    if (!home.empty() && home.starts_with("/home/"))
        paths.push_back({home, home});
    if (!runtimeDir.empty() && runtimeDir.starts_with("/run/user/"))
        paths.push_back({runtimeDir, runtimeDir});
    return paths;
}

} // namespace ankaros
