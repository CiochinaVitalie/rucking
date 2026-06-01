#include <stdio.h>

#include "a4988_stepper.hpp"
#include "pico/stdlib.h"

static constexpr uint PAN_STEP_PIN = 2;
static constexpr uint PAN_DIR_PIN = 3;
static constexpr uint PAN_ENABLE_PIN = 4;

static constexpr float A4988_TEST_ONE_REV_DEG = 360.0f;

static constexpr A4988StepperConfig PAN_CONFIG {
    .full_steps_per_revolution = 200.0f,
    .microstep_divider = 16.0f,
    .gear_ratio = 1.0f,
    .direction_inverted = false,
    .enable_active_low = true,
    .step_high_time_us = 3,
    .min_step_period_us = 700,
    .min_position_degrees = -360.0f,
    .max_position_degrees = 360.0f,
};

static void log_stepper_state(const char* axis_name, const A4988Stepper& stepper)
{
    printf("%s state: cmd=%.3f deg actual=%.3f deg steps=%ld total=%lu limits=[%.1f, %.1f]\n",
           axis_name,
           stepper.commanded_degrees(),
           stepper.actual_degrees(),
           stepper.current_steps(),
           stepper.total_steps(),
           stepper.config().min_position_degrees,
           stepper.config().max_position_degrees);
}

static void run_a4988_driver_test(A4988Stepper& stepper)
{
    printf("=== A4988 driver test start ===\n");
    printf("This test rotates PAN by one full revolution and then idles.\n");
    log_stepper_state("PAN", stepper);

    stepper.move_relative_degrees(A4988_TEST_ONE_REV_DEG);
    log_stepper_state("PAN", stepper);

    while (true) {
        sleep_ms(1000);
    }
}

int main()
{
    stdio_init_all();
    
    A4988Stepper pan(PAN_STEP_PIN, PAN_DIR_PIN, PAN_ENABLE_PIN, PAN_CONFIG);

    pan.init();
    pan.enable();

    printf("A4988 stepper driver test start\n");
    printf("PAN pins: STEP=%u DIR=%u EN=%u\n", PAN_STEP_PIN, PAN_DIR_PIN, PAN_ENABLE_PIN);
    printf("Step angle: PAN=%.4f deg/step\n", pan.step_angle_degrees());
    printf("Step timing: pulse=%lu us min_period=%lu us\n",
           static_cast<unsigned long>(pan.config().step_high_time_us),
           static_cast<unsigned long>(pan.config().min_step_period_us));

    run_a4988_driver_test(pan);
}
