#pragma once

#include <cmath>
#include <cstdint>

#include "pico/stdlib.h"

struct A4988StepperConfig {
    float full_steps_per_revolution {200.0f};
    float microstep_divider {1.0f};
    float gear_ratio {1.0f};

    bool direction_inverted {false};
    bool enable_active_low {true};

    uint32_t step_high_time_us {3};
    uint32_t min_step_period_us {700};
    uint32_t dir_setup_time_us {3};
    uint32_t dir_hold_time_us {3};

    float min_position_degrees {-360.0f};
    float max_position_degrees {360.0f};
};

class A4988Stepper {
public:
    A4988Stepper(uint step_pin, uint dir_pin, uint enable_pin, const A4988StepperConfig& config)
        : step_pin_(step_pin),
          dir_pin_(dir_pin),
          enable_pin_(enable_pin),
          config_(config) {
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
        gpio_put(enable_pin_, config_.enable_active_low ? 0 : 1);
    }

    void disable() {
        gpio_put(enable_pin_, config_.enable_active_low ? 1 : 0);
    }

    void set_direction(bool positive) {
        const bool dir_level = config_.direction_inverted ? !positive : positive;
        gpio_put(dir_pin_, dir_level ? 1 : 0);
    }

    void step_pulse(uint32_t high_us, uint32_t low_us) {
        gpio_put(step_pin_, 1);
        sleep_us(high_us);
        gpio_put(step_pin_, 0);
        sleep_us(low_us);
    }

    long degrees_to_steps(float degrees) const {
        const float steps_per_degree = mechanical_steps_per_revolution() / 360.0f;
        return lroundf(degrees * steps_per_degree);
    }

    void move_relative_degrees(float delta_degrees, uint32_t step_period_us = 0) {
        const float target_degrees = clamp_target_degrees(commanded_degrees_ + delta_degrees);
        const long steps = degrees_to_steps(target_degrees - commanded_degrees_);
        move_relative_steps(steps, step_period_us);
        commanded_degrees_ = target_degrees;
    }

    void move_relative_degrees_unclamped(float delta_degrees, uint32_t step_period_us = 0) {
        const long steps = degrees_to_steps(delta_degrees);
        move_relative_steps(steps, step_period_us);
        commanded_degrees_ += delta_degrees;
    }

    void move_absolute_degrees(float target_degrees, uint32_t step_period_us = 0) {
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

    const A4988StepperConfig& config() const {
        return config_;
    }

    float steps_to_degrees(long steps) const {
        const float steps_per_degree = mechanical_steps_per_revolution() / 360.0f;
        return static_cast<float>(steps) / steps_per_degree;
    }

private:
    static float clamp_value(float value, float minimum, float maximum) {
        if (value < minimum) {
            return minimum;
        }
        if (value > maximum) {
            return maximum;
        }
        return value;
    }

    float clamp_target_degrees(float target_degrees) const {
        return clamp_value(
            target_degrees,
            config_.min_position_degrees,
            config_.max_position_degrees);
    }

    float mechanical_steps_per_revolution() const {
        return config_.full_steps_per_revolution * config_.microstep_divider * config_.gear_ratio;
    }

    void move_relative_steps(long steps, uint32_t step_period_us) {
        if (steps == 0) {
            return;
        }

        const uint32_t effective_period_us = step_period_us > config_.min_step_period_us
            ? step_period_us
            : config_.min_step_period_us;
        const uint32_t low_time_us = effective_period_us > config_.step_high_time_us
            ? effective_period_us - config_.step_high_time_us
            : 0;

        set_direction(steps > 0);
        sleep_us(config_.dir_setup_time_us);

        const unsigned long count = static_cast<unsigned long>(steps > 0 ? steps : -steps);
        for (unsigned long i = 0; i < count; ++i) {
            step_pulse(config_.step_high_time_us, low_time_us);
        }
        sleep_us(config_.dir_hold_time_us);

        current_steps_ += steps;
        total_steps_ += count;
    }

    uint step_pin_;
    uint dir_pin_;
    uint enable_pin_;
    A4988StepperConfig config_;
    float commanded_degrees_ {0.0f};
    long current_steps_ {0};
    unsigned long total_steps_ {0};
};
