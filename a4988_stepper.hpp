#pragma once

#include <cmath>
#include <cstdint>

#include "pico/stdlib.h"

class A4988Stepper {
public:
    A4988Stepper(uint step_pin,
                 uint dir_pin,
                 uint enable_pin,
                 float steps_per_revolution,
                 float microsteps,
                 float gear_ratio = 1.0f,
                 bool dir_inverted = false,
                 bool enable_active_low = true)
        : step_pin_(step_pin),
          dir_pin_(dir_pin),
          enable_pin_(enable_pin),
          steps_per_revolution_(steps_per_revolution),
          microsteps_(microsteps),
          gear_ratio_(gear_ratio),
          dir_inverted_(dir_inverted),
          enable_active_low_(enable_active_low) {
    }

    void init() {
        gpio_init(step_pin_);
        gpio_set_dir(step_pin_, GPIO_OUT);
        gpio_put(step_pin_, 0);

        gpio_init(dir_pin_);
        gpio_set_dir(dir_pin_, GPIO_OUT);
        gpio_put(dir_pin_, 0);

        gpio_init(enable_pin_);
        gpio_set_dir(enable_pin_, GPIO_OUT);
        disable();
    }

    void enable() {
        gpio_put(enable_pin_, enable_active_low_ ? 0 : 1);
    }

    void disable() {
        gpio_put(enable_pin_, enable_active_low_ ? 1 : 0);
    }

    void set_direction(bool positive) {
        const bool dir_level = dir_inverted_ ? !positive : positive;
        gpio_put(dir_pin_, dir_level ? 1 : 0);
    }

    void step_pulse(uint32_t high_us = 3, uint32_t low_us = 700) {
        gpio_put(step_pin_, 1);
        sleep_us(high_us);
        gpio_put(step_pin_, 0);
        sleep_us(low_us);
    }

    long degrees_to_steps(float degrees) const {
        const float steps_per_degree = (steps_per_revolution_ * microsteps_ * gear_ratio_) / 360.0f;
        return lroundf(degrees * steps_per_degree);
    }

    void move_relative_degrees(float delta_degrees, uint32_t step_period_us = 700) {
        const long steps = degrees_to_steps(delta_degrees);
        move_relative_steps(steps, step_period_us);
        commanded_degrees_ += delta_degrees;
    }

    void move_absolute_degrees(float target_degrees, uint32_t step_period_us = 700) {
        move_relative_degrees(target_degrees - commanded_degrees_, step_period_us);
    }

    float commanded_degrees() const {
        return commanded_degrees_;
    }

    long current_steps() const {
        return current_steps_;
    }

    unsigned long total_steps() const {
        return total_steps_;
    }

    float actual_degrees() const {
        return steps_to_degrees(current_steps_);
    }

    float step_angle_degrees() const {
        return steps_to_degrees(1);
    }

    float steps_to_degrees(long steps) const {
        const float steps_per_degree = (steps_per_revolution_ * microsteps_ * gear_ratio_) / 360.0f;
        return static_cast<float>(steps) / steps_per_degree;
    }

private:
    void move_relative_steps(long steps, uint32_t step_period_us) {
        if (steps == 0) {
            return;
        }

        set_direction(steps > 0);

        const unsigned long count = static_cast<unsigned long>(steps > 0 ? steps : -steps);
        for (unsigned long i = 0; i < count; ++i) {
            step_pulse(3, step_period_us);
        }

        current_steps_ += steps;
        total_steps_ += count;
    }

    uint step_pin_;
    uint dir_pin_;
    uint enable_pin_;
    float steps_per_revolution_;
    float microsteps_;
    float gear_ratio_;
    bool dir_inverted_;
    bool enable_active_low_;
    float commanded_degrees_ {0.0f};
    long current_steps_ {0};
    unsigned long total_steps_ {0};
};
