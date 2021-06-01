//SensiML Includes
#include "kb.h"
#include "sml_output.h"
#include "sml_recognition_run.h"
//#include "dcl_commands.h"
#include "sensor_ssss.h"

#define MODEL_INDEX KB_MODEL_Pipeline_1_rank_4_INDEX

int sml_recognition_run_batch(signed short *data_batch, int batch_sz, uint8_t num_sensors, uint32_t sensor_id)
{
	int ret;
	int batch_index = 0;
	signed short* data;

	for(batch_index=0; batch_index < batch_sz; batch_index++)
	{
	    data = &data_batch[batch_index*num_sensors];
	    switch(sensor_id)
	    {
	    	case SENSOR_SSSS_ID:
                    //dbg_str_int("MODEL_INDEX", MODEL_INDEX);
	    	    //dbg_str_int("data_0", data[0]);
	    	    //dbg_str_int("data_1", data[1]);
	    	    //dbg_str_int("data_2", data[2]);
	    	    //dbg_str_int("data_3", data[3]);
	    	    ret = kb_run_model((SENSOR_DATA_T *)data, num_sensors, MODEL_INDEX);

	    	    if (ret >= 0){
                        dbg_str_int("ret", ret);
	    		sml_output_results(MODEL_INDEX, ret);
	    		kb_reset_model(MODEL_INDEX);
	    	    };
	    	    break;
	    	default:
	    	    break;
	    }
	}
	return ret;
}

int sml_recognition_run_single(signed short *data, uint32_t sensor_id)
{
	int ret;
	uint8_t num_sensors = 0;

	switch(sensor_id)
	{
	    case SENSOR_SSSS_ID:
		ret = kb_run_model((SENSOR_DATA_T *)data, num_sensors, MODEL_INDEX);
		if (ret >= 0){
    		    sml_output_results(MODEL_INDEX, ret);
		    kb_reset_model(MODEL_INDEX);
		};
		break;
	    default:
		break;
	}
	return ret;
}
