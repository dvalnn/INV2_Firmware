#ifndef _KALMAN_H_
#define _KALMAN_H_

#include <ArduinoEigen.h>
#include "quaternion.h"
#include "HardwareCfg.h"

using namespace Eigen;


///_______________________________GENERIC KALMAN______________________________________________________

class ekf{
    public: 
        unsigned long last_update, last_predict,delta_update,delta_predict;
        MatrixXf A,P,H,Q,R,K,B,U,X,Z,X_off;//Z and X must be Line vectors
        MatrixXf last_X,last_Z, R_mean, Q_mean, Z_rem, X_rem; 
        int cur_X = 0, cur_Z = 0;

        /// @brief update step of kalman filter
        void update();

        /// @brief predict step of kalman filter
        void predict();

        /// @brief updates measurments covariance matrix
        void R_update();

        /// @brief updates process covariance matrix
        void Q_update();

        /// @brief Kalman function to be called
        /// @param new_Z new measurments
        /// @param new_U new inputs
        /// @return state
        MatrixXf tick(MatrixXf new_Z, MatrixXf new_U);

};


//________________________ALTITUDE_KALMAN____________________________\\

/// @brief vertical, acceleration, velosity and position Kalman filter @paragraph   
/// x[0]->altitude  
/// x[1]->velocity  
/// x[2]->acceleration  
class alt_kalman: public ekf{

    public:

        MyQuaternion Quat;
        
        /// @brief Sets A matrix-> necessary cicle call because A is state dependent 
        void A_update();

        /// @brief Sets B matrix-> necessary cicle call because B is state dependent 
        void B_update();

        /// @brief Sets H matrix-> necessary cicle call because H is state dependent 
        void H_update();

        /// @brief Inicialization of all matrices
        void begin();

        void update_offset(float alt_off, float vel_off, float acc_off);

        /// @brief Kalman function to be called
        /// @param new_Z new measurments
        /// @param new_U new inputs
        /// @return state
        MatrixXf cicle(MyQuaternion new_Quat, MatrixXf new_Z, MatrixXf new_U);
};

#endif