#include <stdio.h>

#include "pico/stdlib.h"
#include "a4988_stepper.hpp"
#include <ruckig/ruckig.hpp>
#include "hardware/uart.h"

using namespace ruckig;

static constexpr uint PAN_STEP_PIN = 2;
static constexpr uint PAN_DIR_PIN = 3;
static constexpr uint PAN_ENABLE_PIN = 4;

static constexpr uint TILT_STEP_PIN = 5;
static constexpr uint TILT_DIR_PIN = 6;
static constexpr uint TILT_ENABLE_PIN = 7;

const uint UART_TX_PIN = 0;
const uint UART_RX_PIN = 1;
const uint BAUD_RATE = 115200;

static constexpr float A4988_TEST_SWEEP_DEG = 45.0f;
static constexpr uint32_t A4988_TEST_STEP_PERIOD_US = 1200;
static constexpr uint32_t A4988_TEST_SETTLE_MS = 500;

static void log_stepper_state(const char* axis_name, const A4988Stepper& stepper)
{
    printf("%s state: cmd=%.3f deg actual=%.3f deg steps=%ld total=%lu\n",
           axis_name,
           stepper.commanded_degrees(),
           stepper.actual_degrees(),
           stepper.current_steps(),
           stepper.total_steps());
}

static void run_a4988_axis_test(const char* axis_name,
                                A4988Stepper& stepper,
                                float sweep_degrees,
                                uint32_t step_period_us)
{
    printf("%s continuous one-direction test start: step=%.3f deg step_period=%lu us\n",
           axis_name,
           sweep_degrees,
           static_cast<unsigned long>(step_period_us));
    log_stepper_state(axis_name, stepper);
}

static void run_a4988_driver_test(A4988Stepper& pan)
{
    printf("=== A4988 driver test start ===\n");
    printf("This test rotates PAN continuously in one direction.\n");
    run_a4988_axis_test("PAN", pan, A4988_TEST_SWEEP_DEG, A4988_TEST_STEP_PERIOD_US);

    while (true) {
        pan.move_relative_degrees(A4988_TEST_SWEEP_DEG, A4988_TEST_STEP_PERIOD_US);

        log_stepper_state("PAN", pan);
        sleep_ms(A4988_TEST_SETTLE_MS);
    }
}

int main()
{
    stdio_init_all();

    uart_init(uart0, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    
    A4988Stepper pan(PAN_STEP_PIN, PAN_DIR_PIN, PAN_ENABLE_PIN, 200.0f, 16.0f);
    A4988Stepper tilt(TILT_STEP_PIN, TILT_DIR_PIN, TILT_ENABLE_PIN, 200.0f, 16.0f);

    pan.init();
    tilt.init();
    pan.enable();
    tilt.enable();

    Ruckig<2> otg(0.01); // 10 ms control cycle
    InputParameter<2> input;
    OutputParameter<2> output;

    // Degrees-based planning.
    input.current_position = {0.0, 0.0};
    input.current_velocity = {0.0, 0.0};
    input.current_acceleration = {0.0, 0.0};

    input.target_position = {90.0, -45.0};
    input.target_velocity = {0.0, 0.0};
    input.target_acceleration = {0.0, 0.0};

    input.max_velocity = {180.0, 180.0};     // deg/s
    input.max_acceleration = {360.0, 360.0}; // deg/s^2
    input.max_jerk = {1000.0, 1000.0};       // deg/s^3

    printf("Ruckig + A4988 test start\n");
    printf("PAN pins: STEP=%u DIR=%u EN=%u\n", PAN_STEP_PIN, PAN_DIR_PIN, PAN_ENABLE_PIN);
    printf("TILT pins: STEP=%u DIR=%u EN=%u\n", TILT_STEP_PIN, TILT_DIR_PIN, TILT_ENABLE_PIN);
    printf("Step angle: PAN=%.4f deg/step  TILT=%.4f deg/step\n",
           pan.step_angle_degrees(),
           tilt.step_angle_degrees());

    run_a4988_driver_test(pan);
}
