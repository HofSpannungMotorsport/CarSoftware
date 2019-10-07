#ifndef DUMMY_S_CAR_H
#define DUMMY_S_CAR_H

#include "carpi.h"

class DummySCar : public SCar {
    public:
        DummySCar(Sync &syncer) : SCar(syncer) {
            _state = ALMOST_READY_TO_DRIVE;
        }

        void setState(car_state_t state) {
            _state = state;
        }
};

#endif // DUMMY_S_CAR_H