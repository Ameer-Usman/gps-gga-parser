#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * @brief nmea_gga_validator function validates and check the input NMEA sentence by checking its integrity.
 *
 * @param The NMEA sentence is given to the function as the parameter.
 * @return The return type is bool which indicates whether the sentence is valid or not (true or false)
 */
bool nmea_gga_validator(char NMEA_SENTENCE[])
{
    bool s_status = false;
    char temp[strlen(NMEA_SENTENCE) + 1];
    char temp1[strlen(NMEA_SENTENCE) + 1];
    static char s_isValid = 0;
    static int s_i = 0;
    static int s_hexValue;
    //Making a copy of the NMEA sentence to validate the data packet i.e., GGA
    strcpy(temp, NMEA_SENTENCE);
    //Using the copy to store the string before the delimitor ',' into pointer to a charachter data_packet
    char *data_packet = strtok(temp, ",");
    //Validation of the sentence format by comparing the string pointed by data_packet with "$GPGGA"
    if (!(strcmp(data_packet, "$GPGGA"))) {
        printf("INFO: GGA packet confirmed.\n");
        goto cont;
    }
    else {
        printf("ERROR: Data packet is not GGA!\n");
        return s_status;
    }
    cont:
    //Making a copy of the NMEA sentence to validate the integrity of the gps data
    strcpy(temp1, NMEA_SENTENCE);
    //Data splitting using "*" as a delimitor to seperate the checksum value
    char *data_packet1 = strtok(temp1, "*");
    //storing checksum value at a location pointed by pointer to charachter checksum
    char *checksum = strtok(NULL, "\0");
    //converting the checksum string into hexadecimal value for comparison purpose
    sscanf(checksum, "%x", &s_hexValue);
    //Ignoring the '$' charachter because it is not the part of the data integrity check
    if (temp1[s_i] == '$') s_i++;
    //Calculating the bitwise XOR calculation algorithm from the NMEA-0183 documentation
    while (temp1[s_i] != '*' && temp1[s_i] != '\0') {
        //XOR the next charachter with the XOR result of all the previous charachters
        s_isValid ^= temp1[s_i];
        //increment the index
        s_i++;
    }
    //Validity is checked by comparing the charachter represented by the accumulated XOR result 's_isValid' with
    //the casted charachter representation of 's_hexValue'
    if (s_isValid == (char) s_hexValue) {
        s_status = true;
    }
    else {
        return s_status;
    }
    return s_status;
}

//A test string is used to check whether the function gga_validator is working properly.

void app_main(void)
{
    char NMEA_SENTENCE[] = "$GPGGA,092750.000,5321.6802,N,00630.3372,W,1,8,1.03,61.7,M,55.2,M,,*76";
    if (nmea_gga_validator(NMEA_SENTENCE)) {
        printf("INFO: Data is valid.\n");
    }
    else {
        printf("ERROR: Data is not valid.\n");
    }
}