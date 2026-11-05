/**
 * Unit tests for the scoreboard i2c control program.
 *
 * The wiringPi i2c functions are replaced by a simulated bus that records
 * every transfer, so the tests can check exactly which bytes would be sent
 * to the atmega without any hardware.
 *
 * Build and run with: make -C test
 */
#include <stdio.h>
#include <string.h>

/* Pull in the program under test, renaming its entry point */
#define main scoreboard_main
#include "../src/main.c"
#undef main

/* ---------------------------------------------------------------- */
/* Simulated i2c bus                                                 */
/* ---------------------------------------------------------------- */

#define MAX_TRANSFERS 16

typedef struct
{
	int is_reg16; /* 0: wiringPiI2CWrite, 1: wiringPiI2CWriteReg16 */
	int reg;
	int data;
} transfer;

static transfer bus[MAX_TRANSFERS];
static int bus_count;
static int setup_addr;
static int setup_result;
static int write_result;

static void bus_reset(void)
{
	memset(bus, 0, sizeof(bus));
	bus_count = 0;
	setup_addr = -1;
	setup_result = 3;
	write_result = 0;
}

int wiringPiI2CSetup(const int devId)
{
	setup_addr = devId;
	return setup_result;
}

int wiringPiI2CWrite(int fd, int data)
{
	(void) fd;
	if (bus_count < MAX_TRANSFERS)
	{
		bus[bus_count].is_reg16 = 0;
		bus[bus_count].reg = -1;
		bus[bus_count].data = data;
		bus_count++;
	}
	return write_result;
}

int wiringPiI2CWriteReg16(int fd, int reg, int data)
{
	(void) fd;
	if (bus_count < MAX_TRANSFERS)
	{
		bus[bus_count].is_reg16 = 1;
		bus[bus_count].reg = reg;
		bus[bus_count].data = data;
		bus_count++;
	}
	return write_result;
}

/* ---------------------------------------------------------------- */
/* Minimal test framework                                            */
/* ---------------------------------------------------------------- */

static int failures;
static int checks;

#define CHECK_EQ(actual, expected) do { \
	int a_ = (actual), e_ = (expected); \
	checks++; \
	if (a_ != e_) { \
		failures++; \
		fprintf(stderr, "  FAIL %s:%d: %s == %d, expected %d\n", \
			__FILE__, __LINE__, #actual, a_, e_); \
	} \
} while (0)

/* Run the program with the given arguments (argv[0] is added) */
static int run(int argc, const char * args[])
{
	char * argv[8];
	int i;
	argv[0] = (char *) "scoreboard";
	for (i = 0; i < argc && i < 7; i++)
	{
		argv[i + 1] = (char *) args[i];
	}
	argv[argc + 1] = NULL;

	/* Silence usage output of the program under test */
	FILE * saved = stderr;
	FILE * devnull = fopen("/dev/null", "w");
	if (devnull)
	{
		stderr = devnull;
	}
	int rc = scoreboard_main(argc + 1, argv);
	if (devnull)
	{
		stderr = saved;
		fclose(devnull);
	}
	return rc;
}

static void expect_write(int idx, int data)
{
	CHECK_EQ(bus[idx].is_reg16, 0);
	CHECK_EQ(bus[idx].data, data);
}

static void expect_reg16(int idx, int reg, int data)
{
	CHECK_EQ(bus[idx].is_reg16, 1);
	CHECK_EQ(bus[idx].reg, reg);
	CHECK_EQ(bus[idx].data, data);
}

/* ---------------------------------------------------------------- */
/* Test cases                                                        */
/* ---------------------------------------------------------------- */

static void test_no_args_prints_usage(void)
{
	bus_reset();
	CHECK_EQ(run(0, NULL), E_NOT_ENOUGH_ARGS);
	CHECK_EQ(bus_count, 0);
	CHECK_EQ(setup_addr, -1); /* no connection opened */
}

static void test_connection_error(void)
{
	const char * args[] = { "H1", "5" };
	bus_reset();
	setup_result = -1;
	CHECK_EQ(run(2, args), E_CONNECTION_ERROR);
	CHECK_EQ(bus_count, 0);
}

static void test_fd_zero_is_valid(void)
{
	const char * args[] = { "pwm", "5" };
	bus_reset();
	setup_result = 0;
	CHECK_EQ(run(2, args), E_SUCCESS);
	CHECK_EQ(bus_count, 3);
}

static void test_send_digit(void)
{
	const char * args[] = { "H1", "7" };
	bus_reset();
	CHECK_EQ(run(2, args), E_SUCCESS);
	CHECK_EQ(setup_addr, 0x10);
	CHECK_EQ(bus_count, 1);
	expect_write(0, '7');
}

static void test_send_only_first_symbol(void)
{
	const char * args[] = { "H1", "AB" };
	bus_reset();
	CHECK_EQ(run(2, args), E_SUCCESS);
	CHECK_EQ(bus_count, 1);
	expect_write(0, 'A');
}

static void test_send_without_digit_writes_blank(void)
{
	const char * args[] = { "H1" };
	bus_reset();
	CHECK_EQ(run(1, args), E_SUCCESS);
	CHECK_EQ(bus_count, 1);
	expect_write(0, ' ');
}

static void test_send_error(void)
{
	const char * args[] = { "H1", "7" };
	bus_reset();
	write_result = -1;
	CHECK_EQ(run(2, args), E_FAILURE_SEND_CMD);
}

static void test_pwm_without_value(void)
{
	const char * args[] = { "pwm" };
	bus_reset();
	CHECK_EQ(run(1, args), E_NOT_ENOUGH_ARGS);
	CHECK_EQ(bus_count, 0);
}

static void test_pwm_on_enables(void)
{
	const char * args[] = { "pwm", "on" };
	bus_reset();
	CHECK_EQ(run(2, args), E_SUCCESS);
	CHECK_EQ(bus_count, 1);
	expect_write(0, CMD_PWM_ENABLE); /* 0x03 */
}

static void test_pwm_off_disables(void)
{
	const char * args[] = { "pwm", "off" };
	bus_reset();
	CHECK_EQ(run(2, args), E_SUCCESS);
	CHECK_EQ(bus_count, 1);
	expect_write(0, CMD_PWM_DISABLE); /* 0x02 */
}

static void test_pwm_brightness_step(void)
{
	const char * args[] = { "pwm", "5" };
	bus_reset();
	CHECK_EQ(run(2, args), E_SUCCESS);
	CHECK_EQ(bus_count, 3);
	/* (10000 - 200) / 10 * 5 + 200 = 5100 for on and off time */
	expect_reg16(0, CMD_PWM_ON_TIME, 5100);
	expect_reg16(1, CMD_PWM_OFF_TIME, 5100);
	expect_write(2, CMD_PWM_APPLY);
}

static void test_pwm_brightness_extremes(void)
{
	const char * max_args[] = { "pwm", "10" };
	const char * min_args[] = { "pwm", "0" };

	bus_reset();
	CHECK_EQ(run(2, max_args), E_SUCCESS);
	expect_reg16(0, CMD_PWM_ON_TIME, 10000);
	expect_reg16(1, CMD_PWM_OFF_TIME, 200);

	bus_reset();
	CHECK_EQ(run(2, min_args), E_SUCCESS);
	expect_reg16(0, CMD_PWM_ON_TIME, 200);
	expect_reg16(1, CMD_PWM_OFF_TIME, 10000);
}

static void test_pwm_brightness_is_clamped(void)
{
	const char * high_args[] = { "pwm", "15" };
	const char * low_args[] = { "pwm", "-3" };

	bus_reset();
	CHECK_EQ(run(2, high_args), E_SUCCESS);
	expect_reg16(0, CMD_PWM_ON_TIME, 10000);
	expect_reg16(1, CMD_PWM_OFF_TIME, 200);

	bus_reset();
	CHECK_EQ(run(2, low_args), E_SUCCESS);
	expect_reg16(0, CMD_PWM_ON_TIME, 200);
	expect_reg16(1, CMD_PWM_OFF_TIME, 10000);
}

static void test_pwm_raw_clock_cycles(void)
{
	const char * args[] = { "pwm", "100", "200" };
	bus_reset();
	CHECK_EQ(run(3, args), E_SUCCESS);
	CHECK_EQ(bus_count, 3);
	expect_reg16(0, CMD_PWM_ON_TIME, 100);
	expect_reg16(1, CMD_PWM_OFF_TIME, 200);
	expect_write(2, CMD_PWM_APPLY);
}

static void test_pwm_write_error(void)
{
	const char * args[] = { "pwm", "on" };
	bus_reset();
	write_result = -1;
	CHECK_EQ(run(2, args), E_FAILURE_PWM);
}

int main(void)
{
	test_no_args_prints_usage();
	test_connection_error();
	test_fd_zero_is_valid();
	test_send_digit();
	test_send_only_first_symbol();
	test_send_without_digit_writes_blank();
	test_send_error();
	test_pwm_without_value();
	test_pwm_on_enables();
	test_pwm_off_disables();
	test_pwm_brightness_step();
	test_pwm_brightness_extremes();
	test_pwm_brightness_is_clamped();
	test_pwm_raw_clock_cycles();
	test_pwm_write_error();

	printf("%d checks, %d failures\n", checks, failures);
	return failures ? 1 : 0;
}
