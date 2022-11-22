#ifndef SOFTWAREMOTORCONTROLLER_H
#define SOFTWAREMOTORCONTROLLER_H

#include "../interface/IMotorController.h"

class SoftwareMotorController : public IMotorController {
    public:
        SoftwareMotorController() {
            setComponentType(COMPONENT_MOTOR);
            setObjectType(OBJECT_SOFTWARE);
        }

        SoftwareMotorController(id_sub_component_t componentSubId)
            : SoftwareMotorController() {
            setComponentSubId(componentSubId);
        }

        virtual message_build_result_t buildMessage(CarMessage &carMessage) {
            
            return MESSAGE_BUILD_OK;
        }

        virtual message_parse_result_t parseMessage(CarMessage &carMessage) {
            message_parse_result_t result = MESSAGE_PARSE_OK;
            for (car_sub_message_t &subMessage : carMessage.subMessages) {
                

                switch (subMessage.data[0])
                {
                case 0:
                    /* code */
                    break;
                
                case 1:
                    /* code */
                    break;
                case 2:
                    /* code */
                    break;

                default:
                    result = MESSAGE_PARSE_ERROR;
                    break;
            }
            
            return result;
        }


       virtual motor_controller_status_t getStatus() {
        return _status;
       }

        // State (-> Status got from the Motor Controller)
        virtual motor_controller_state_t getState(){
            return _state;
        }

        // Speed
        virtual float getSpeed(){
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
        virtual float getDcVoltageAge() {
            return _dcVoltageAge;
        }

    private:

        motor_controller_status_t _status = 0;
        motor_controller_state_t _state = 0;
        
        float _speed = 0;
        float _speedAge = 0;
        float _current = 0;
        
        int16_t _motorTemp = 0;
        float _motorTempAge = 0;

        int16_t _servoTemp = 0;
        float _servoTempAge = 0;

        int16_t _airTemp = 0;
        float _airTempAge = 0;

        float _dcVoltage = 0;
        float _dcVoltageAge = 0;

        virtual motor_controller_status_t setStatus(motor_controller_status_t status) {
        _status = status;
       }

        // State (-> Status got from the Motor Controller)
        virtual motor_controller_state_t setState(motor_controller_state_t state){
            _state = state;
        }

        // Speed
        virtual float setSpeed(float speed){
            _speed = speed;
        }
        virtual float setSpeedAge(float speedAge){
            _speedAge = speedAge;
        }

        // Current
        virtual float setCurrent(flat current){
            _current = current;
        }

        // Temperature
        virtual int16_t setMotorTemp(int16_t motorTemp){
            _motorTemp = motorTemp;
        }
        virtual float setMotorTempAge(float motorTempAge){
            _motorTempAge = motorTempAge;
        }
        virtual int16_t setServoTemp(int16_t servoTemp){
            _servoTemp = servoTemp;
        }
        virtual float setServoTempAge(float servoTempAge){
            _servoTempAge = servoTempAge;
        }
        virtual int16_t setAirTemp(int16_t airTemp){
            _airTemp = airTemp;
        }
        virtual float setAirTempAge(float airTempAge){
            _airTempAge = airTempAge;
        }

        // Voltage
        virtual float setDcVoltage(float dcVoltage){
            _dcVoltage = dcVoltage;
        }
        virtual float setDcVoltageAge(float dcVoltageAge) {
            _dcVoltageAge;
        }

};

#endif