#pragma once

#include <cstdint>

#include "pico/stdlib.h"

static constexpr uint32_t A4988_STEP_HIGH_US = 3;
static constexpr uint32_t A4988_STEP_PERIOD_US = 700;
static constexpr int A4988_PIN_UNUSED = -1;

enum class A4988Microstep : uint8_t {
    Full = 1,
    Half = 2,
    Quarter = 4,
    Eighth = 8,
    Sixteenth = 16,
};

class A4988Stepper {
public:
    A4988Stepper(
        uint step_pin,
        uint dir_pin,
        uint enable_pin,
        int ms1_pin = A4988_PIN_UNUSED,
        int ms2_pin = A4988_PIN_UNUSED,
        int ms3_pin = A4988_PIN_UNUSED,
        A4988Microstep microstep = A4988Microstep::Full)
        : step_pin_(step_pin),
          dir_pin_(dir_pin),
          enable_pin_(enable_pin),
          ms1_pin_(ms1_pin),
          ms2_pin_(ms2_pin),
          ms3_pin_(ms3_pin),
          microstep_(microstep) {
    }

    void init() {
        init_output(step_pin_, false);
        init_output(dir_pin_, false);
        init_output(enable_pin_, true);
        init_optional_output(ms1_pin_, false);
        init_optional_output(ms2_pin_, false);
        init_optional_output(ms3_pin_, false);
        set_microstep(microstep_);
    }

    void enable() {
        gpio_put(enable_pin_, 0);
    }

    void disable() {
        gpio_put(enable_pin_, 1);
    }

    void set_microstep(A4988Microstep microstep) {
        microstep_ = microstep;

        switch (microstep) {
        case A4988Microstep::Full:
            set_microstep_pins(false, false, false);
            break;
        case A4988Microstep::Half:
            set_microstep_pins(true, false, false);
            break;
        case A4988Microstep::Quarter:
            set_microstep_pins(false, true, false);
            break;
        case A4988Microstep::Eighth:
            set_microstep_pins(true, true, false);
            break;
        case A4988Microstep::Sixteenth:
            set_microstep_pins(true, true, true);
            break;
        }
    }

    uint32_t microsteps() const {
        return static_cast<uint32_t>(microstep_);
    }

    uint32_t steps_per_motor_revolution(uint32_t full_steps_per_revolution) const {
        return full_steps_per_revolution * microsteps();
    }

    void move_steps(int32_t steps, uint32_t period_us = A4988_STEP_PERIOD_US) {
        if (steps == 0) {
            return;
        }

        gpio_put(dir_pin_, steps > 0 ? 1 : 0);

        const uint32_t low_us = period_us > A4988_STEP_HIGH_US
            ? period_us - A4988_STEP_HIGH_US
            : 0;
        const uint32_t count = abs_steps(steps);

        for (uint32_t i = 0; i < count; ++i) {
            gpio_put(step_pin_, 1);
            sleep_us(A4988_STEP_HIGH_US);
            gpio_put(step_pin_, 0);
            sleep_us(low_us);
        }
    }

private:
    static bool has_pin(int pin) {
        return pin >= 0;
    }

    static void init_output(uint pin, bool high) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_put(pin, high ? 1 : 0);
    }

    static void init_optional_output(int pin, bool high) {
        if (has_pin(pin)) {
            init_output(static_cast<uint>(pin), high);
        }
    }

    static void set_optional_pin(int pin, bool high) {
        if (has_pin(pin)) {
            gpio_put(static_cast<uint>(pin), high ? 1 : 0);
        }
    }

    static uint32_t abs_steps(int32_t steps) {
        return steps > 0
            ? static_cast<uint32_t>(steps)
            : static_cast<uint32_t>(-(steps + 1)) + 1;
    }

    void set_microstep_pins(bool ms1, bool ms2, bool ms3) {
        set_optional_pin(ms1_pin_, ms1);
        set_optional_pin(ms2_pin_, ms2);
        set_optional_pin(ms3_pin_, ms3);
    }

    uint step_pin_;
    uint dir_pin_;
    uint enable_pin_;
    int ms1_pin_;
    int ms2_pin_;
    int ms3_pin_;
    A4988Microstep microstep_;
};
