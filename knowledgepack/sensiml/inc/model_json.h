#ifndef __MODEL_JSON_H__
#define __MODEL_JSON_H__

const char recognition_model_string_json[] = {"{\"NumModels\":1,\"ModelIndexes\":{\"0\":\"Pipeline_1_rank_4\"},\"ModelDescriptions\":[{\"Name\":\"Pipeline_1_rank_4\",\"ClassMaps\":{\"1\":\"FAN\",\"2\":\"LAMP\",\"3\":\"LAMP_FAN\",\"4\":\"NO_LOAD\",\"0\":\"Unknown\"},\"ModelType\":\"DecisionTreeEnsemble\"}]}"};

int recognition_model_string_json_len = sizeof(recognition_model_string_json);

#endif /* __MODEL_JSON_H__ */