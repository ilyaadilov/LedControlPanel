#ifndef __PROJECT_CONFIG_H
#define __PROJECT_CONFIG_H

// Global settings for project

#define PRODUCTION                 // Uncomment at production




#ifdef PRODUCTION

#else

#define USE_USART_LOGGING                 // Logging with usart_printf

#endif




#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif


#endif /* __PROJECT_CONFIG_H */
