#pragma once

#include <iostream>
#include <stdexcept>
#include <cstdint>

namespace demo {
    using Cents = std::int64_t;
    using Grams = std::int64_t;

    enum class Zone {Local, Remote};

    struct Shipment {
        Cents subtotal; 
        Grams grams = 1000;
        Zone zone = Zone::Local;
        bool loyal = false;
    };
    
    class DeliveryPolicy {
    public:
        Cents quote(const Shipment& s) const {
            if (s.subtotal < 0)
                throw std::invalid_argument("negative order subtotal");
            if (s.grams <= 0 || s.grams > 50000)
                throw std::invalid_argument("invalid shipment weight");
            return base_fee(s) + overweight_fee(s);
        }

    private:
        static Cents base_fee(const Shipment& s) {
            if (s.zone == Zone::Remote)
                return s.loyal ? 50000 : 70000;
            if (s.subtotal >= (s.loyal ? 300000 : 500000))
                return 0;
            return 30000;
        }

        static Cents overweight_fee(const Shipment& s) {
            const Grams over = s.grams - 20000;
            if (over <= 0) return 0;
            if (s.zone == Zone::Remote)
                return ((over + 999) / 1000) * 30000;
            return (over / 5000 + 1) * 100000;
        }
    };

}
