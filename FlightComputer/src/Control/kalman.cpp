#include <kalman.h>

///_______________________________GENERIC KALMAN______________________________________________________

//#define KALMAN_DEBBUG

void ekf::update(){
    Eigen::MatrixXf IS;

    #ifdef KALMAN_UPDATE_DEBBUG
        #define SIZES
        #ifdef SIZES
            Serial.printf("Sizes\n X: %d | %d H: %d | %d Z: %d | %d R: %d | %d P: %d | %d\n",X.rows(),X.cols(),H.rows(),H.cols(),Z.rows(),Z.cols(),R.rows(),R.cols(),P.rows(),P.cols());
        #endif
        Serial.println("Starting IS update");
    #endif

    IS= H * P * H.transpose() + R;

    #ifdef KALMAN_UPDATE_DEBBUG
        Serial.println("IS done");
        Serial.println("Starting K update");
    #endif

    K= P *  H.transpose() * IS.inverse();

    #ifdef KALMAN_UPDATE_DEBBUG
        Serial.println("K done");
        Serial.printf("K size: %d | %d\n",K.rows(),K.cols());
        Serial.println("Starting X update");
    #endif

    X = X + K * (Z - H * X);
    
    #ifdef KALMAN_UPDATE_DEBBUG
        Serial.println("X done");
        Serial.println("Starting P update");
    #endif

    P=(MatrixXf::Identity(P.rows(),P.cols())-K * H)*P;

    #ifdef KALMAN_UPDATE_DEBBUG
        Serial.println("P done");
    #endif

}

void ekf::predict(){
    #ifdef KALMAN_PREDICT_DEBBUG
        #define SIZES
        #ifdef SIZES
            Serial.printf("Sizes\n X: %d | %d U: %d | %d A: %d | %d B: %d | %d P: %d | %d Q: %d | %d\n",(A*X).rows(),(A*X).cols(),(B*U).rows(),(B*U).cols(),A.rows(),A.cols(),B.rows(),B.cols(),P.rows(),P.cols(),Q.rows(),Q.cols());
        #endif
        Serial.println("Starting X predict");
    #endif
    X = A * X + B * U;
    #ifdef KALMAN_PREDICT_DEBBUG
        Serial.println("X done");
        Serial.println("Starting P predict");
    #endif
    P = A * (P * A.transpose()) + Q;
}

void ekf::R_update(){
    
    
    //R_mean=last_Z.rowwise().sum()/last_Z.cols();

    R_mean = R_mean - Z_rem + Z;

    for(int i=0; i<Z.size();i++)
        R(i,i) = R(i,i) + ((Z(i)-R_mean(i)*R_mean(i)))/((float)last_Z.rows());

}

void ekf::Q_update(){
    
    //Q_mean=last_X.rowwise().sum()/last_X.cols();

    Q_mean = Q_mean - X_rem + X;

    for(int i=0; i<X.size();i++)
        Q(i,i) = Q(i,i) + ((X(i)-Q_mean(i)*Q_mean(i)))/((float)last_X.rows());
        
}

MatrixXf ekf::tick(MatrixXf new_Z, MatrixXf new_U){
    if(new_Z.rows()!=Z.rows()||new_Z.cols()!=Z.cols()){
        Serial.printf("Kalman error - Z has worng size %d / %d instead of %d / %d\n",new_Z.rows(),new_Z.cols(),Z.rows(),Z.cols());
        return X;
    }

    if(new_U.rows()!=U.rows()||new_U.cols()!=U.cols()){
        Serial.printf("Kalman error - U has worng size%d /%d \n",new_U.rows(),new_U.cols());
        return X;
    }

    #ifdef KALMAN_DEBBUG
        Serial.println("Right Sizes");
    #endif

    U = new_U;

    delta_predict = millis()-last_predict;

    #ifdef KALMAN_DEBBUG
        Serial.println("Starting predict");
    #endif

    predict();
    
    #ifdef KALMAN_DEBBUG
        Serial.println("Predict Done");
    #endif

    last_predict = millis();

    #ifdef KALMAN_DEBBUG
        Serial.println("Z check");
        Serial.printf(" [%f,%f,%f,%f,%f,%f,%f]\n",new_Z(all,all));
        Serial.printf(" [%f,%f,%f,%f,%f,%f,%f]\n",Z(all,all));
    #endif

    if(Z != new_Z){

        #ifdef KALMAN_DEBBUG
            Serial.println("Starting last_Z update");
            Serial.printf(" [%f,%f,%f,%f,%f,%f,%f]\n",new_Z(all,all));
            Serial.printf(" [%f,%f,%f,%f,%f,%f,%f\n%f,%f,%f,%f,%f,%f,%f\n%f,%f,%f,%f,%f,%f,%f]\n",last_Z(all,all));
        #endif
        Z_rem = last_Z(all,cur_Z);
        last_Z(all,cur_Z)=new_Z;
        if(cur_Z==MAX_STORAGE-1)
            cur_Z=0;
        else
            ++cur_Z;

        /* for(int i=MAX_STORAGE-1; i>1;i--){
            #ifdef KALMAN_DEBBUG
                //#define LAST_Z_DEBBUG
                #ifdef LAST_Z_DEBBUG
                    Serial.print(i);
                    Serial.printf(": %f | %f | %f | %f | %f | %f | %f\n",last_Z(all,i-1));
                #endif
            #endif
            last_Z(all,i-1)=last_Z(all,i);
        }

        last_Z(all,0) =  new_Z; */
        Z = new_Z; 
        
        #ifdef KALMAN_DEBBUG
            Serial.println("Z updated");
            Serial.printf(" [%f,%f,%f,%f,%f,%f,%f\n%f,%f,%f,%f,%f,%f,%f\n%f,%f,%f,%f,%f,%f,%f]\n",last_Z(all,all));
        #endif

        delta_update = millis()-last_update;

        #ifdef KALMAN_DEBBUG
            Serial.println("Starting R");
        #endif

        R_update();

        #ifdef KALMAN_DEBBUG
            Serial.println("R done");
            Serial.println("Starting update");
        #endif

        update();

        #ifdef KALMAN_DEBBUG
            Serial.println("Update done");
        #endif

        last_update = millis();
    }

    /* for(int i=MAX_STORAGE-1; i>1;i--){
            #ifdef KALMAN_DEBBUG
                //#define LAST_X_DEBBUG
                #ifdef LAST_X_DEBBUG
                    Serial.print(i);
                    Serial.printf(": %f | %f | %f | %f | %f | %f | %f\n",last_X(all,i-1));
                #endif
            #endif
            last_X(all,i-1)=last_X(all,i);
        }

        last_X(all,0) =  X; */
    X_rem = last_X(all,cur_X);
    last_X(all,cur_X)=X;
    if(cur_X==MAX_STORAGE-1)
        cur_X=0;
    else
        ++cur_X;

    #ifdef KALMAN_DEBBUG        
        Serial.println("Starting Q");
    #endif

    Q_update();

    #ifdef KALMAN_DEBBUG
        Serial.println("Q done");
    #endif

    return X;

}


///________________________________ORIENTATION KALMAN_____________________________________________

void orientation::begin(){
    last_X = MatrixXf::Zero(7,MAX_STORAGE);
    last_Z = MatrixXf::Zero(7,MAX_STORAGE);
    P = MatrixXf::Identity(7,7);
    H = MatrixXf::Identity(7,7);
    Q = MatrixXf::Identity(7,7);
    R = MatrixXf::Identity(7,7);
    A = MatrixXf::Identity(7,7);
    K = MatrixXf::Zero(7,7);
    B = MatrixXf::Zero(7,3);
    Z = MatrixXf::Zero(7,1);
    U = MatrixXf::Zero(3,1);
    X = MatrixXf::Zero(7,1);
    R_mean = MatrixXf::Zero(7,1);
    Q_mean = MatrixXf::Zero(7,1);
}

void orientation::A_update(){

    float t = (float)delta_predict/2000.0;

    A <<      1, -U(0)*t, -U(1)*t, -U(2)*t, 0, 0, 0,
         U(0)*t,       1,  U(2)*t, -U(1)*t, 0, 0, 0,
         U(1)*t, -U(2)*t,       1,  U(0)*t, 0, 0, 0,
         U(2)*t,  U(1)*t, -U(0)*t,       1, 0, 0, 0,
              0,       0,       0,       0, 1, 0, 0,
              0,       0,       0,       0, 0, 1, 0,
              0,       0,       0,       0, 0, 0, 1;


}

void orientation::B_update(){
    float t = (float)delta_predict/1000.0;
    #ifdef KALMAN_B_DEBBUG
        Serial.print("t: ");
        Serial.print(t);
        Serial.print(" |X(0): ");
        Serial.print(X(0));
        Serial.print(" |X(1): ");
        Serial.print(X(1));
        Serial.print(" |X(2): ");
        Serial.print(X(2));
        Serial.print(" |X(3): ");
        Serial.println(X(3));
    #endif
    B <<  X(1),  X(2),  X(3),  
         -X(0),  X(3), -X(2),
         -X(3), -X(0),  X(1), 
          X(2), -X(1), -X(0),
             t,     0,     0,
             0,     t,     0,
             0,     0,     t;


}

MatrixXf orientation::cicle(MatrixXf new_Z, MatrixXf new_U){
    
    #ifdef KALMAN_DEBBUG
        Serial.println("Starting A");
    #endif

    A_update();

    #ifdef KALMAN_DEBBUG
        Serial.println("A done");
        Serial.println("Starting B");
    #endif

    B_update();

    #ifdef KALMAN_DEBBUG
        Serial.println("B done");
    #endif

    return tick(new_Z, new_U);
}



///________________________________ALTITUDE KALMAN_____________________________________________

void alt_kalman::begin(){
    last_X = MatrixXf::Zero(9,MAX_STORAGE);
    last_Z = MatrixXf::Zero(9,MAX_STORAGE);
    P = MatrixXf::Identity(9,9);
    H = MatrixXf::Zero(9,9);
    Q = MatrixXf::Identity(9,9);
    R = MatrixXf::Identity(9,9);
    A = MatrixXf::Identity(9,9);
    K = MatrixXf::Zero(9,9);
    B = MatrixXf::Zero(9,9);
    Z = MatrixXf::Zero(9,1);
    U = MatrixXf::Zero(9,1);
    X = MatrixXf::Zero(9,1);
    R_mean = MatrixXf::Zero(9,1);
    Q_mean = MatrixXf::Zero(9,1);
}

void alt_kalman::A_update(){

    float t = (float)delta_predict/2000.0;

    A << 1,t,0.5*t*t,0,    0,      0,0,0,      0,
         0,1,      t,0,    0,      0,0,0,      0,
         0,0,      0,0,    0,      0,0,0,      0,
         0,0,      0,1,    t,0.5*t*t,0,0,      0,
         0,0,      0,0,    1,      t,0,0,      0,
         0,0,      0,0,    0,      0,0,0,      0,
         0,0,      0,0,    0,      0,0,t,0.5*t*t,
         0,0,      0,0,    0,      0,0,1,      t,
         0,0,      0,0,    0,      0,0,0,      0;

}

void alt_kalman::H_update(){
    
    float t = (float)delta_update/1000.0;

    H << 1,t,                                 0.5*t*t,0,    0,                                      0,0,0,                                      0,
         0,1,                                       t,0,    0,                                      0,0,0,                                      0,
         0,0, 1 - 2*(Quat.qy*Quat.qy+Quat.qz*Quat.qz),0,    0,  2*(Quat.qx*Quat.qy + Quat.qz*Quat.qw),0,0,  2*(Quat.qx*Quat.qz - Quat.qy*Quat.qw),
         0,0,                                       0,1,    t,                                0.5*t*t,0,0,                                      0,
         0,0,                                       0,0,    1,                                      t,0,0,                                      0,
         0,0,   2*(Quat.qx*Quat.qy - Quat.qz*Quat.qw),0,    0,1 - 2*(Quat.qx*Quat.qx+Quat.qz*Quat.qz),0,0,  2*(Quat.qy*Quat.qz + Quat.qx*Quat.qw),
         0,0,                                       0,0,    0,                                      0,1,t,                                0.5*t*t,
         0,0,                                       0,0,    0,                                      0,0,1,                                      t,
         0,0,   2*(Quat.qx*Quat.qz + Quat.qy*Quat.qw),0,    0,  2*(Quat.qy*Quat.qz - Quat.qx*Quat.qw),0,0,1 - 2*(Quat.qx*Quat.qx+Quat.qy*Quat.qy);


}

void alt_kalman::B_update(){
    
    float t = (float)delta_predict/1000.0;

    B << 0,0,                                       0,0,    0,                                      0,0,0,                                      0,
         0,0,                                       0,0,    0,                                      0,0,0,                                      0,
         0,0, 1 - 2*(Quat.qy*Quat.qy+Quat.qz*Quat.qz),0,    0,  2*(Quat.qx*Quat.qy - Quat.qz*Quat.qw),0,0,  2*(Quat.qx*Quat.qz + Quat.qy*Quat.qw),
         0,0,                                       0,0,    0,                                      0,0,0,                                      0,
         0,0,                                       0,0,    0,                                      0,0,0,                                      0,
         0,0,   2*(Quat.qx*Quat.qy + Quat.qz*Quat.qw),0,    0,1 - 2*(Quat.qx*Quat.qx+Quat.qz*Quat.qz),0,0,  2*(Quat.qy*Quat.qz - Quat.qx*Quat.qw),
         0,0,                                       0,0,    0,                                      0,0,0,                                      0,
         0,0,                                       0,0,    0,                                      0,0,0,                                      0,
         0,0,   2*(Quat.qx*Quat.qz - Quat.qy*Quat.qw),0,    0,  2*(Quat.qy*Quat.qz + Quat.qx*Quat.qw),0,0,1 - 2*(Quat.qx*Quat.qx+Quat.qy*Quat.qy);


}
MatrixXf alt_kalman::cicle(MyQuaternion new_Quat, MatrixXf new_Z, MatrixXf new_U){
    
    #ifdef KALMAN_DEBBUG
        Serial.println("Starting A");
    #endif
    Quat = new_Quat;
    A_update();
    B_update();
    #ifdef KALMAN_DEBBUG
        Serial.println("A done");
        Serial.println("Starting H");
    #endif

    H_update();

    #ifdef KALMAN_DEBBUG
        Serial.println("H done");
    #endif

    return tick(new_Z, new_U);
}



