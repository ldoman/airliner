#include "../pe_app.h"

// required number of samples for sensor
// to initialize
//
#define 	REQ_LAND_INIT_COUNT (1)
#define 	LAND_TIMEOUT   		(1000000) // 1.0 s

void PE::landInit()
{
	// measure
	math::Vector3F y;
	y.Zero();

	if (landMeasure(y) != CFE_SUCCESS)
	{
		m_LandCount = 0;
	}

	// if finished
	if (m_LandCount > REQ_LAND_INIT_COUNT)
	{
		m_LandTimeout = false;

		(void) CFE_EVS_SendEvent(PE_LAND_OK_INF_EID, CFE_EVS_INFORMATION,
								 "Land detector initialized");
	}
}

int PE::landMeasure(math::Vector3F &y)
{
	m_TimeLastLand = m_Timestamp;
	y.Zero();
	m_LandCount += 1;
	return CFE_SUCCESS;
}

void PE::landCorrect()
{
	// measure land
	math::Vector3F y;
	y.Zero();

	if (landMeasure(y) != CFE_SUCCESS)
	{
		return;
	}

	// measurement matrix
	math::Matrix3F10 C;
	C.Zero();
	// y = -(z - tz)
	C[Y_land_vx][X_vx] = 1.0f;
	C[Y_land_vy][X_vy] = 1.0f;
	C[Y_land_agl][X_z] = -1.0f; // measured altitude, negative down dir.
	C[Y_land_agl][X_tz] = 1.0f; // measured altitude, negative down dir.

	// use parameter covariance
	math::Matrix3F3 R;
	R.Zero();
	R[Y_land_vx][Y_land_vx] = m_Params.LAND_VXY_STDDEV * m_Params.LAND_VXY_STDDEV;
	R[Y_land_vy][Y_land_vy] = m_Params.LAND_VXY_STDDEV * m_Params.LAND_VXY_STDDEV;
	R[Y_land_agl][Y_land_agl] = m_Params.LAND_Z_STDDEV * m_Params.LAND_Z_STDDEV;

	// residual
	math::Matrix3F3 S_I;
	S_I.Zero();
	S_I = (C * (m_StateCov * C.Transpose())) + R;
	S_I = S_I.Inversed();

	math::Vector3F r;
	r.Zero();
	r = y - C * m_StateVec;

	m_Ekf2InnovationsMsg.HaglInnov = r[Y_land_agl];
	m_Ekf2InnovationsMsg.HaglInnovVar = R[Y_land_agl][Y_land_agl];

	// fault detection
	float beta = (r.Transpose() * (S_I * r));

	// artifically increase beta threshhold to prevent fault during landing
	float beta_thresh = 1e2f;

	if (beta / BETA_TABLE[n_y_land] > beta_thresh)
	{
		if (!m_LandFault)
		{
			m_LandFault = true;
			(void) CFE_EVS_SendEvent(PE_LAND_FAULT_ERR_EID, CFE_EVS_ERROR,
									 "Land detector fault, beta %5.2f", double(beta));
		}

		// abort correction
		return;
	}
	else if (m_LandFault)
	{
		m_LandFault = false;
		(void) CFE_EVS_SendEvent(PE_LAND_OK_INF_EID, CFE_EVS_ERROR,
								 "Land detector OK");
		m_LandInitialized = true;
	}

	// kalman filter correction always for land detector
	math::Matrix10F3 K;
	K.Zero();
	K = m_StateCov * C.Transpose() * S_I;

	math::Vector10F dx;
	dx.Zero();
	dx = K * r;

	m_StateVec += dx;
	m_StateCov -= K * C * m_StateCov;
}

void PE::landCheckTimeout()
{
	if (m_Timestamp - m_TimeLastLand > LAND_TIMEOUT)
	{
		if (!m_LandTimeout)
		{
			m_LandTimeout = true;
			m_LandCount = 0;
			(void) CFE_EVS_SendEvent(PE_LAND_TIMEOUT_ERR_EID, CFE_EVS_ERROR,
									 "Land detector timeout");
		}
	}
}

