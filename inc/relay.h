#ifndef __RELAY_H__
#define __RELAY_H__

#define BUTTON_PIN GPIO_0
#define RELAY_PIN GPIO_7

void RelaySwitchTask(void *pvParameters);

#endif /* __RELAY_H__ */
