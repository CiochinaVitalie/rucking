#include <stdio.h>

#include "a4988_stepper.hpp"
#include "pico/stdlib.h"

static constexpr uint PAN_STEP_PIN = 2;
static constexpr uint PAN_DIR_PIN = 3;
static constexpr uint PAN_ENABLE_PIN = 4;
static constexpr int PAN_MS1_PIN = A4988_PIN_UNUSED;
static constexpr int PAN_MS2_PIN = A4988_PIN_UNUSED;
static constexpr int PAN_MS3_PIN = A4988_PIN_UNUSED;
static constexpr uint32_t PAN_FULL_STEPS_PER_REVOLUTION = 4640;
static constexpr float PAN_GEAR_RATIO = 1.0f;
static constexpr A4988Microstep PAN_MICROSTEP = A4988Microstep::Full;
static constexpr float PAN_TEST_DEGREES = 360.0f;

static int32_t round_to_steps(float steps)
{
    return static_cast<int32_t>(steps >= 0.0f ? steps + 0.5f : steps - 0.5f);
}

static float output_steps_per_revolution(const A4988Stepper& stepper)
{
    const float motor_steps = static_cast<float>(
        stepper.steps_per_motor_revolution(PAN_FULL_STEPS_PER_REVOLUTION));
    return motor_steps * PAN_GEAR_RATIO;
}

static int32_t output_degrees_to_steps(const A4988Stepper& stepper, float degrees)
{
    return round_to_steps((output_steps_per_revolution(stepper) * degrees) / 360.0f);
}

static void move_output_degrees(A4988Stepper& stepper, float degrees)
{
    stepper.move_steps(output_degrees_to_steps(stepper, degrees));
}

static void run_a4988_driver_test(A4988Stepper& stepper)
{
    printf("=== A4988 driver test start ===\n");
    printf("Rotating PAN output by %.3f degrees.\n", PAN_TEST_DEGREES);
    move_output_degrees(stepper, PAN_TEST_DEGREES);

    while (true) {
        sleep_ms(1000);
    }
}

int main()
{
    stdio_init_all();
    
    A4988Stepper pan(
        PAN_STEP_PIN,
        PAN_DIR_PIN,
        PAN_ENABLE_PIN,
        PAN_MS1_PIN,
        PAN_MS2_PIN,
        PAN_MS3_PIN,
        PAN_MICROSTEP);

    pan.init();
    pan.enable();

    printf("A4988 stepper driver test start\n");
    printf("PAN pins: STEP=%u DIR=%u EN=%u\n", PAN_STEP_PIN, PAN_DIR_PIN, PAN_ENABLE_PIN);
    printf("PAN MS pins: MS1=%d MS2=%d MS3=%d\n", PAN_MS1_PIN, PAN_MS2_PIN, PAN_MS3_PIN);
    printf("Microstep mode: 1/%lu\n", static_cast<unsigned long>(pan.microsteps()));
    printf("Gear ratio: %.3f:1\n", PAN_GEAR_RATIO);
    printf("Motor steps per revolution: %lu\n",
           static_cast<unsigned long>(pan.steps_per_motor_revolution(PAN_FULL_STEPS_PER_REVOLUTION)));
    printf("Output steps per revolution: %lu\n",
           static_cast<unsigned long>(round_to_steps(output_steps_per_revolution(pan))));
    printf("Output steps per degree: %.3f\n", output_steps_per_revolution(pan) / 360.0f);
    printf("Test move: %.3f deg => %ld steps\n",
           PAN_TEST_DEGREES,
           static_cast<long>(output_degrees_to_steps(pan, PAN_TEST_DEGREES)));
    printf("Step timing: pulse=%lu us period=%lu us\n",
           static_cast<unsigned long>(A4988_STEP_HIGH_US),
           static_cast<unsigned long>(A4988_STEP_PERIOD_US));

    run_a4988_driver_test(pan);
}
