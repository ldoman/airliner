#include "../pe_app.h"
#include <math/Matrix6F10.hpp>
#include <math/Matrix6F6.hpp>
#include <math/Vector6F.hpp>
#include <math/Matrix1F6.hpp>


/* required number of samples for sensor to initialize */
#define REQ_GPS_INIT_COUNT  (10)
/* 1.0 s */
#define GPS_TIMEOUT         (1000000)


void PE::gpsInit()
{
    /* check for good gps signal */
    uint8 nSat = m_VehicleGpsPositionMsg.SatellitesUsed;
    float eph = m_VehicleGpsPositionMsg.EpH;
    float epv = m_VehicleGpsPositionMsg.EpV;
    uint8 fix_type = m_VehicleGpsPositionMsg.FixType;

    if (nSat < 6 ||
        eph > m_Params.GPS_EPH_MAX ||
        epv > m_Params.GPS_EPV_MAX ||
        fix_type < 3)
    {
        m_GpsStats.reset();
        return;
    }

    /* measure */
    math::Vector6F y;

    if (gpsMeasure(y) != CFE_SUCCESS) 
    {
        m_GpsStats.reset();
        return;
    }

    /* if finished */
    if (m_GpsStats.getCount() > REQ_GPS_INIT_COUNT)
    {
        /* get mean gps values */
        double gpsLat = m_GpsStats.getMean()[0];
        double gpsLon = m_GpsStats.getMean()[1];
        float gpsAlt = m_GpsStats.getMean()[2];

        m_GpsTimeout = false;
        m_GpsStats.reset();

        if (!m_ReceivedGps)
        {
            /* this is the first time we have received gps */
            m_ReceivedGps = true;

            /* note we subtract X_z which is in down directon so it is 
             * an addition
             **/
            m_GpsAltOrigin = gpsAlt + m_StateVec[X_z];

            /* find lat, lon of current origin by subtracting x and y
             * if not using vision position since vision will
             * have it's own origin, not necessarily where vehicle starts
             **/ 
            if (!m_MapRef.init_done)
            {
                double gpsLatOrigin = 0;
                double gpsLonOrigin = 0;
                /* reproject at current coordinates */
                map_projection_init(&m_MapRef, gpsLat, gpsLon, m_Timestamp);
                /* find origin */
                map_projection_reproject(&m_MapRef, -m_StateVec[X_x], -m_StateVec[X_y], &gpsLatOrigin, &gpsLonOrigin);
                /* reinit origin */
                map_projection_init(&m_MapRef, gpsLatOrigin, gpsLonOrigin, m_Timestamp);

                /* always override alt origin on first GPS to fix
                 * possible baro offset in global altitude at init
                 **/
                m_AltOrigin = m_GpsAltOrigin;
                m_AltOriginInitialized = true;

                (void) CFE_EVS_SendEvent(PE_GPS_OK_INF_EID, CFE_EVS_INFORMATION,
                        "GPS initialized origin. Lat: %6.2f Lon: %6.2f Alt: %5.1f m",
                        gpsLatOrigin, gpsLonOrigin, double(m_GpsAltOrigin));
            }

            (void) CFE_EVS_SendEvent(PE_GPS_OK_INF_EID, CFE_EVS_INFORMATION,
                    "GPS initialized. Lat: %6.2f Lon: %6.2f Alt: %5.1f m",
                    gpsLat, gpsLon, double(gpsAlt));
        }
    }
}


int PE::gpsMeasure(math::Vector6F &y)
{
    /* gps measurement */
    y.Zero();
    y[0] = m_VehicleGpsPositionMsg.Lat * 1e-7;
    y[1] = m_VehicleGpsPositionMsg.Lon * 1e-7;
    y[2] = m_VehicleGpsPositionMsg.Alt * 1e-3;
    y[3] = m_VehicleGpsPositionMsg.Vel_n_m_s;
    y[4] = m_VehicleGpsPositionMsg.Vel_e_m_s;
    y[5] = m_VehicleGpsPositionMsg.Vel_d_m_s;

    /* increament sums for mean */
    m_GpsStats.update(y);
    m_TimeLastGps = m_Timestamp;
    return CFE_SUCCESS;
}


void PE::gpsCorrect()
{
    int i = 0;

    /* measure */
    math::Vector6F y_global;

    if (gpsMeasure(y_global) != CFE_SUCCESS) 
    {
        return; 
    }

    /* gps measurement in local frame */
    double  lat = y_global[0];
    double  lon = y_global[1];
    float  alt = y_global[2];
    float px = 0;
    float py = 0;

    float pz = -(alt - m_GpsAltOrigin);
    map_projection_project(&m_MapRef, lat, lon, &px, &py);
    math::Vector6F y;
    y.Zero();
    y[0] = px;
    y[1] = py;
    y[2] = pz;
    y[3] = y_global[3];
    y[4] = y_global[4];
    y[5] = y_global[5];

    /* gps measurement matrix, measures position and velocity */
    //	Matrix<float, n_y_gps, n_x> C;
    //	C.setZero();
    math::Matrix6F10 C;
    C.Zero();
    C[Y_gps_x][X_x] = 1;
    C[Y_gps_y][X_y] = 1;
    C[Y_gps_z][X_z] = 1;
    C[Y_gps_vx][X_vx] = 1;
    C[Y_gps_vy][X_vy] = 1;
    C[Y_gps_vz][X_vz] = 1;

    /* gps covariance matrix */
    //	SquareMatrix<float, n_y_gps> R;
    //	R.setZero();
    math::Matrix6F6 R;
    R.Zero();
    /* default to parameter, use gps cov if provided */
    //	float var_xy = _gps_xy_stddev.get() * _gps_xy_stddev.get();
    float var_xy = m_Params.GPS_XY_STDDEV * m_Params.GPS_XY_STDDEV;
    //	float var_z = _gps_z_stddev.get() * _gps_z_stddev.get();
    float var_z = m_Params.GPS_Z_STDDEV * m_Params.GPS_Z_STDDEV;
    //	float var_vxy = _gps_vxy_stddev.get() * _gps_vxy_stddev.get();
    float var_vxy = m_Params.GPS_VXY_STDDEV * m_Params.GPS_VXY_STDDEV;
    //	float var_vz = _gps_vz_stddev.get() * _gps_vz_stddev.get();
    float var_vz = m_Params.GPS_VZ_STDDEV * m_Params.GPS_VZ_STDDEV;

    /* if field is not below minimum, set it to the value provided */
    //	if (_sub_gps.get().eph > _gps_xy_stddev.get()) {
    //		var_xy = _sub_gps.get().eph * _sub_gps.get().eph;
    //	}
    if(m_VehicleGpsPositionMsg.EpH > m_Params.GPS_XY_STDDEV)
    {
        var_xy = m_VehicleGpsPositionMsg.EpH * m_VehicleGpsPositionMsg.EpH;
    }
    //	if (_sub_gps.get().epv > _gps_z_stddev.get()) {
    //		var_z = _sub_gps.get().epv * _sub_gps.get().epv;
    //	}
    if (m_VehicleGpsPositionMsg.EpV > m_Params.GPS_Z_STDDEV)
    {
        var_z = m_VehicleGpsPositionMsg.EpV * m_VehicleGpsPositionMsg.EpV;
    }
    //	float gps_s_stddev =  _sub_gps.get().s_variance_m_s;
    float gps_s_stddev = m_VehicleGpsPositionMsg.SVariance;
    //	if (gps_s_stddev > _gps_vxy_stddev.get()) {
    //		var_vxy = gps_s_stddev * gps_s_stddev;
    //	}
    if(gps_s_stddev > m_Params.GPS_VXY_STDDEV)
    {
        var_vxy = gps_s_stddev * gps_s_stddev;
    }
    //	if (gps_s_stddev > _gps_vz_stddev.get()) {
    //		var_vz = gps_s_stddev * gps_s_stddev;
    //	}
    if(gps_s_stddev > m_Params.GPS_VZ_STDDEV)
    {
        var_vz = gps_s_stddev * gps_s_stddev;
    }

    //	R(0, 0) = var_xy;
    //	R(1, 1) = var_xy;
    //	R(2, 2) = var_z;
    //	R(3, 3) = var_vxy;
    //	R(4, 4) = var_vxy;
    //	R(5, 5) = var_vz;
    R[0][0] = var_xy;
    R[1][1] = var_xy;
    R[2][2] = var_z;
    R[3][3] = var_vxy;
    R[4][4] = var_vxy;
    R[5][5] = var_vz;

    /* get delayed x */
    uint8 i_hist = 0;

    //	if (getDelayPeriods(_gps_delay.get(), &i_hist)  < 0) { return; }
    //	Vector<float, n_x> x0 = _xDelay.get(i_hist);
    if(getDelayPeriods(m_Params.GPS_DELAY, &i_hist) < 0)
    {
        return;
    }
    
    math::Matrix10F1 temp;
    temp = m_XDelay.Get(i_hist);
    math::Vector10F x0;

    x0 = temp.ToVector();
    /* residual */
    //	Vector<float, n_y_gps> r = y - C * x0;
    /* 6F - 6F10 * 10x1(10F)*/
    math::Vector6F r;
    r.Zero();
    r = y - C * x0;

    //	for (int i = 0; i < 6; i ++) {
    //		_pub_innov.get().vel_pos_innov[i] = r(i);
    //		_pub_innov.get().vel_pos_innov_var[i] = R(i, i);
    //	}
    for(i = 0; i < 6; i++)
    {
        m_Ekf2InnovationsMsg.VelPosInnov[i] = r[i];
        m_Ekf2InnovationsMsg.VelPosInnovVar[i] = r[i];
    }

    //	Matrix<float, n_y_gps, n_y_gps> S_I = inv<float, 6>(C * _P * C.transpose() + R);
    math::Matrix6F6 S_I;
    S_I.Zero();

    S_I = C * m_StateCov * C.Transpose() + R;
    S_I = S_I.Inversed();

    /* fault detection */
    //	float beta = (r.transpose() * (S_I * r))(0, 0);
    /* 6x6 * 6x1 */
    
    math::Matrix1F6 rTranspose;
    rTranspose[0][0] = r[0];
    rTranspose[0][1] = r[1];
    rTranspose[0][2] = r[2];
    rTranspose[0][3] = r[3];
    rTranspose[0][4] = r[4];
    rTranspose[0][5] = r[5];
    
    float beta = (rTranspose * (S_I * r))[0][0];
    /* artifically increase beta threshhold to prevent fault during 
     * landing
     **/ 
    float beta_thresh = 1e2f;

    if (beta / BETA_TABLE[n_y_gps] > beta_thresh) 
    {
        if (!m_GpsFault)
        {
            (void) CFE_EVS_SendEvent(PE_GPS_FAULT_ERR_EID, CFE_EVS_ERROR,
                    "GPS fault, %3g %3g %3g %3g %3g %3g",
                    double(r[0]*r[0] / S_I[0][0]),  double(r[1]*r[1] / S_I[1][1]), double(r[2]*r[2] / S_I[2][2]),
                    double(r[3]*r[3] / S_I[3][3]),  double(r[4]*r[4] / S_I[4][4]), double(r[5]*r[5] / S_I[5][5]));
            /* TODO move to single fault bool */
            m_GpsFault = true;
        }
    }
    else if (m_GpsFault)
    {
    	m_GpsFault = false;
        (void) CFE_EVS_SendEvent(PE_GPS_OK_INF_EID, CFE_EVS_INFORMATION,
                "GPS OK");

        m_GpsInitialized = true;
    }

    /* kalman filter correction always for GPS */
    //	Matrix<float, n_x, n_y_gps> K = _P * C.transpose() * S_I;
    math::Matrix10F6 K;
    K.Zero();

    /* 10x10 * 6x10' (10x6) * 6x6 */
    K = m_StateCov * C.Transpose() * S_I;
    //	Vector<float, n_x> dx = K * r;
    math::Vector10F dx;
    dx.Zero();
    dx = K * r;

    //	_x += dx;
    /* 10F * 10F */
    m_StateVec = m_StateVec + dx;

    //	_P -= K * C * _P;
    /* 10x10 - (10x6 * 6x10 * 10x10)*/
    m_StateCov = m_StateCov - K * C * m_StateCov;
}


void PE::gpsCheckTimeout()
{
	if (m_Timestamp - m_TimeLastGps > GPS_TIMEOUT)
	{
		if (!m_GpsTimeout)
		{
			m_GpsTimeout = true;
			m_GpsStats.reset();
			(void) CFE_EVS_SendEvent(PE_GPS_TIMEOUT_ERR_EID, CFE_EVS_ERROR,
									 "GPS timeout");
		}
	}
}
