#ifdef EXPERIMENTAL_DISPLAY_ACTIVE
#ifndef SOFTWAREMOTORCONTROLLER_H
#define SOFTWAREMOTORCONTROLLER_H
#include "../interface/IMotorController.h"

class SoftwareMotorController : public IMotorController {
    
    public:
        SoftwareMotorController() {
            setComponentType(COMPONENT_DISPLAY);
            setObjectType(OBJECT_SOFTWARE);
            _valueAge.start();
        }

        SoftwareMotorController(id_sub_component_t componentSubId)
            : SoftwareMotorController() {
            setComponentSubId(componentSubId);
        }
        // Status
        virtual motor_controller_status_t getStatus(){
            return _status;
        }

        // State (-> Status got from the Motor Controller)
        virtual motor_controller_state_t getState(){
            return _state;
        }

        // Speed
        virtual motor_controller_speed_t getSpeed(){
            return _speed;
        }
        virtual float getSpeedAge(){
            return _speedAge;
        }

        // Current
        virtual float getCurrent(){
            return _current;
        }

        // Temperature
        virtual int16_t getMotorTemp(){
            return _motorTemp;
        }
        virtual float getMotorTempAge(){
            return _motorTempAge;
        }
        virtual int16_t getServoTemp(){
            return _servoTemp;
        }  
        virtual float getServoTempAge(){
            return _servoTempAge;
        }
        virtual int16_t getAirTemp(){
            return _airTemp;
        }
        virtual float getAirTempAge(){
            return _airTempAge;
        }

        // Voltage
        virtual float getDcVoltage(){
            return _dcVoltage;
        }
        virtual float getDcVoltageAge(){
            return _dcVoltageAge;
        }

        
        void beginCommunication() {}

        void setTorque(float torque){}

        virtual message_build_result_t buildMessage(CarMessage &carMessage) {
            car_sub_message_t subMessage;

            subMessage.length = 1;

            motor_controller_status_t motorStatus = this->getStatus();
            uint8_t valueToSent = 0;

            if (motorStatus == MOTOR_CONTROLLER_BAD_VAR_GIVEN) {
                valueToSent = 1;
            } else if (motorStatus == MOTOR_CONTROLLER_ERROR) {
                valueToSent = 2;
            }

            subMessage.data[0] = valueToSent;

            #ifdef PEDAL_MESSAGE_HANDLER_DEBUG
                pcSerial.printf("[SoftwarePedal]@buildMessage: SoftwareObject calibrationStatus: 0x%x\tmsg.data[0]: 0x%x\tvalueToSent: %i\n", calibrationStatus, subMessage.data[0], valueToSent);
            #endif

            carMessage.addSubMessage(subMessage);

            return MESSAGE_BUILD_OK;
        }

        virtual message_parse_result_t parseMessage(CarMessage &carMessage) {
            message_parse_result_t result = MESSAGE_PARSE_OK;
            for (car_sub_message_t &subMessage : carMessage.subMessages) {
                if(subMessage.length != 11) // not a valid message
                    result = MESSAGE_PARSE_ERROR;

                this->setStatus(subMessage.data[0]);
                this->setState(subMessage.data[1]);

                uint16_t speed16 = subMessage.data[2] | (subMessage.data[3] << 8);
                float speed = (float)speed16 / 65535.0;
                this->setSpeed(speed);

                uint16_t current16 = subMessage.data[4] | (subMessage.data[5] << 8);
                float current = (float)current16 / 65535.0;
                this->setCurrent(current);

                int16_t motorTemp = subMessage.data[6];
                this->setMotorTemp(motorTemp);
                
                uint16_t servoTemp = subMessage.data[7];
                this->setServoTemp(servoTemp);
                
                uint16_t airTemp = subMessage.data[8];
                this->setAirTemp(airTemp);

                uint16_t dcVoltage16 = subMessage.data[9] | (subMessage.data[10] << 8);
                float dcVoltage = (float)dcVoltage16 / 65535.0;
                this->setDCVoltage(dcVoltage);

                _valueAge.reset();
                _valueAge.start();

                #ifdef PEDAL_MESSAGE_HANDLER_DEBUG
                    pcSerial.printf("[SoftwarePedal]@parseMessage: HardwareObject (float)pedalValue: %.3f\t(uint16_t)pedalValue: %i\tmsg.data[1]: 0x%x\tmsg.data[2]: 0x%x\n", newValue, newValue16, subMessage.data[1], subMessage.data[2]);
                #endif

            }
            
            return result;
        }

        float getValueAge() {
            return _valueAge.read();
        }

        void resetAge() {
            _valueAge.reset();
            _valueAge.start();
        }

    private:
        motor_controller_status_t _status;
        motor_controller_state_t _state;

        motor_controller_rfe_enable_t _rfeEnabled;
        motor_controller_run_enable_t _runEnabled;

        Timer _valueAge;
        Timer _speedAge;
        Timer _motorTempAge;
        Timer _servoTempAge;
        Timer _airTempAge;
        Timer _dcVoltageAge;

        motor_controller_speed_t _speed;
        motor_controller_current_t _current;
        motor_controller_motor_temp_t _motorTemp;
        motor_controller_servo_temp_t _servoTemp;
        motor_controller_air_temp_t _airTemp;
        motor_controller_dc_voltage_t _dcVoltage;

        virtual void setStatus(motor_controller_status_t status) {
            _status = status;
        }
        
        virtual void setState(motor_controller_state_t state) {
            _state = state;
        }

        virtual void setRFE(motor_controller_rfe_enable_t rfe) {
            _rfeEnabled = rfe;
        }
        
        virtual void setRUN(motor_controller_run_enable_t run) {
            _runEnabled = run;
        }

        virtual void setSpeed(motor_controller_speed_t speed) {
            _speed = speed;
        }
        
        virtual void setCurrent(motor_controller_current_t current) {
            _current = current;
        }

        virtual void setMotorTemp(motor_controller_motor_temp_t motorTemp) {
            _motorTemp = motorTemp;
        }

        virtual void setServoTemp(motor_controller_servo_temp_t servoTemp) {
            _servoTemp = servoTemp;
        }
        
        virtual void setAirTemp(motor_controller_air_temp_t airTemp) {
            _airTemp = airTemp;
        }

        virtual void setDCVoltage(motor_controller_dc_voltage_t dcVoltage) {
            _dcVoltage = dcVoltage;
        }
};

#endif // SOFTWAREMOTORCONTROLLER_H
#endif