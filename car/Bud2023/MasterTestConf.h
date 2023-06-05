#ifndef MASTERCONF_H
#define MASTERCONF_H

#include "mbed.h"
#include "hardware/Pins_MasterTest.h"
#define MESSAGE_REPORT

DigitalOut buzzer_out(MASTER_BUZZER_OUT);
DigitalOut brakelight_out(MASTER_BRAKELIGHT_OUT);
DigitalOut pump_pwm_out(MASTER_PUMP_PWM_OUT);
DigitalOut fans_pwm_out(MASTER_FANS_PWM_OUT);
DigitalOut run_out(MASTER_RUN_OUT);
DigitalOut rfe_out(MASTER_RFE_OUT);
DigitalOut bspd_test_out(MASTER_BSPD_TEST_OUT);
DigitalIn dout1_in(MASTER_DOUT1_IN);
DigitalIn dout2_in(MASTER_DOUT2_IN);
DigitalIn pump_pwm_in(MASTER_PUMP_PWM_IN);
DigitalIn rpm_sensor_rear_right(MASTER_RPM_SENSOR_REAR_RIGHT);
DigitalIn rpm_sensor_rear_left(MASTER_RPM_SENSOR_REAR_LEFT);
DigitalIn brake_pressure_sensor(MASTER_BRAKE_PRESSURE_SENSOR);
DigitalIn shutdown_bspd_monitoring(MASTER_SHUTDOWN_BSPD_MONITORING);
DigitalIn shutdown_hvd_monitoring(MASTER_SHUTDOWN_HVD_MONITORING);
DigitalIn shutdown_mainhoop_monitoring(MASTER_SHUTDOWN_MAINHOOP_MONITORING);
DigitalIn shutdown_accu_monitoring(MASTER_SHUTDOWN_ACCU_MONITORING);
DigitalIn shutdown_tsms_monitoring(MASTER_SHUTDOWN_TSMS_MONITORING);
DigitalIn shutdown_inverter_monitoring(MASTER_SHUTDOWN_INVERTER_MONITORING);

Serial pc(USBTX, USBRX); // Connection to PC over Serial

class Master
{
public:
    // mbed write DigitalOut high
    void buzzer_on()
    {
        buzzer_out = 1;
        pc.printf("buzzer on\n");
    }
    void brakelight_on()
    {
        brakelight_out = 1;
        pc.printf("brakelight on\n");
    }
    void pump_pwm_on()
    {
        pump_pwm_out = 1;
        pc.printf("pump_pwm on\n");
    }
    void fans_pwm_on()
    {
        fans_pwm_out = 1;
        pc.printf("fans_pwm on\n");
    }
    void run_on()
    {
        run_out = 1;
        pc.printf("run on\n");
    }
    void rfe_on()
    {
        rfe_out = 1;
        pc.printf("rfe on\n");
    }
    void bspd_test_on()
    {
        bspd_test_out = 1;
        pc.printf("bspd_test on\n");
    }
    // mbed write DigitalOut low
    void buzzer_off()
    {
        buzzer_out = 0;
        pc.printf("buzzer off\n");
    }
    void brakelight_off()
    {
        brakelight_out = 0;
        pc.printf("brakelight off\n");
    }
    void pump_pwm_off()
    {
        pump_pwm_out = 0;
        pc.printf("pump_pwm off\n");
    }
    void fans_pwm_off()
    {
        fans_pwm_out = 0;
        pc.printf("fans_pwm off\n");
    }
    void run_off()
    {
        run_out = 0;
        pc.printf("run off\n");
    }
    void rfe_off()
    {
        rfe_out = 0;
        pc.printf("rfe off\n");
    }
    void bspd_test_off()
    {
        bspd_test_out = 0;
        pc.printf("bspd_test off\n");
    }

    // mbed read DigitalIn
    int dout1_read() { return dout1_in; }
    int dout2_read() { return dout2_in; }
    int pump_pwm_read() { return pump_pwm_in; }
    int rpm_sensor_rear_right_read() { return rpm_sensor_rear_right; }
    int rpm_sensor_rear_left_read() { return rpm_sensor_rear_left; }
    int brake_pressure_sensor_read() { return brake_pressure_sensor; }
    int shutdown_bspd_monitoring_read() { return shutdown_bspd_monitoring; }
    int shutdown_hvd_monitoring_read() { return shutdown_hvd_monitoring; }
    int shutdown_mainhoop_monitoring_read() { return shutdown_mainhoop_monitoring; }
    int shutdown_accu_monitoring_read() { return shutdown_accu_monitoring; }
    int shutdown_tsms_monitoring_read() { return shutdown_tsms_monitoring; }
    int shutdown_inverter_monitoring_read() { return shutdown_inverter_monitoring; }

    void printAllValues()
    {
        // print all inputs to serial
        pc.printf("dout1_in: %d\n", dout1_in.read());
        pc.printf("dout2_in: %d\n", dout2_in.read());
        pc.printf("pump_pwm_in: %d\n", pump_pwm_in.read());
        pc.printf("rpm_sensor_rear_right: %d\n", rpm_sensor_rear_right.read());
        pc.printf("rpm_sensor_rear_left: %d\n", rpm_sensor_rear_left.read());
        pc.printf("brake_pressure_sensor: %d\n", brake_pressure_sensor.read());
        pc.printf("shutdown_bspd_monitoring: %d\n", shutdown_bspd_monitoring.read());
        pc.printf("shutdown_hvd_monitoring: %d\n", shutdown_hvd_monitoring.read());
        pc.printf("shutdown_mainhoop_monitoring: %d\n", shutdown_mainhoop_monitoring.read());
        pc.printf("shutdown_accu_monitoring: %d\n", shutdown_accu_monitoring.read());
        pc.printf("shutdown_tsms_monitoring: %d\n", shutdown_tsms_monitoring.read());
        pc.printf("shutdown_inverter_monitoring: %d\n", shutdown_inverter_monitoring.read());
    }

    void setup()
    {
        pc.baud(9600);
        pc.printf("MasterTest\n");

        dout1_in.mode(PullUp);
        dout2_in.mode(PullUp);
        pump_pwm_in.mode(PullUp);
        rpm_sensor_rear_right.mode(PullUp);
        rpm_sensor_rear_left.mode(PullUp);
        brake_pressure_sensor.mode(PullUp);
        shutdown_bspd_monitoring.mode(PullUp);
        shutdown_hvd_monitoring.mode(PullUp);
        shutdown_mainhoop_monitoring.mode(PullUp);
        shutdown_accu_monitoring.mode(PullUp);
        shutdown_tsms_monitoring.mode(PullUp);
        shutdown_inverter_monitoring.mode(PullUp);
    }

    void loop()
    {
        // // read the serial and set the outputs accordingly (for testing)
        if (pc.readable())
        {
            char c = pc.getc();
            if (c == '1')
            {
                buzzer_on();
            }
            else if (c == '2')
            {
                buzzer_off();
            }
            else if (c == '3')
            {
                brakelight_on();
            }
            else if (c == '4')
            {
                brakelight_off();
            }
            else if (c == '5')
            {
                pump_pwm_on();
            }
            else if (c == '6')
            {
                pump_pwm_off();
            }
            else if (c == '7')
            {
                fans_pwm_on();
            }
            else if (c == '8')
            {
                fans_pwm_off();
            }
            else if (c == '9')
            {
                run_on();
            }
            else if (c == '0')
            {
                run_off();
            }
            else if (c == 'q')
            {
                rfe_on();
            }
            else if (c == 'w')
            {
                rfe_off();
            }
            else if (c == 'e')
            {
                bspd_test_on();
            }
            else if (c == 'r')
            {
                bspd_test_off();
            }
            else if (c == 'p')
            {
                printAllValues();
            }
        }
    }
};

Master runtime;

#endif
