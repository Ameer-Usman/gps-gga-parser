#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"

/**
 * @brief GPS time (UTC)
 */
typedef struct {
    int hour;
    int minutes;
    double seconds;
} gpsData_Time_t;

/**
 * @brief GPS position i.e., latitude, longitude, Altitude
 */
typedef struct {
    int latDeg;
    float latMin;
    char latInd[2];
    int longDeg;
    float longMin;
    char long_Ind[2];
    float altitude;
    char altitude_U[2];
} gpsData_Position_t;

/**
 * @brief GPS parsed data
 */
typedef struct {
    gpsData_Time_t time;
    gpsData_Position_t position;
    int gpsData_satInView;
    int gpsData_qIndicator;
    float gpsData_geoSep;
    char gpsData_geoSep_U[2];
    float gpsData_hdop;
    float gpsData_dgps;
    int gpsData_drsID;
} nmea_Parsed_t;

/**
 * @brief nmea_gga_validator function validates and check the input NMEA sentence by checking its integrity.
 * @param The NMEA sentence is given to the function as the parameter through a pointer.
 * @return The return type is bool which indicates whether the sentence is valid or not (true or false)
 */
bool nmea_gga_validator(char *SENTENCE)
{
    if (SENTENCE[0] == '\0') {
        return false;
    }
    else {
        bool s_status = false;
        char temp[strlen(SENTENCE) + 1];
        char temp1[strlen(SENTENCE) + 1];
        static char s_isValid = 0;
        static int s_i = 0;
        static int s_hexValue;
        //Making a copy of the NMEA sentence to validate the data packet i.e., GGA
        strcpy(temp, SENTENCE);
        //Using the copy to store the string before the delimitor ',' into pointer to a charachter data_packet
        char *data_packet = strtok(temp, ",");
        //Validation of the sentence format by comparing the string pointed by data_packet with "$GPGGA"
        if (!(strcmp(data_packet, "$GPGGA"))) {
            printf("INFO: GGA packet confirmed.\n");
        }
        else {
            printf("ERROR: Data packet is not GGA!\n");
            return s_status;
        }
        //Making a copy of the NMEA sentence to validate the integrity of the gps data
        strcpy(temp1, SENTENCE);
        //Data splitting using "*" as a delimitor to seperate the checksum value
        strtok(temp1, "*");
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
}

/**
 * @brief emptyFieldsHandler function introduces a special charachter (@) wherever there is an empty string.
 * @param The NMEA sentence is given to the function as the parameter through a pointer.
 * @return The return type is pointer to a charachter which gives the modified string with (@) in place of empty fields.
 */
char* emptyFieldsHandler(char* SENTENCE2)
{    
    char *temp = malloc(strlen(SENTENCE2) + 15 + 1); // Allocate memory for the modified string
    
    if (temp == NULL) {
        printf("Memory NOT allocated!\n");
        while(1);
    }
    
    strcpy(temp, SENTENCE2);
    int j = 0; // Index for the modified string
    //Here in this loop a modified string is being created by inserting a special charachter to indicate empty strings.
    for (int i = 0; i < strlen(SENTENCE2); i++) {
        temp[j++] = SENTENCE2[i]; // Copy current character to temp
        
        if (SENTENCE2[i] == ',' && (SENTENCE2[i+1] == ',' || SENTENCE2[i+1] == '*')) {
            temp[j++] = '@'; // Insert '@' character
        }
    }
    temp[j] = '\0'; // Null charachter for end of the modified string
    return temp;    // returning the pointer to the modified string, the memory allocated to this pointer will be freed in the calling function
}

/**
 * @brief Parse_gps_data function parses the validated string into readable information such as longitude, latitude, time etc.
 * @param NMEA_SENTENCE is given to the function as the input parameter.
 * @return The return type of the function is nmea_Parsed_t type which will provide the parsed data.
 */

//As for now the return type is void, it will be changed to proper return type later.
void Parse_gps_data(char *SENTENCE1)
{
    //Validate the GGA string.
    if (nmea_gga_validator(SENTENCE1) == false) {
        printf("ERROR: Data is not valid!");
        return;
    }
    //Handle empty fields.
    char *data = emptyFieldsHandler(SENTENCE1);
    char temp[strlen(data) + 1];
    char temp1[15];
    //Making a copy of the NMEA sentence for parsing
    strcpy(temp, data);
    //Free the allocated memory as it is no longer needed.
    free(data);
    //Data splitting using "*" as a delimitor to seperate the checksum value
    strcpy(temp, strtok(temp, "*"));
    //Eliminating the packet format indicator '$GPGGA' from the string to parse the data
    strtok(temp, ",");
    //String (temp) with the unparsed GPS-data
    strcpy(temp, strtok(NULL, "\0"));
    //UTC-time raw string Parsing
    strcpy(temp1, strtok(temp, ","));
    static int s_temp = 0;
    if (temp1[0] == '@'){
        //If the string is empty, the empty warning will be generated here.
        printf("WARNING: UTC-Time field is empty!\n");
        strcpy(temp1, strtok(NULL, ","));
    }
    else {
        //Loop to make sure the formatting is correct.
        for (int s_i = 0; s_i <= 9; s_i++ ) {
            //Making sure only the allowed charchters are present in the string.
            if ((temp1[s_i] >= '0' && temp1[s_i] <= '9') || temp1[s_i] == '.') {
                s_temp++;
            }
            else {
                break;
            }
        }
        //Comparing the s_temp variable with the utc string length to ensure the string is of valid length i.e, 10.
        if (s_temp == strlen(temp1) && temp1[6] == '.') {
            //Here the actual parsing will be done, for now I'm just using a print statement.
            printf("INFO: Valid-UTC String.\n%s\n", temp1);
        }
        else {
            printf("ERROR: UTC-Time String is not formatted correctly i.e., hhmmss.sss!!!\n");
        }
    }
    //Latitude raw string parsing and validation
    strcpy(temp1, strtok(NULL, ","));
    s_temp = 0;
    if (temp1[0] == '@'){
        //If the string is empty, the empty warning will be generated here.
        printf("WARNING: Latitude field is empty!\n");
    }
    else {
        //Loop to make sure the formatting is correct.
        for (int s_i = 0; s_i <= 8; s_i++ ) {
            //Making sure only the allowed charchters are present in the string.
            if ((temp1[s_i] >= '0' && temp1[s_i] <= '9') || temp1[s_i] == '.') {
                s_temp++;
            }
            else {
                break;
            }
        }
        //Comparing the s_temp variable with the latitude string length to ensure the string is of valid length i.e, 9.
        if (s_temp == strlen(temp1) && temp1[4] == '.') {
            //Here the actual parsing will be done, for now I'm just using a print statement.
            printf("INFO: Valid-Latitude String.\n%s\n", temp1);
        }
        else {
            printf("ERROR: Latitude String is not formatted correctly i.e., ddmm.mmmm!!!\n");
        }
    }
    //Latitude-indicator (N/S) string validation
    strcpy(temp1, strtok(NULL, ","));
    if (temp1[0] == '@'){
        //If the string is empty, the empty warning will be generated here.
        printf("WARNING: Latitude Indicator field is empty!.\n");
    }
    else if ((temp1[0] == 'N' || temp1[0] == 'S') && strlen(temp1) == 1) {
        //Already parsed because it is only one charachter N/S
        printf("INFO: Valid Indicator. \n%s\n", temp1);
    }
    else {
        //Error when indicator is invalid.
        printf("ERROR: Invalid Latitude indicator! \n");
    }
    //Longitude raw string parsing and validation
    strcpy(temp1, strtok(NULL, ","));
    s_temp = 0;
    if (temp1[0] == '@'){
        //If the string is empty, the empty warning will be generated here.
        printf("WARNING: Longitude field is empty!\n");
    }
    else {
        //Loop to make sure the formatting is correct.
        for (int s_i = 0; s_i <= 9; s_i++ ) {
            //Making sure only the allowed charchters are present in the string.
            if ((temp1[s_i] >= '0' && temp1[s_i] <= '9') || temp1[s_i] == '.') {
                s_temp++;
            }
            else {
                break;
            }
        }
        //Comparing the s_temp variable with the longitude string length to ensure the string is of valid length i.e, 10.
        if (s_temp == strlen(temp1) && temp1[5] == '.') {
            //Here the actual parsing will be done, for now I'm just using a print statement.
            printf("INFO: Valid-Longitude String.\n%s\n", temp1);
        }
        else {
            printf("ERROR: Longitude String is not formatted correctly i.e., ddmm.mmmm!!!\n");
        }
    }
    //Longitude-indicator (E/W) string validation.
    strcpy(temp1, strtok(NULL, ","));
    if (temp1[0] == '@'){
        //If the string is empty, the empty warning will be generated here.
        printf("WARNING: Longitude Indicator field is empty!\n");
    }
    else if ((temp1[0] == 'W' || temp1[0] == 'E') && strlen(temp1) == 1) {
        //Already parsed because it is only one charachter W/E.
        printf("INFO: Valid Indicator. \n%s\n", temp1);
    }
    else {
        //Error when indicator is invalid.
        printf("ERROR: Invalid Laongitude indicator! \n");
    }
    //GPS quality indicator
    strcpy(temp1, strtok(NULL, ","));
    if (temp1[0] == '@'){
        //If the string is empty, the empty warning will be generated here.
        printf("WARNING: GPS Quality Indicator field is empty!\n");
    }
    else if ((atoi(temp1) >= 0 && atoi(temp1) <= 8) && strlen(temp1) == 1) {
        //Already parsed because it is only one charachter range (0-8).
        printf("INFO: Valid quality Indicator.\n%s\n", temp1);
    }
    else {
        //Error when indicator is invalid.
        printf("ERROR: Invalid quality indicator!\n");
    }
    //Sattelites tracked for gps-data transmission
    strcpy(temp1, strtok(NULL, ","));
    s_temp = 0;
    if (temp1[0] == '@'){
        //If the string is empty, the empty warning will be generated here.
        printf("WARNING: Satellite data field is empty!\n");
    }
    else {
        //Loop to make sure the formatting is correct.
        for (int s_i = 0; s_i <= 2; s_i++ ) {
            //Making sure only the allowed charchters are present in the string. Also that maximum number of
            //satellites can be 12 Reference: NMEA-0183 documentation.
            if ((temp1[s_i] >= '0' && temp1[s_i] <= '9') && strlen(temp1) <= 2 && (atoi(temp1) >= 0 && atoi(temp1) <= 12)) {
                s_temp++;
            }
            else {
                break;
            }
        }
        //Comparing the s_temp variable with the satellite data string length to ensure the string is of valid length.
        if (s_temp == strlen(temp1)) {
            //Here the actual parsing will be done, for now I'm just using a print statement.
            printf("INFO: Valid Satellite data string.\n%s\n", temp1);
        }
        else {
            printf("ERROR: Satellite data is not Correct i.e., Range (0-12)!!!\n");
        }
    }
    //HDOP data
    strcpy(temp1, strtok(NULL, ","));
    s_temp = 0;
    if (temp1[0] == '@'){
        //If the string is empty, the empty warning will be generated here.
        printf("WARNING: HDOP data field is empty!\n");
    }
    else {
        //Loop to make sure the formatting is correct.
        for (int s_i = 0; s_i <= strlen(temp1); s_i++ ) {
            //Making sure only the allowed charchters are present in the string.
            if ((temp1[s_i] >= '0' && temp1[s_i] <= '9' && atof(temp1) >= 0.00) || temp1[s_i] == '.' ) {
                s_temp++;
            }
            else {
                break;
            }
        }
        //Comparing the s_temp variable with the HDOP string length to ensure the string is of valid length.
        if (s_temp == strlen(temp1)) {
            //Here the actual parsing will be done, for now I'm just using a print statement.
            printf("INFO: HDOP data string is valid.\n%s\n", temp1);
        }
        else {
            printf("ERROR: HDOP data is NOT Correct!!!\n");
        }
    }
    //Altitude string parsing
    strcpy(temp1, strtok(NULL, ","));
    s_temp = 0;
    if (temp1[0] == '@'){
        //If the string is empty, the empty warning will be generated here.
        printf("WARNING: Altitude data field is empty!\n");
    }
    else {
        //Loop to make sure the formatting is correct.
        for (int s_i = 0; s_i <= strlen(temp1); s_i++ ) {
            //Making sure only the allowed charchters are present in the string.
            if ((temp1[s_i] >= '0' && temp1[s_i] <= '9' && atof(temp1) >= 0.00) || temp1[s_i] == '.' ) {
                s_temp++;
            }
            else {
                break;
            }
        }
        //Comparing the s_temp variable with the Altitude string length to ensure the string is of valid length.
        if (s_temp == strlen(temp1)) {
            //Here the actual parsing will be done, for now I'm just using a print statement.
            printf("INFO: Altitude data string is valid.\n%s\n", temp1);
        }
        else {
            printf("ERROR: Altitude data is not Correct!!!\n");
        }
    }
    //Altitude units (meter/M)
    strcpy(temp1, strtok(NULL, ","));
    if (temp1[0] == '@'){
        //If the string is empty, the empty warning will be generated here.
        printf("WARNING: Longitude Indicator field is empty!\n");
    }
    else if ((temp1[0] == 'M') && strlen(temp1) == 1) {
        //Already parsed because it is only one charachter (M).
        printf("INFO: Valid unit.\n%s\n", temp1);
    }
    else {
        //Error when indicator is invalid.
        printf("ERROR: Invalid Altitude unit! \n");
    }
    //Height of geoid string parsing
    strcpy(temp1, strtok(NULL, ","));
    s_temp = 0;
    if (temp1[0] == '@'){
        //If the string is empty, the empty warning will be generated here.
        printf("WARNING: Geoid height data field is empty!\n");
    }
    else {
        //Loop to make sure the formatting is correct.
        for (int s_i = 0; s_i <= strlen(temp1); s_i++ ) {
            //Making sure only the allowed charchters are present in the string.
            if ((temp1[s_i] >= '0' && temp1[s_i] <= '9' && atof(temp1) >= 0.00) || temp1[s_i] == '.' ) {
                s_temp++;
            }
            else {
                break;
            }
        }
        //Comparing the s_temp variable with the geoid height string length to ensure the string is of valid length.
        if (s_temp == strlen(temp1)) {
            //Here the actual parsing will be done, for now I'm just using a print statement.
            printf("INFO: Geoid height data string is valid.\n%s\n", temp1);
        }
        else {
            printf("ERROR: Geoid height data is NOT Correct!!!\n");
        }
    }
    //Geoid height units (meter/M)
    strcpy(temp1, strtok(NULL, ","));
    if (temp1[0] == '@'){
        //If the string is empty, the empty warning will be generated here.
        printf("WARNING: Geoid height Indicator field is empty!\n");
    }
    else if ((temp1[0] == 'M') && strlen(temp1) == 1) {
        //Already parsed because it is only one charachter (M).
        printf("INFO: Valid unit. \n%s\n", temp1);
    }
    else {
        //Error when indicator is invalid.
        printf("ERROR: Invalid geoid height unit!\n");
    }
    //Time since last DGPS update string validation
    strcpy(temp1, strtok(NULL, ","));
    s_temp = 0;
    if (temp1[0] == '@'){
        //If the string is empty, the empty warning will be generated here.
        printf("WARNING: Time since last DGPS update data field is empty!\n");
    }
    else {
        //Loop to make sure the formatting is correct.
        for (int s_i = 0; s_i <= strlen(temp1); s_i++ ) {
            //Making sure only the allowed charchters are present in the string.
            if ((temp1[s_i] >= '0' && temp1[s_i] <= '9' && atof(temp1) >= 0.00) || temp1[s_i] == '.' ) {
                s_temp++;
            }
            else {
                break;
            }
        }
        //Comparing the s_temp variable with the Time since last DGPS update string length to ensure the string is of valid length.
        if (s_temp == strlen(temp1)) {
            //Here the actual parsing will be done, for now I'm just using a print statement.
            printf("INFO: Time since last DGPS update data string is valid.\n%s\n", temp1);
        }
        else {
            printf("ERROR: Time since last DGPS update data is NOT Correct!!!\n");
        }
    }
    //Differential reference station ID data string
    strcpy(temp1, strtok(NULL, ","));
    s_temp = 0;
    if (temp1[0] == '@'){
        //If the string is empty, the empty warning will be generated here.
        printf("WARNING: Differential reference station ID field is empty!\n");
    }
    else {
        //Loop to make sure the formatting is correct.
        for (int s_i = 0; s_i <= 2; s_i++ ) {
            //Making sure only the allowed charchters are present in the string. Also that maximum number of
            //differential reference station IDs can be 1023 Reference: NMEA-0183 documentation.
            if ((temp1[s_i] >= '0' && temp1[s_i] <= '9') && strlen(temp1) <= 4 && (atoi(temp1) >= 0 && atoi(temp1) <= 1023)) {
                s_temp++;
            }
            else {
                break;
            }
        }
        //Comparing the s_temp variable with the differential reference station ID string length to ensure the string is of valid length.
        if (s_temp == strlen(temp1)) {
            //Here the actual parsing will be done, for now I'm just using a print statement.
            printf("INFO: Valid Differential Reference Station ID.\n%s\n", temp1);
        }
        else {
            printf("ERROR: Differential reference station ID is NOT Correct i.e., Range (0000-1023)!!!\n");
        }
    }
}

//A test string is used to check whether the function Parse_gps_data is working properly.
void app_main(void) {
    char *NMEA_SENTENCE = "$GPGGA,092750.000,5321.6802,N,00630.3372,W,1,8,1.03,61.7,M,55.2,M,,*76";
    Parse_gps_data(NMEA_SENTENCE);
}