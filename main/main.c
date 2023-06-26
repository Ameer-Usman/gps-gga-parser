#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"

//--------------------------Structs------------------------------------------
typedef struct {

   
} gps_data_t;

typedef struct {
   
} utc_time_t;

typedef struct {
   
} position_t;

typedef struct {
   
} gps_data_t;


//---------------------------function definitions------------------------------
bool validator(char[]);
char[] parse-gps-data(char[], gps_data_t);


void app_main(void){
    char gps[]="$GPGSA,A,3,10,07,05,02,29,04,08,13,,,,,1.72,1.03,1.38*0A";

    /*
    if (validator(gps)){
        printf("Data is valid.");
    }
    else
    printf("Data is not valid;");
    */
}



//----------------------------------------------------------------------------

bool validator(char gps_packet_gga[]) {
    char d[] = "*";
    char temp[strlen(gps_packet_gga) + 1];
    strcpy(temp, gps_packet_gga);
    char *data_packet = strtok(temp, d);
    char *checksum = strtok(NULL, "\0");
    int hexValue;
    sscanf(checksum, "%x", &hexValue);
    char isValid = 0;
    int i = 0;
    if (temp[i] == '$')
        i++;
    while (temp[i] != '*' && temp[i] != '\0') {
        isValid ^= temp[i];
        i++;
    }
    printf("%c, %s\n", isValid, checksum);
    bool status = false;
    if (isValid == (char)hexValue) {
        status = true;
    }
    else {
        return status;
    }
    return status;
}
