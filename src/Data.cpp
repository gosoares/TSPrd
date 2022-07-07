#include "Data.h"

Data::Data(const Instance& instance, const AlgParams& params)
    : V(instance.V), N(V - 1), timesMatrix(instance.timesMatrix), releaseDates(instance.releaseDates),
      biggerReleaseDate(*max_element(releaseDates.begin(), releaseDates.end())), symmetric(instance.symmetric),
      params(params), startTime(std::chrono::steady_clock::now()), generator(params.seed) {}

std::chrono::milliseconds Data::elapsedTime() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime);
}
