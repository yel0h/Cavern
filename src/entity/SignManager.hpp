#ifndef CAVERN_SIGNMANAGER_HPP
#define CAVERN_SIGNMANAGER_HPP
#include "Sign.hpp"
#include <vector>

class SignManager
{
public:
    static constexpr const char *line1 = "MIND THE DROP";
    static constexpr const char *line2 = "WATCH YOUR STEP";
    static constexpr const char *line3 = "-- yeloh";
    static constexpr int maxSigns = 512;
    std::vector<Sign> signs;

    void place(glm::vec3 pos, float yaw);

    void reset() { signs.clear(); }

    bool save(const char *path) const;

    bool load(const char *path);
};
#endif//CAVERN_SIGNMANAGER_HPP