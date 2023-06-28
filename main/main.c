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
 * @brief GPS position (latitude)
 */
typedef struct {
    int latDeg;
    float latMin;
    char latInd[2];
} gpsData_latitude_t;

/**
 * @brief GPS position (longitude)
 */
typedef struct {
    int longDeg;
    float longMin;
    char longInd[2];
} gpsData_longitude_t;

/**
 * @brief GPS position (altitude)
 */
typedef struct {
    float alt;
    char altInd[2];
} gpsData_altitude_t;

/**
 * @brief GPS position integrated struct i.e., latitude, longitude, Altitude
 */
typedef struct {
    gpsData_latitude_t LATITUDE;
    gpsData_longitude_t LONGITUDE;
    gpsData_altitude_t ALTITUDE;
} gpsData_Position_t;

/**
 * @brief Geiodal Separation struct
 */
typedef struct {
    float gpsData_geoSep;
    char gpsData_geoSepInd[2];
}gpsData_GeoSep_t;

/**
 * @brief GPS parsed data
 */
typedef struct {
    gpsData_Time_t gpsData_time;
    gpsData_Position_t gpsData_position;
    gpsData_GeoSep_t gpsData_gS;
    int gpsData_satTracked;
    int gpsData_qIndicator;
    float gpsData_hdop;
    float gpsData_tDgps;
    char gpsData_drsID[5];
} nmea_Parsed_t;

/**
 * @brief Store the status of emptiness of fields
 */
typedef struct {
    bool isEmpty_time;
    bool isEmpty_latitude;
    bool isEmpty_latitudeInd;
    bool isEmpty_longitude;
    bool isEmpty_longitudeInd;
    bool isEmpty_altitude;
    bool isEmpty_altitudeInd;
    bool isEmpty_qInd;
    bool isEmpty_satellite;
    bool isEmpty_hdop;
    bool isEmpty_geoSep;
    bool isEmpty_geoSepInd;
    bool isEmpty_tDgps;
    bool isEmpty_drsID;
}gpsData_isEmpty_t;

/**
 * @brief Store the status of incorrectness of fields
 */
typedef struct {
    bool isFalse_time;
    bool isFalse_latitude;
    bool isFalse_latitudeInd;
    bool isFalse_longitude;
    bool isFalse_longitudeInd;
    bool isFalse_altitude;
    bool isFalse_altitudeInd;
    bool isFalse_qInd;
    bool isFalse_satellite;
    bool isFalse_hdop;
    bool isFalse_geoSep;
    bool isFalse_geoSepInd;
    bool isFalse_tDgps;
    bool isFalse_drsID;
}gpsData_isFalse_t;

//Global variable declarations
static nmea_Parsed_t parsedData;
static gpsData_isEmpty_t s_statusE;
static gpsData_isFalse_t s_statusF;
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
char* emptyFieldsHandler(char* SENTENCE)
{    
    char *temp = malloc(strlen(SENTENCE) + 15 + 1); // Allocate memory for the modified string
    
    if (temp == NULL) {
        printf("Memory NOT allocated! Code is now in while(1).\n");
        while(1);
    }
    
    strcpy(temp, SENTENCE);
    int j = 0; // Index for the modified string
    //Here in this loop a modified string is being created by inserting a special charachter to indicate empty strings.
    for (int i = 0; i < strlen(SENTENCE); i++) {
        temp[j++] = SENTENCE[i]; // Copy current character to temp
        
        if (SENTENCE[i] == ',' && (SENTENCE[i+1] == ',' || SENTENCE[i+1] == '*')) {
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
nmea_Parsed_t Parse_gps_data(char *SENTENCE)
{
    
    //Validate the GGA string.
    if (nmea_gga_validator(SENTENCE) == false) {
        printf("ERROR: Data is not valid!");
        return parsedData;
    }
    //Handle empty fields.
    char *data = emptyFieldsHandler(SENTENCE);
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
        //Setting the empty status true
        s_statusE.isEmpty_time = true;
    }
    else {
        //Loop to make sure the formatting is correct.
        for (int i = 0; i <= 9; i++ ) {
            //Making sure only the allowed charchters are present in the string.
            if ((temp1[i] >= '0' && temp1[i] <= '9') || temp1[i] == '.') {
                s_temp++;
            }
            else {
                break;
            }
        }
        //Comparing the s_temp variable with the utc string length to ensure the string is of valid length i.e, 10.
        if (s_temp == strlen(temp1) && temp1[6] == '.') {
            //parsing
            char tempParse[3] = "0";
            parsedData.gpsData_time.hour = atoi(strncpy(tempParse, temp1, 2));
            parsedData.gpsData_time.minutes = atoi(strncpy(tempParse, temp1 + 2, 2));
            parsedData.gpsData_time.seconds = atof(strncpy(tempParse, temp1 + 2 + 2, 2));
        }
        else {
            s_statusF.isFalse_time = true;
        }
    }
    //Latitude raw string parsing and validation
    strcpy(temp1, strtok(NULL, ","));
    s_temp = 0;
    if (temp1[0] == '@'){
        //Setting the empty status true
        s_statusE.isEmpty_latitude = true;
    }
    else {
        //Loop to make sure the formatting is correct.
        for (int i = 0; i <= 8; i++ ) {
            //Making sure only the allowed charchters are present in the string.
            if ((temp1[i] >= '0' && temp1[i] <= '9') || temp1[i] == '.') {
                s_temp++;
            }
            else {
                break;
            }
        }
        //Comparing the s_temp variable with the latitude string length to ensure the string is of valid length i.e, 9.
        if (s_temp == strlen(temp1) && temp1[4] == '.') {
            //parsing
            char tempParse[3] = "0";
            parsedData.gpsData_position.LATITUDE.latDeg = atoi(strncpy(tempParse, temp1, 2));
            parsedData.gpsData_position.LATITUDE.latMin = atof(strncpy(tempParse, temp1 + 2, strlen(temp1) - 2));
        }
        else {
            s_statusF.isFalse_latitude = true;
        }
    }
    //Latitude-indicator (N/S) string validation
    strcpy(temp1, strtok(NULL, ","));
    if (temp1[0] == '@'){
        //Setting the empty status true
        s_statusE.isEmpty_latitudeInd = true;
    }
    else if ((temp1[0] == 'N' || temp1[0] == 'S') && strlen(temp1) == 1) {
        //Store in the relevant struct variable
        strncpy(parsedData.gpsData_position.LATITUDE.latInd, temp1, 1);
    }
    else {
        //Error when indicator is invalid.
        s_statusF.isFalse_latitudeInd = true;
        printf("ERROR: Invalid Latitude indicator! \n");
    }
    //Longitude raw string parsing and validation
    strcpy(temp1, strtok(NULL, ","));
    s_temp = 0;
    if (temp1[0] == '@'){
        //Setting the empty status true
        s_statusE.isEmpty_longitude = true;
    }
    else {
        //Loop to make sure the formatting is correct.
        for (int i = 0; i <= 9; i++ ) {
            //Making sure only the allowed charchters are present in the string.
            if ((temp1[i] >= '0' && temp1[i] <= '9') || temp1[i] == '.') {
                s_temp++;
            }
            else {
                break;
            }
        }
        //Comparing the s_temp variable with the longitude string length to ensure the string is of valid length i.e, 10.
        if (s_temp == strlen(temp1) && temp1[5] == '.') {
            //parsing
            char tempParse[4] = "0";
            parsedData.gpsData_position.LONGITUDE.longDeg = atoi(strncpy(tempParse, temp1, 3));
            parsedData.gpsData_position.LONGITUDE.longMin = atof(strncpy(tempParse, temp1 + 3, strlen(temp1) - 3));
        }
        else {
            s_statusF.isFalse_longitude = true;
        }
    }
    //Longitude-indicator (E/W) string validation.
    strcpy(temp1, strtok(NULL, ","));
    if (temp1[0] == '@'){
        //Setting the empty status true
        s_statusE.isEmpty_longitudeInd = true;
    }
    else if ((temp1[0] == 'W' || temp1[0] == 'E') && strlen(temp1) == 1) {
        //Store in the relevant struct variable
        strncpy(parsedData.gpsData_position.LONGITUDE.longInd, temp1, 1);
    }
    else {
        //Error when indicator is invalid.
        s_statusF.isFalse_longitudeInd = true;
        printf("ERROR: Invalid Laongitude indicator! \n");
    }
    //GPS quality indicator
    strcpy(temp1, strtok(NULL, ","));
    if (temp1[0] == '@'){
        //Setting the empty status true
        s_statusE.isEmpty_qInd = true;
    }
    else if ((atoi(temp1) >= 0 && atoi(temp1) <= 8) && strlen(temp1) == 1) {
        //Store in the relevant struct variable
        parsedData.gpsData_qIndicator = atoi(temp1);
    }
    else {
        //Error when indicator is invalid.
        s_statusF.isFalse_qInd = true;
    }
    //Sattelites tracked for gps-data transmission
    strcpy(temp1, strtok(NULL, ","));
    s_temp = 0;
    if (temp1[0] == '@'){
        //Setting the empty status true
        s_statusE.isEmpty_satellite = true;
    }
    else {
        //Loop to make sure the formatting is correct.
        for (int i = 0; i <= 2; i++ ) {
            //Making sure only the allowed charchters are present in the string. Also that maximum number of
            //satellites can be 12 Reference: NMEA-0183 documentation.
            if ((temp1[i] >= '0' && temp1[i] <= '9') && strlen(temp1) <= 2 && (atoi(temp1) >= 0 && atoi(temp1) <= 12)) {
                s_temp++;
            }
            else {
                break;
            }
        }
        //Comparing the s_temp variable with the satellite data string length to ensure the string is of valid length.
        if (s_temp == strlen(temp1)) {
            //Store in the relevant struct variable
            parsedData.gpsData_satTracked = atoi(temp1);
        }
        else {
            s_statusF.isFalse_satellite = true;
        }
    }
    //HDOP data
    strcpy(temp1, strtok(NULL, ","));
    s_temp = 0;
    if (temp1[0] == '@'){
        //Setting the empty status true
        s_statusE.isEmpty_hdop = true;
    }
    else {
        //Loop to make sure the formatting is correct.
        for (int i = 0; i <= strlen(temp1); i++ ) {
            //Making sure only the allowed charchters are present in the string.
            if ((temp1[i] >= '0' && temp1[i] <= '9' && atof(temp1) >= 0.00) || temp1[i] == '.' ) {
                s_temp++;
            }
            else {
                break;
            }
        }
        //Comparing the s_temp variable with the HDOP string length to ensure the string is of valid length.
        if (s_temp == strlen(temp1)) {
            //Store in the relevant struct variable
            parsedData.gpsData_hdop = atof(temp1);
        }
        else {
            s_statusF.isFalse_hdop = true;
        }
    }
    //Altitude string parsing
    strcpy(temp1, strtok(NULL, ","));
    s_temp = 0;
    if (temp1[0] == '@'){
        //Setting the empty status true
        s_statusE.isEmpty_altitude = true;
    }
    else {
        //Parsing and validation of Altitude field
        if (temp1[0] == '-') {
            for (int i = 0; i <= strlen(temp1) - 1; i++) {
                temp1[i] = temp1[i + 1];
            }
            temp1[strlen(temp1)] = '\0';
            for (int i = 0; i <= strlen(temp1); i++ ) {
            //Making sure only the allowed charchters are present in the string.
            if ((temp1[i] >= '0' && temp1[i] <= '9' && atof(temp1) >= 0.00) || temp1[i] == '.' ) {
                s_temp++;
            }
            else {
                break;
            }
            }
            //Comparing the s_temp variable with the Altitude string length to ensure the string is of valid length.
            if (s_temp == strlen(temp1)) {
                //Store in the relevant struct variable
                parsedData.gpsData_position.ALTITUDE.alt = -1 * atof(temp1);
            }
            else {
                s_statusF.isFalse_altitude = true;
            }
        }
        else {
            for (int i = 0; i <= strlen(temp1); i++ ) {
                //Making sure only the allowed charchters are present in the string.
                if ((temp1[i] >= '0' && temp1[i] <= '9' && atof(temp1) >= 0.00) || temp1[i] == '.' ) {
                    s_temp++;
                }
                else {
                    break;
                }
            }
            //Comparing the s_temp variable with the Altitude string length to ensure the string is of valid length.
            if (s_temp == strlen(temp1)) {
                //Store in the relevant struct variable
                parsedData.gpsData_position.ALTITUDE.alt = atof(temp1);
            }
            else {
                s_statusF.isFalse_altitude = true;
            }
        }
    }
    //Altitude units (meter/M)
    strcpy(temp1, strtok(NULL, ","));
    if (temp1[0] == '@'){
        //Setting the empty status true
        s_statusE.isEmpty_altitudeInd = true;
    }
    else if ((temp1[0] == 'M') && strlen(temp1) == 1) {
        //Store in the relevant struct variable
            strcpy(parsedData.gpsData_position.ALTITUDE.altInd, temp1);
    }
    else {
        //Error when indicator is invalid.
        s_statusF.isFalse_altitudeInd = true;
        printf("ERROR: Invalid Altitude unit! \n");
    }
    //Height of geoid string parsing
    strcpy(temp1, strtok(NULL, ","));
    s_temp = 0;
    if (temp1[0] == '@'){
        //Setting the empty status true
        s_statusE.isEmpty_geoSep = true;
    }
    else {
        //Parsing and validation of Geoid Separation field
        if (temp1[0] == '-') {
            for (int i = 0; i <= strlen(temp1) - 1; i++) {
                temp1[i] = temp1[i + 1];
            }
            temp1[strlen(temp1)] = '\0';
            for (int i = 0; i <= strlen(temp1); i++ ) {
                //Making sure only the allowed charchters are present in the string.
                if ((temp1[i] >= '0' && temp1[i] <= '9') || temp1[i] == '.') {
                    s_temp++;
                }
                else {
                    break;
                }
            }
            //Comparing the s_temp variable with the geoid height string length to ensure the string is of valid length.
            if (s_temp == strlen(temp1)) {
                //Store in the relevant struct variable
                parsedData.gpsData_gS.gpsData_geoSep = -1 * atof(temp1);
            }
            else {
                s_statusF.isFalse_geoSep = true;
            }
        }
        else {
            for (int i = 0; i <= strlen(temp1); i++ ) {
            //Making sure only the allowed charchters are present in the string.
            if ((temp1[i] >= '0' && temp1[i] <= '9') || temp1[i] == '.') {
                s_temp++;
            }
            else {
                break;
            }
        }
        //Comparing the s_temp variable with the geoid height string length to ensure the string is of valid length.
        if (s_temp == strlen(temp1)) {
            //Store in the relevant struct variable
            parsedData.gpsData_gS.gpsData_geoSep = atof(temp1);
        }
        else {
            s_statusF.isFalse_geoSep = true;
        }
        }
        
    }
    //Geoid height units (meter/M)
    strcpy(temp1, strtok(NULL, ","));
    if (temp1[0] == '@'){
        //Setting the empty status true
        s_statusE.isEmpty_geoSepInd = true;
    }
    else if ((temp1[0] == 'M') && strlen(temp1) == 1) {
        //Store in the relevant struct variable
        strcpy(parsedData.gpsData_gS.gpsData_geoSepInd, temp1);
    }
    else {
        //Error when indicator is invalid.
        s_statusF.isFalse_geoSepInd = true;
        printf("ERROR: Invalid geoid height indicator!\n");
    }
    //Time since last DGPS update string validation
    strcpy(temp1, strtok(NULL, ","));
    s_temp = 0;
    if (temp1[0] == '@'){
        //Setting the empty status true
        s_statusE.isEmpty_tDgps = true;
    }
    else {
        //Loop to make sure the formatting is correct.
        for (int i = 0; i <= strlen(temp1); i++ ) {
            //Making sure only the allowed charchters are present in the string.
            if ((temp1[i] >= '0' && temp1[i] <= '9' && atof(temp1) >= 0.00) || temp1[i] == '.' ) {
                s_temp++;
            }
            else {
                break;
            }
        }
        //Comparing the s_temp variable with the Time since last DGPS update string length to ensure the string is of valid length.
        if (s_temp == strlen(temp1)) {
            //Store in the relevant struct variable
            parsedData.gpsData_tDgps = atof(temp1);
        }
        else {
            s_statusF.isFalse_tDgps = true;
        }
    }
    //Differential reference station ID data string
    strcpy(temp1, strtok(NULL, "\0"));
    s_temp = 0;
    if (temp1[0] == '@'){
        //Setting the empty status true
        s_statusE.isEmpty_drsID = true;
    }
    else {
        //Loop to make sure the formatting is correct.
        for (int i = 0; i <= 4; i++ ) {
            //Making sure only the allowed charchters are present in the string. Also that maximum number of
            //differential reference station IDs can be 1023 Reference: NMEA-0183 documentation.
            if ((temp1[i] >= '0' && temp1[i] <= '9') && strlen(temp1) == 4 && (atoi(temp1) >= 0 && atoi(temp1) <= 1023)) {
                s_temp++;
            }
            else {
                break;
            }
        }
        //Comparing the s_temp variable with the differential reference station ID string length to ensure the string is of valid length.
        if (s_temp == strlen(temp1)) {
            //Store in the relevant struct variable
            strcpy(parsedData.gpsData_drsID, temp1);
        }
        else {
            s_statusF.isFalse_drsID = true;
        }
    }
    return parsedData;
}

/**
 * @brief printParseData function prints the parsed data in accordance with the isEmpty and isFalse status.
 * @param nmea_Parsed_t i.e., the parsed data (struct) is given as a parameter.
 * @return void
 */
void printParsedData (nmea_Parsed_t p_data)
{
    //Print parsed data
    if (s_statusE.isEmpty_time) {
        printf("WARNING: UTC-TIME data field is empty!\n");
    }
    else if (s_statusF.isFalse_time) {
        printf("ERROR: UTC-Time String is NOT formatted correctly i.e., hhmmss.sss!!!\n");
    }
    else {
        printf("UTC-TIME----------------------------> %d:%d:%.3f\n", parsedData.gpsData_time.hour, parsedData.gpsData_time.minutes, parsedData.gpsData_time.seconds);
    }
    if (s_statusE.isEmpty_latitude) {
        printf("WARNING: LATITUDE data field is empty!\n");
    }
    else if (s_statusF.isFalse_latitude) {
        printf("ERROR: LATITUDE data is NOT formatted correctly i.e., ddmm.mmmm!!!\n");
    }
    else {
        printf("LATITUDE----------------------------> %d° %.4f\' (%s)\n", parsedData.gpsData_position.LATITUDE.latDeg, parsedData.gpsData_position.LATITUDE.latMin, parsedData.gpsData_position.LATITUDE.latInd);
    }
    if (s_statusE.isEmpty_longitude) {
        printf("WARNING: LONGITUDE data field is empty!\n");
    }
    else if (s_statusF.isFalse_longitude) {
        printf("ERROR: LONGITUDE data is NOT formatted correctly i.e., dddmm.mmmm!!!\n");
    }
    else {
        printf("LONGITUDE---------------------------> %d° %.4f\' (%s)\n", parsedData.gpsData_position.LONGITUDE.longDeg, parsedData.gpsData_position.LONGITUDE.longMin, parsedData.gpsData_position.LONGITUDE.longInd);
    }
    if (s_statusE.isEmpty_altitude) {
        printf("WARNING: ALTITUDE data field is empty!\n");
    }
    else if (s_statusF.isFalse_altitude) {
        printf("ERROR: ALTITUDE data is NOT Correct!!!\n");
    }
    else {
        printf("ALTITUDE(Above MSL)-----------------> %.1f (%s)\n", parsedData.gpsData_position.ALTITUDE.alt, parsedData.gpsData_position.ALTITUDE.altInd);
    }
    if (s_statusE.isEmpty_qInd) {
        printf("WARNING: GPS-QUALITY INDICATOR data field is empty!\n");
    }
    else if (s_statusF.isFalse_qInd) {
        printf("ERROR: GPS-QUALITY INDICATOR is NOT Correct!!!\n");
    }
    else {
        printf("GPS-QUALITY INDICATOR---------------> %d\n", parsedData.gpsData_qIndicator);
    }
    if (s_statusE.isEmpty_satellite) {
        printf("WARNING: SATELLITE TRACKED data field is empty!\n");
    }
    else if (s_statusF.isFalse_satellite) {
        printf("ERROR: SATELLITE TRACKED data is NOT Correct i.e., Range (0-12)!!!\n");
    }
    else {
        printf("SATELLITE TRACKED-------------------> %d\n", parsedData.gpsData_satTracked);
    }
    if (s_statusE.isEmpty_hdop) {
        printf("WARNING: HDOP data field is empty!\n");
    }
    else if (s_statusF.isFalse_hdop) {
        printf("ERROR: HDOP data is NOT Correct!!!\n");
    }
    else {
        printf("HDOP--------------------------------> %.2f\n", parsedData.gpsData_hdop);
    }
    if (s_statusE.isEmpty_geoSep) {
        printf("WARNING: GEOIDAL SEPARATION data field is empty!\n");
    }
    else if (s_statusF.isFalse_geoSep) {
        printf("ERROR: GEOIDAL SEPARATION data is NOT Correct!!!\n");
    }
    else {
        printf("GEOIDAL SEPARATION------------------> %.1f (%s)\n", parsedData.gpsData_gS.gpsData_geoSep, parsedData.gpsData_gS.gpsData_geoSepInd);
    }
    if (s_statusE.isEmpty_tDgps) {
        printf("WARNING: TIME OF LAST DGPS UPDATE data field is empty!\n");
    }
    else if (s_statusF.isFalse_tDgps) {
        printf("ERROR: TIME OF LAST DGPS UPDATE data is NOT Correct!!!\n");
    }
    else {
        printf("TIME OF LAST DGPS UPDATE------------> %.2f\n", parsedData.gpsData_tDgps);
    }
    if (s_statusE.isEmpty_drsID) {
        printf("WARNING: DIFFERENTIAL REFERENCE STATION ID data field is empty!\n");
    }
    else if (s_statusF.isFalse_drsID) {
        printf("ERROR: DIFFERENTIAL REFERENCE STATION ID is NOT Correct i.e., Range (0000-1023)!!!\n");
    }
    else {
        printf("DIFFERENTIAL REFERENCE STATION ID---> %s\n", parsedData.gpsData_drsID);
    }
}

/**
 * @brief getTime function prints the UTC-TIME to console and gives the hour, minute and second values to the user
 * @param NMEA_SENTENCE is given as the parameter
 * @return gpsData_Time_t is the return type which returns the utc-time
 */
gpsData_Time_t getTime (char* SENTENCE)
{
    nmea_Parsed_t getTime = Parse_gps_data(SENTENCE);
    if (s_statusE.isEmpty_longitude) {
        printf("WARNING: UTC-TIME data field is empty!\n");
        return getTime.gpsData_time;
    }
    else if (s_statusF.isFalse_time) {
        printf("ERROR: UTC-Time String is NOT formatted correctly i.e., hhmmss.sss!!!\n");
        return getTime.gpsData_time;
    }
    else {
        printf("UTC-TIME----------------------------> %d:%d:%.3f\n", getTime.gpsData_time.hour, getTime.gpsData_time.minutes, getTime.gpsData_time.seconds);
        return getTime.gpsData_time;
    }
}

/**
 * @brief getLongitude function prints the longitude to console and gives the longitude data to user
 * @param NMEA_SENTENCE is given as the parameter
 * @return gpsData_longitude_t is the return type which returns the longitude
 */
gpsData_longitude_t getLongitude (char* SENTENCE)
{
    nmea_Parsed_t getlong = Parse_gps_data(SENTENCE);
    if (s_statusE.isEmpty_longitude) {
        printf("WARNING: LONGITUDE data field is empty!\n");
        return getlong.gpsData_position.LONGITUDE;
    }
    else if (s_statusF.isFalse_longitude) {
        printf("ERROR: LONGITUDE data is NOT formatted correctly i.e., dddmm.mmmm!!!\n");
        return getlong.gpsData_position.LONGITUDE;
    }
    else {
        printf("\nLONGITUDE---------------------------> %d° %.4f\' (%s)\n", getlong.gpsData_position.LONGITUDE.longDeg, getlong.gpsData_position.LONGITUDE.longMin, getlong.gpsData_position.LONGITUDE.longInd);
        return getlong.gpsData_position.LONGITUDE;
    }   
}

/**
 * @brief getLatitude function prints the latitude to console and gives the latitude data to user
 * @param NMEA_SENTENCE is given as the parameter
 * @return gpsData_latitude_t is the return type which returns the latitude
 */
gpsData_latitude_t getLatitude (char* SENTENCE)
{
    nmea_Parsed_t getlat = Parse_gps_data(SENTENCE);
    if (s_statusE.isEmpty_longitude) {
        printf("WARNING: LONGITUDE data field is empty!\n");
        return getlat.gpsData_position.LATITUDE;
    }
    else if (s_statusF.isFalse_latitude) {
        printf("ERROR: LATITUDE data is NOT formatted correctly i.e., ddmm.mmmm!!!\n");
        return getlat.gpsData_position.LATITUDE;
    }
    else {
        printf("\nLATITUDE----------------------------> %d° %.4f\' (%s)\n", getlat.gpsData_position.LATITUDE.latDeg, getlat.gpsData_position.LATITUDE.latMin, getlat.gpsData_position.LATITUDE.latInd);
        return getlat.gpsData_position.LATITUDE;
    }
}

/**
 * @brief getAltitude function prints the altitude to console and gives the altitude data to user
 * @param NMEA_SENTENCE is given as the parameter
 * @return gpsData_altitude_t is the return type which returns the altitude
 */
gpsData_altitude_t getAltitude (char* SENTENCE)
{
    nmea_Parsed_t getalt = Parse_gps_data(SENTENCE);
    if (s_statusE.isEmpty_altitude) {
        printf("WARNING: Altitude data field is empty!\n");
        return getalt.gpsData_position.ALTITUDE;
    }
    else if (s_statusF.isFalse_altitude) {
        printf("ERROR: ALTITUDE data is NOT Correct!!!\n");
        return getalt.gpsData_position.ALTITUDE;
    }
    else {
        printf("\nALTITUDE(Above MSL)-----------------> %.1f (%s)\n", getalt.gpsData_position.ALTITUDE.alt, getalt.gpsData_position.ALTITUDE.altInd);
        return getalt.gpsData_position.ALTITUDE;
    }
}

/**
 * @brief getGeoSep function prints the geoid height to console and gives the geoid height data to user
 * @param NMEA_SENTENCE is given as the parameter
 * @return gpsData_GeoSep_t is the return type which returns the geoid height
 */
gpsData_GeoSep_t getGeoSep (char* SENTENCE)
{
    nmea_Parsed_t getGeo = Parse_gps_data(SENTENCE);
    if (s_statusE.isEmpty_geoSep) {
        printf("WARNING: GEOIDAL SEPARATION data field is empty!\n");
        return getGeo.gpsData_gS;
    }
    else if (s_statusF.isFalse_geoSep) {
        printf("ERROR: GEOIDAL SEPARATION data is NOT Correct!!!\n");
        return getGeo.gpsData_gS;
    }
    else {
        printf("GEOIDAL SEPARATION------------------> %.1f (%s)\n", getGeo.gpsData_gS.gpsData_geoSep, getGeo.gpsData_gS.gpsData_geoSepInd);
        return getGeo.gpsData_gS;
    }
}

/**
 * @brief getHdop function prints the HDOP to console and gives HDOP value to user
 * @param NMEA_SENTENCE is given as the parameter
 * @return float is the return type which returns the HDOP
 */
float getHdop (char* SENTENCE)
{
    nmea_Parsed_t gethdop = Parse_gps_data(SENTENCE);
    if (s_statusE.isEmpty_hdop) {
        printf("WARNING: HDOP data field is empty!\n");
        return gethdop.gpsData_hdop;
    }
    else if (s_statusF.isFalse_hdop) {
        printf("ERROR: HDOP data is NOT Correct!!!\n");
        return gethdop.gpsData_hdop;
    }
    else {
        printf("HDOP--------------------------------> %.f\n", gethdop.gpsData_hdop);
        return gethdop.gpsData_hdop;
    }
}

/**
 * @brief getTdgps function prints the TIME OF LAST DGPS UPDATE to console and gives tDgps value to user
 * @param NMEA_SENTENCE is given as the parameter
 * @return float is the return type which returns the gpsData_tDgps
 */
float getTdgps (char* SENTENCE)
{
    nmea_Parsed_t gettDgps = Parse_gps_data(SENTENCE);
    if (s_statusE.isEmpty_tDgps) {
        printf("WARNING: TIME OF LAST DGPS UPDATE data field is empty!\n");
        return gettDgps.gpsData_tDgps;
    }
    else if (s_statusF.isFalse_tDgps) {
        printf("ERROR: TIME OF LAST DGPS UPDATE data is NOT Correct!!!\n");
        return gettDgps.gpsData_tDgps;
    }
    else {
        printf("TIME OF LAST DGPS UPDATE------------> %.2f\n", gettDgps.gpsData_tDgps);
        return gettDgps.gpsData_tDgps;
    }
}

/**
 * @brief getSatData function prints the satellite data to console and gives satTracked value to user
 * @param NMEA_SENTENCE is given as the parameter
 * @return int is the return type which returns the gpsData_satTracked
 */
int getSatData (char* SENTENCE)
{
    nmea_Parsed_t getsat = Parse_gps_data(SENTENCE);
    if (s_statusE.isEmpty_satellite) {
        printf("WARNING: SATELLITE TRACKED data field is empty!\n");
        return getsat.gpsData_satTracked;
    }
    else if (s_statusF.isFalse_satellite) {
        printf("ERROR: SATELLITE TRACKED data is NOT Correct i.e., Range (0-12)!!!\n");
        return getsat.gpsData_satTracked;
    }
    else {
        printf("SATELLITE TRACKED-------------------> %d\n", getsat.gpsData_satTracked);
        return getsat.gpsData_satTracked;
    }
}

/**
 * @brief getSatData function prints the satellite data to console and gives satTracked value to user
 * @param NMEA_SENTENCE is given as the parameter
 * @return int is the return type which returns the gpsData_satTracked
 */
int getQInd (char* SENTENCE)
{
    nmea_Parsed_t getqind = Parse_gps_data(SENTENCE);
    if (s_statusE.isEmpty_satellite) {
        printf("WARNING: GPS-QUALITY INDICATOR data field is empty!\n");
        return getqind.gpsData_qIndicator;
    }
    else if (s_statusF.isFalse_qInd) {
        printf("ERROR: GPS-QUALITY INDICATOR is NOT Correct!!!\n");
        return getqind.gpsData_qIndicator;
    }
    else {
        printf("GPS-QUALITY INDICATOR---------------> %d\n", getqind.gpsData_qIndicator);
        return getqind.gpsData_qIndicator;
    }
}

/**
 * @brief getDrs function prints the DIFFERENTIAL REFERENCE STATION ID data to console and gives drsID value to user
 * @param NMEA_SENTENCE is given as the parameter
 * @return char* is the return type which returns the pointer to gpsData_drsID
 */
char* getDrs (char* SENTENCE)
{
    nmea_Parsed_t getdrsID = Parse_gps_data(SENTENCE);
    if (s_statusE.isEmpty_satellite) {
        printf("WARNING: DIFFERENTIAL REFERENCE STATION ID data field is empty!\n");
        return getdrsID.gpsData_drsID;
    }
    else if (s_statusF.isFalse_drsID) {
        printf("ERROR: DIFFERENTIAL REFERENCE STATION ID is NOT Correct i.e., Range (0000-1023)!!!\n");
        return getdrsID.gpsData_drsID;
    }
    else {
        printf("DIFFERENTIAL REFERENCE STATION ID---> %s\n", getdrsID.gpsData_drsID);
        return getdrsID.gpsData_drsID;
    }
}

//A test string is used to check whether the function Parse_gps_data is working properly.
void app_main(void) {
    char *NMEA_SENTENCE = "$GPGGA,002153.000,3342.6618,N,11751.3858,W,1,10,1.2,27.0,M,-34.2,M,,0000*5E";
    char* l = getDrs(NMEA_SENTENCE);
    
}