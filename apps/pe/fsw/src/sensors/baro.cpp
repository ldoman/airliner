#include "../pe_app.h"
#include <math/Matrix1F10.hpp>
#include <math/Matrix1F1.hpp>

/* required number of samples for sensor to initialize */
#define REQ_BARO_INIT_COUNT   (100)
/* 0.1 s */
#define BARO_TIMEOUT          (100000)

void PE::baroInit()
{
    /* measure */
	math::Vector1F y;

	if (baroMeasure(y) != CFE_SUCCESS)
	{
		m_BaroStats.reset();
		return;
	}

	// if finished
	if (m_BaroStats.getCount() > REQ_BARO_INIT_COUNT)
	{
		m_BaroAltOrigin = m_BaroStats.getMean()[0];

		(void) CFE_EVS_SendEvent(PE_BARO_OK_INF_EID, CFE_EVS_INFORMATION,
								 "Baro initialized. Mean: (%d) Std dev: (%d) cm",
								 (int)m_BaroStats.getMean()[0],
								 (int)(100 * m_BaroStats.getStdDev()[0]));

		m_BaroTimeout = false;

		if (!m_AltOriginInitialized)
		{
			m_AltOriginInitialized = true;
			m_AltOrigin = m_BaroAltOrigin;
		}
	}
}


int32 PE::baroMeasure(math::Vector1F &y)
{
	//measure
	y.Zero();
	y[0] = m_SensorCombinedMsg.BaroAlt;
	m_BaroStats.update(y);
	//m_TimeLastBaro = m_Timestamp;
	return CFE_SUCCESS;
}


void PE::baroCorrect()
{
    /* measure */
    math::Vector1F y;
    y.Zero();

    if (baroMeasure(y) != CFE_SUCCESS)
    {
        return;
    }

    /* subtract baro origin alt */
    y[0] -= m_BaroAltOrigin;

    /* baro measurement matrix */
    math::Matrix1F10 C;
    C.Zero();
    /* measured altitude, negative down dir */
    C[Y_baro_z][X_z] = -1.0f;

    math::Matrix1F1 R;
    R.Zero();
    R[0][0] = m_Params.BARO_STDDEV * m_Params.BARO_STDDEV;

    /* residual */
    math::Matrix1F1 S_I;
    S_I.Zero();

//	Matrix<float, n_y_baro, n_y_baro> S_I =
//		inv<float, n_y_baro>((C * _P * C.transpose()) + R);

    /* ((1x10 * 10x10) * 10x1) + 1x1) */
    S_I = C * m_StateCov * C.Transpose() + R;
    
    /* Take the inverse of a 1x1 matrix (reciprical of the single entry) */
    S_I[0][0] = 1 / S_I[0][0];

//	Vector<float, n_y_baro> r = y - (C * _x);
    math::Vector1F r;
    r.Zero();
    /* Vector1F -  (1x10 * Vector10F) */
    r = y - (C * m_StateVec);

//	// fault detection
//	float beta = (r.transpose() * (S_I * r))(0, 0);
    /* fault detection 1F * 1x1 * 1F */
    float beta = r[0] * S_I[0][0] * r[0];

    if (beta > BETA_TABLE[n_y_baro])
    {
        if (!m_BaroFault)
        {
            (void) CFE_EVS_SendEvent(PE_BARO_FAULT_ERR_EID, CFE_EVS_ERROR,
                    "Baro fault, r %5.2f m, beta %5.2f", r[0], beta);
            m_BaroFault = true;
        }

    }
    else if (m_BaroFault)
    {
    	m_BaroFault = false;
        (void) CFE_EVS_SendEvent(PE_BARO_OK_INF_EID, CFE_EVS_INFORMATION,
                "Baro OK");

        m_BaroInitialized = true;
    }

// kalman filter correction always
//	Matrix<float, n_x, n_y_baro> K = _P * C.transpose() * S_I;
//	Vector<float, n_x> dx = K * r;
//	_x += dx;
//	_P -= K * C * _P;
    math::Matrix10F1 K;
    K.Zero();
    /* 10x10 * 10x1 * 1x1 */
    K = m_StateCov * C.Transpose() * S_I;

    /* 10x1 * 1x1 */
    //dx = K * r;
    math::Matrix10F1 temp;
    temp.Zero();
    temp = K * r;

    math::Vector10F dx;
    dx.Zero();
    dx = temp.ToVector();

    /* 10F + 10F*/
    m_StateVec = m_StateVec + dx;
    /* 10x10 - 10x1 * 1x10 * 10x10 */
    m_StateCov = m_StateCov - K * C * m_StateCov;
}


void PE::baroCheckTimeout()
{
    uint64 Timestamp = 0;

	if (m_Timestamp > m_TimeLastBaro)
	{
        Timestamp = m_Timestamp - m_TimeLastBaro;
    }
	else if (m_Timestamp < m_TimeLastBaro)
	{
        Timestamp = m_TimeLastBaro - m_Timestamp;
    }

	if (Timestamp > BARO_TIMEOUT)
	{
		if (!m_BaroTimeout)
		{
			m_BaroTimeout = true;
			m_BaroStats.reset();
			(void) CFE_EVS_SendEvent(PE_BARO_TIMEOUT_ERR_EID, CFE_EVS_ERROR,
									 "Baro timeout: %llu us", Timestamp);
		}
	}
}
