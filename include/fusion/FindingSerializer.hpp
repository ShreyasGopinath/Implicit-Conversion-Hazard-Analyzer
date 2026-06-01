#pragma once

#include <string>
#include "fusion/Finding.hpp"

namespace fusion {

class FindingSerializer {
   public:
    // Serialize Finding to JSON
    static std::string toJSON(const Finding& finding, bool pretty = true);

    // Serialize Finding to SARIF-compatible structure
    static std::string toSARIF(const Finding& finding);

   private:
    static std::string escapeJSON(const std::string& str);
    static std::string indent(int level);
};

}  // namespace fusion
