/*
 * mt19937_rng.h
 *
 *  Created on: Jan 9, 2020
 *      Author: eyck
 */

#ifndef _UTIL_MT19937_RNG_H_
#define _UTIL_MT19937_RNG_H_

#include <iostream>
#include <random>

namespace util {
class MT19937 {
public:
    /**
     * Seeds the mersenne twister PRNG with the given value
     * @param new_seed
     */
    static void seed(uint64_t new_seed = std::mt19937_64::default_seed) { inst().seed(new_seed); }
    /**
     * generates the next random integer number with uniform distribution (similar to rand() )
     * @return
     */
    static uint64_t uniform() {
        std::uniform_int_distribution<uint64_t> u;
        return u(inst());
    }
    /**
     * generates the next random integer number with normal distribution (similar to rand() )
     * @return
     */
    static double normal() {
        std::normal_distribution<> u;
        return u(inst());
    }
    /**
     * generates the next random integer number with log normal distribution (similar to rand() )
     * @return
     */
    static double lognormal() {
        std::lognormal_distribution<> u;
        return u(inst());
    }

private:
    static std::mt19937_64& inst() {
        static std::mt19937_64 rng;
        return rng;
    }
};
} // namespace util
#endif /* _UTIL_MT19937_RNG_H_ */
