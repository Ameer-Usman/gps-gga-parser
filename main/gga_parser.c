#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gga_parser.h"

/**
 * @brief default_values of isEmpty status
 */
#define DEFAULT_ISEMPTY_STATUS {            \
    .isEmpty_time = false,                  \
    .isEmpty_latitude = false,              \
    .isEmpty_latitudeInd = false,           \
    .isEmpty_longitude = false,             \
    .isEmpty_longitudeInd = false,          \
    .isEmpty_altitude = false,              \
    .isEmpty_altitudeInd = false,           \
    .isEmpty_qInd = false,                  \
    .isEmpty_satellite = false,             \
    .isEmpty_hdop = false,                  \
    .isEmpty_geoSep = false,                \
    .isEmpty_geoSepInd = false,             \
    .isEmpty_tDgps = false,                 \
    .isEmpty_drsID = false                  \
}

/**
 * @brief default_values of isFalse status
 */
#define DEFAULT_ISFALSE_STATUS {            \
    .isFalse_gga = false,                   \
    .isFalse_time = false,                  \
    .isFalse_latitude = false,              \
    .isFalse_latitudeInd = false,           \
    .isFalse_longitude = false,             \
    .isFalse_longitudeInd = false,          \
    .isFalse_altitude = false,              \
    .isFalse_altitudeInd = false,           \
    .isFalse_qInd = false,                  \
    .isFalse_satellite = false,             \
    .isFalse_hdop = false,                  \
    .isFalse_geoSep = false,                \
    .isFalse_geoSepInd = false,             \
    .isFalse_tDgps = false,                 \
    .isFalse_drsID = false                  \
}

//Global declarations
static nmea_Parsed_t s_parsedData      = DEFAULT_PARSED_DATA;
static nmea_Parsed_t s_defaultCheck    = DEFAULT_PARSED_DATA;
static gpsData_Time_t s_time           = DEFAULT_TIME;
static gpsData_latitude_t s_latitude   = DEFAULT_LATITUDE;
static gpsData_longitude_t s_longitude = DEFAULT_LONGITUDE;
static gpsData_altitude_t s_altitude   = DEFAULT_ALTITUDE;
static gpsData_GeoSep_t s_geosep       = DEFAULT_GEOSEP;
static gpsData_isEmpty_t s_statusE     = DEFAULT_ISEMPTY_STATUS;
static gpsData_isFalse_t s_statusF     = DEFAULT_ISFALSE_STATUS;

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
        static int s_isValid = 0;
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
    char *temp = malloc(strlen(SENTENCE) + MAX_DATA_FIELDS + 1); // Allocate memory for the modified string
    
    if (temp == NULL) {
        printf("ERROR: Memory NOT allocated!\n");
        assert(temp == NULL);
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

nmea_Parsed_t Parse_gps_data(char *SENTENCE)
{
    
    //Validate the GGA string.
    if (nmea_gga_validator(SENTENCE) == false) {
        printf("ERROR: Data is not valid!\n");
        static nmea_Parsed_t s_pData = DEFAULT_PARSED_DATA;
        s_parsedData = s_pData;
        return s_parsedData;
    }
    //Handle empty fields.
    char *data = emptyFieldsHandler(SENTENCE);
    char temp[strlen(data) + 1];
    char temp1[MAX_ARR_LEN_INDV_FIELDS];
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
    if (temp1[0] == '@') {
        //Setting the empty status true
        s_statusE.isEmpty_time = true;
    }
    else {
        s_temp = 0;
        //Loop to make sure the formatting is correct.
        for (int i = 0; i <= TIME_FIELD_LEN; i++ ) {
            //Making sure only the allowed charchters are present in the string.
            if ((temp1[i] >= '0' && temp1[i] <= '9') || temp1[i] == '.') {
                s_temp++;
            }
            else {
                break;
            }
        }
        //Comparing the s_temp variable with the utc string length to ensure the string is of valid length i.e, 10.
        if (s_temp == strlen(temp1) && temp1[TIME_DEC_PNT_POS] == '.') {
            //parsing
            char tempParse[TIME_FIELD_LEN];
            s_parsedData.gpsData_time.hour = atoi(strncpy(tempParse, temp1, TIME_HOUR_STR_LEN));
            s_parsedData.gpsData_time.minutes = atoi(strncpy(tempParse, temp1 + TIME_HOUR_STR_LEN, TIME_MIN_STR_LEN));
            s_parsedData.gpsData_time.seconds = atof(strncpy(tempParse, temp1 + TIME_HOUR_STR_LEN + TIME_MIN_STR_LEN, TIME_SEC_STR_LEN));
        }
        else {
            s_statusF.isFalse_time = true;
        }
    }
    //Latitude raw string parsing and validation
    strcpy(temp1, strtok(NULL, ","));
    s_temp = 0;
    if (temp1[0] == '@') {
        //Setting the empty status true
        s_statusE.isEmpty_latitude = true;
    }
    else {
        s_temp = 0;
        //Loop to make sure the formatting is correct.
        for (int i = 0; i <= LAT_FIELD_LEN; i++ ) {
            //Making sure only the allowed charchters are present in the string.
            if ((temp1[i] >= '0' && temp1[i] <= '9') || temp1[i] == '.') {
                s_temp++;
            }
            else {
                break;
            }
        }
        //Comparing the s_temp variable with the latitude string length to ensure the string is of valid length i.e, 9.
        if (s_temp == strlen(temp1) && temp1[LAT_DEC_PNT_POS] == '.') {
            //parsing
            char tempParse[LAT_FIELD_LEN] = "0";
            s_parsedData.gpsData_position.LATITUDE.latDeg = atoi(strncpy(tempParse, temp1, LAT_DEG_LEN));
            s_parsedData.gpsData_position.LATITUDE.latMin = atof(strncpy(tempParse, temp1 + LAT_DEG_LEN, strlen(temp1) - LAT_DEG_LEN));
        }
        else {
            s_statusF.isFalse_latitude = true;
        }
    }
    //Latitude-indicator (N/S) string validation
    strcpy(temp1, strtok(NULL, ","));
    if (temp1[0] == '@') {
        //Setting the empty status true
        s_statusE.isEmpty_latitudeInd = true;
    }
    else if ((temp1[0] == 'N' || temp1[0] == 'S') && strlen(temp1) == 1) {
        //Store in the relevant struct variable
        strncpy(s_parsedData.gpsData_position.LATITUDE.latInd, temp1, 1);
    }
    else {
        //Error when indicator is invalid.
        s_statusF.isFalse_latitudeInd = true;
        printf("ERROR: Invalid Latitude indicator! \n");
    }
    //Longitude raw string parsing and validation
    strcpy(temp1, strtok(NULL, ","));
    s_temp = 0;
    if (temp1[0] == '@') {
        //Setting the empty status true
        s_statusE.isEmpty_longitude = true;
    }
    else {
        s_temp = 0;
        //Loop to make sure the formatting is correct.
        for (int i = 0; i <= LON_FIELD_LEN; i++ ) {
            //Making sure only the allowed charchters are present in the string.
            if ((temp1[i] >= '0' && temp1[i] <= '9') || temp1[i] == '.') {
                s_temp++;
            }
            else {
                break;
            }
        }
        //Comparing the s_temp variable with the longitude string length to ensure the string is of valid length i.e, 10.
        if (s_temp == strlen(temp1) && temp1[LON_DEC_PNT_POS] == '.') {
            //parsing
            char tempParse[LON_FIELD_LEN] = "0";
            s_parsedData.gpsData_position.LONGITUDE.longDeg = atoi(strncpy(tempParse, temp1, LON_DEG_LEN));
            s_parsedData.gpsData_position.LONGITUDE.longMin = atof(strncpy(tempParse, temp1 + LON_DEG_LEN, strlen(temp1) - LON_DEG_LEN));
        }
        else {
            s_statusF.isFalse_longitude = true;
        }
    }
    //Longitude-indicator (E/W) string validation.
    strcpy(temp1, strtok(NULL, ","));
    if (temp1[0] == '@') {
        //Setting the empty status true
        s_statusE.isEmpty_longitudeInd = true;
    }
    else if ((temp1[0] == 'W' || temp1[0] == 'E') && strlen(temp1) == 1) {
        //Store in the relevant struct variable
        strncpy(s_parsedData.gpsData_position.LONGITUDE.longInd, temp1, 1);
    }
    else {
        //Error when indicator is invalid.
        s_statusF.isFalse_longitudeInd = true;
        printf("ERROR: Invalid Laongitude indicator! \n");
    }
    //GPS quality indicator
    strcpy(temp1, strtok(NULL, ","));
    if (temp1[0] == '@') {
        //Setting the empty status true
        s_statusE.isEmpty_qInd = true;
    }
    else if ((atoi(temp1) >= MIN_QI_VAL && atoi(temp1) <= MAX_QI_VAL) && strlen(temp1) == QI_FIELD_LEN) {
        //Store in the relevant struct variable
        s_parsedData.gpsData_qIndicator = atoi(temp1);
    }
    else {
        //Error when indicator is invalid.
        s_statusF.isFalse_qInd = true;
    }
    //Sattelites tracked for gps-data transmission
    strcpy(temp1, strtok(NULL, ","));
    s_temp = 0;
    if (temp1[0] == '@') {
        //Setting the empty status true
        s_statusE.isEmpty_satellite = true;
    }
    else {
        s_temp = 0;
        //Loop to make sure the formatting is correct.
        for (int i = 0; i <= SAT_FIELD_LEN; i++ ) {
            //Making sure only the allowed charchters are present in the string. Also that maximum number of
            //satellites can be 12 Reference: NMEA-0183 documentation.
            if ((temp1[i] >= '0' && temp1[i] <= '9') && strlen(temp1) <= SAT_FIELD_LEN && (atoi(temp1) >= MIN_SAT_VAL && atoi(temp1) <= MAX_SAT_VAL)) {
                s_temp++;
            }
            else {
                break;
            }
        }
        //Comparing the s_temp variable with the satellite data string length to ensure the string is of valid length.
        if (s_temp == strlen(temp1)) {
            //Store in the relevant struct variable
            s_parsedData.gpsData_satTracked = atoi(temp1);
        }
        else {
            s_statusF.isFalse_satellite = true;
        }
    }
    //HDOP data
    strcpy(temp1, strtok(NULL, ","));
    s_temp = 0;
    if (temp1[0] == '@') {
        //Setting the empty status true
        s_statusE.isEmpty_hdop = true;
    }
    else {
        s_temp = 0;
        //Loop to make sure the formatting is correct.
        for (int i = 0; i <= strlen(temp1); i++ ) {
            //Making sure only the allowed charchters are present in the string.
            if ((temp1[i] >= '0' && temp1[i] <= '9' && atof(temp1) > 0.00) || temp1[i] == '.' ) {
                s_temp++;
            }
            else {
                break;
            }
        }
        //Comparing the s_temp variable with the HDOP string length to ensure the string is of valid length.
        if (s_temp == strlen(temp1)) {
            //Store in the relevant struct variable
            s_parsedData.gpsData_hdop = atof(temp1);
        }
        else {
            s_statusF.isFalse_hdop = true;
        }
    }
    //Altitude string parsing
    strcpy(temp1, strtok(NULL, ","));
    s_temp = 0;
    if (temp1[0] == '@') {
        //Setting the empty status true
        s_statusE.isEmpty_altitude = true;
    }
    else {
        s_temp = 0;
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
                s_parsedData.gpsData_position.ALTITUDE.alt = -1 * atof(temp1);
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
                s_parsedData.gpsData_position.ALTITUDE.alt = atof(temp1);
            }
            else {
                s_statusF.isFalse_altitude = true;
            }
        }
    }
    //Altitude units (meter/M)
    strcpy(temp1, strtok(NULL, ","));
    if (temp1[0] == '@') {
        //Setting the empty status true
        s_statusE.isEmpty_altitudeInd = true;
    }
    else if ((temp1[0] == 'M') && strlen(temp1) == 1) {
        //Store in the relevant struct variable
            strcpy(s_parsedData.gpsData_position.ALTITUDE.altInd, temp1);
    }
    else {
        //Error when indicator is invalid.
        s_statusF.isFalse_altitudeInd = true;
        printf("ERROR: Invalid Altitude unit! \n");
    }
    //Height of geoid string parsing
    strcpy(temp1, strtok(NULL, ","));
    s_temp = 0;
    if (temp1[0] == '@') {
        //Setting the empty status true
        s_statusE.isEmpty_geoSep = true;
    }
    else {
        s_temp = 0;
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
                s_parsedData.gpsData_gS.gpsData_geoSep = -1 * atof(temp1);
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
            s_parsedData.gpsData_gS.gpsData_geoSep = atof(temp1);
        }
        else {
            s_statusF.isFalse_geoSep = true;
        }
        }
        
    }
    //Geoid height units (meter/M)
    strcpy(temp1, strtok(NULL, ","));
    if (temp1[0] == '@') {
        //Setting the empty status true
        s_statusE.isEmpty_geoSepInd = true;
    }
    else if ((temp1[0] == 'M') && strlen(temp1) == 1) {
        //Store in the relevant struct variable
        strcpy(s_parsedData.gpsData_gS.gpsData_geoSepInd, temp1);
    }
    else {
        //Error when indicator is invalid.
        s_statusF.isFalse_geoSepInd = true;
        printf("ERROR: Invalid geoid height indicator!\n");
    }
    //Time since last DGPS update string validation
    strcpy(temp1, strtok(NULL, ","));
    s_temp = 0;
    if (temp1[0] == '@') {
        //Setting the empty status true
        s_statusE.isEmpty_tDgps = true;
    }
    else {
        s_temp = 0;
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
            s_parsedData.gpsData_tDgps = atof(temp1);
        }
        else {
            s_statusF.isFalse_tDgps = true;
        }
    }
    //Differential reference station ID data string
    strcpy(temp1, strtok(NULL, "\0"));
    s_temp = 0;
    if (temp1[0] == '@') {
        //Setting the empty status true
        s_statusE.isEmpty_drsID = true;
    }
    else {
        s_temp = 0;
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
            strcpy(s_parsedData.gpsData_drsID, temp1);
        }
        else {
            s_statusF.isFalse_drsID = true;
        }
    }
    return s_parsedData;
}

/**
 * @brief checks the time format based on indvidual values of hr, min, and sec
 * @param void
 * @return void
 */
void checkTime(void)
{
    if (s_parsedData.gpsData_time.hour < 0 || s_parsedData.gpsData_time.hour > 24) {
        printf("ERROR: Hour parameter out of range!!!\n");
        s_statusF.isFalse_time = true;
    }
    if (s_parsedData.gpsData_time.minutes < 0 || s_parsedData.gpsData_time.minutes >= 60) {
        printf("ERROR: Minute parameter out of range!!!\n");
        s_statusF.isFalse_time = true;
    }
    if (s_parsedData.gpsData_time.seconds < 0.0 || s_parsedData.gpsData_time.seconds >= 60.0) {
        printf("ERROR: Seconds parameter out of range!!!\n");
        s_statusF.isFalse_time = true;
    }
}

/**
 * @brief printParseData function prints the parsed data in accordance with the isEmpty and isFalse status.
 * @param nmea_Parsed_t i.e., the parsed data (struct) is given as a parameter.
 * @return void
 */
void printParsedData (nmea_Parsed_t p_data)
{
    //Print parsed data
    if ((p_data.gpsData_time.hour != s_defaultCheck.gpsData_time.hour && p_data.gpsData_time.minutes != s_defaultCheck.gpsData_time.minutes && p_data.gpsData_time.seconds != s_defaultCheck.gpsData_time.seconds) ) {
        checkTime();
    }
    
    if (s_statusE.isEmpty_time) {
        printf("WARNING: UTC-TIME data field is empty!\n");
    }
    else if (s_statusF.isFalse_time) {
        printf("ERROR: UTC-Time String is NOT formatted correctly i.e., hhmmss.sss!!!\n");
        s_statusF.isFalse_time = false;
    }
    else {
        printf("UTC-TIME----------------------------> %d:%d:%.3f\n", s_parsedData.gpsData_time.hour, s_parsedData.gpsData_time.minutes, s_parsedData.gpsData_time.seconds);
    }
    if (s_statusE.isEmpty_latitude) {
        printf("WARNING: LATITUDE data field is empty!\n");
    }
    else if (s_statusF.isFalse_latitude) {
        printf("ERROR: LATITUDE data is NOT formatted correctly i.e., ddmm.mmmm!!!\n");
        s_statusF.isFalse_latitude = false;
    }
    else {
        printf("LATITUDE----------------------------> %d° %.4f\' (%s)\n", s_parsedData.gpsData_position.LATITUDE.latDeg, s_parsedData.gpsData_position.LATITUDE.latMin, s_parsedData.gpsData_position.LATITUDE.latInd);
    }
    if (s_statusE.isEmpty_longitude) {
        printf("WARNING: LONGITUDE data field is empty!\n");
    }
    else if (s_statusF.isFalse_longitude) {
        printf("ERROR: LONGITUDE data is NOT formatted correctly i.e., dddmm.mmmm!!!\n");
        s_statusF.isFalse_longitude = false;
    }
    else {
        printf("LONGITUDE---------------------------> %d° %.4f\' (%s)\n", s_parsedData.gpsData_position.LONGITUDE.longDeg, s_parsedData.gpsData_position.LONGITUDE.longMin, s_parsedData.gpsData_position.LONGITUDE.longInd);
    }
    if (s_statusE.isEmpty_altitude) {
        printf("WARNING: ALTITUDE data field is empty!\n");
    }
    else if (s_statusF.isFalse_altitude) {
        printf("ERROR: ALTITUDE data is NOT Correct!!!\n");
        s_statusF.isFalse_altitude = false;
    }
    else {
        printf("ALTITUDE(Above MSL)-----------------> %.1f (%s)\n", s_parsedData.gpsData_position.ALTITUDE.alt, s_parsedData.gpsData_position.ALTITUDE.altInd);
    }
    if (s_statusE.isEmpty_qInd) {
        printf("WARNING: GPS-QUALITY INDICATOR data field is empty!\n");
    }
    else if (s_statusF.isFalse_qInd) {
        printf("ERROR: GPS-QUALITY INDICATOR is NOT Correct!!!\n");
        s_statusF.isFalse_qInd = false;
    }
    else {
        printf("GPS-QUALITY INDICATOR---------------> %d\n", s_parsedData.gpsData_qIndicator);
    }
    if (s_statusE.isEmpty_satellite) {
        printf("WARNING: SATELLITE TRACKED data field is empty!\n");
    }
    else if (s_statusF.isFalse_satellite) {
        printf("ERROR: SATELLITE TRACKED data is NOT Correct i.e., Range (0-12)!!!\n");
        s_statusF.isFalse_satellite = false;
    }
    else {
        printf("SATELLITE TRACKED-------------------> %d\n", s_parsedData.gpsData_satTracked);
    }
    if (s_statusE.isEmpty_hdop) {
        printf("WARNING: HDOP data field is empty!\n");
    }
    else if (s_statusF.isFalse_hdop) {
        printf("ERROR: HDOP data is NOT Correct!!!\n");
        s_statusF.isFalse_hdop = false;
    }
    else {
        printf("HDOP--------------------------------> %.1f\n", s_parsedData.gpsData_hdop);
    }
    if (s_statusE.isEmpty_geoSep) {
        printf("WARNING: GEOIDAL SEPARATION data field is empty!\n");
    }
    else if (s_statusF.isFalse_geoSep) {
        printf("ERROR: GEOIDAL SEPARATION data is NOT Correct!!!\n");
        s_statusF.isFalse_geoSep = false;
    }
    else {
        printf("GEOIDAL SEPARATION------------------> %.1f (%s)\n", s_parsedData.gpsData_gS.gpsData_geoSep, s_parsedData.gpsData_gS.gpsData_geoSepInd);
    }
    if (s_statusE.isEmpty_tDgps) {
        printf("WARNING: TIME OF LAST DGPS UPDATE data field is empty!\n");
    }
    else if (s_statusF.isFalse_tDgps) {
        printf("ERROR: TIME OF LAST DGPS UPDATE data is NOT Correct!!!\n");
        s_statusF.isFalse_tDgps = false;
    }
    else {
        printf("TIME OF LAST DGPS UPDATE------------> %.2f\n", s_parsedData.gpsData_tDgps);
    }
    if (s_statusE.isEmpty_drsID) {
        printf("WARNING: DIFFERENTIAL REFERENCE STATION ID data field is empty!\n");
    }
    else if (s_statusF.isFalse_drsID) {
        printf("ERROR: DIFFERENTIAL REFERENCE STATION ID is NOT Correct i.e., Range (0000-1023)!!!\n");
        s_statusF.isFalse_drsID = false;
    }
    else {
        printf("DIFFERENTIAL REFERENCE STATION ID---> %s\n", s_parsedData.gpsData_drsID);
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
    checkTime();
    if (s_statusE.isEmpty_longitude) {
        printf("WARNING: UTC-TIME data field is empty!\n");
        return s_time; //default values return
    }
    else if (s_statusF.isFalse_time) {
        printf("ERROR: UTC-Time String is NOT formatted correctly i.e., hhmmss.sss!!!\n");
        return s_time; //default values return
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
        return s_longitude; //default values return
    }
    else if (s_statusF.isFalse_longitude) {
        printf("ERROR: LONGITUDE data is NOT formatted correctly i.e., dddmm.mmmm!!!\n");
        return s_longitude; //default values return
    }
    else {
        printf("LONGITUDE---------------------------> %d° %.4f\' (%s)\n", getlong.gpsData_position.LONGITUDE.longDeg, getlong.gpsData_position.LONGITUDE.longMin, getlong.gpsData_position.LONGITUDE.longInd);
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
        return s_latitude; //default values return
    }
    else if (s_statusF.isFalse_latitude) {
        printf("ERROR: LATITUDE data is NOT formatted correctly i.e., ddmm.mmmm!!!\n");
        return s_latitude; //default values return
    }
    else {
        printf("LATITUDE----------------------------> %d° %.4f\' (%s)\n", getlat.gpsData_position.LATITUDE.latDeg, getlat.gpsData_position.LATITUDE.latMin, getlat.gpsData_position.LATITUDE.latInd);
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
        return s_altitude; //default values return
    }
    else if (s_statusF.isFalse_altitude) {
        printf("ERROR: ALTITUDE data is NOT Correct!!!\n");
        return s_altitude; //default values return
    }
    else {
        printf("ALTITUDE(Above MSL)-----------------> %.1f (%s)\n", getalt.gpsData_position.ALTITUDE.alt, getalt.gpsData_position.ALTITUDE.altInd);
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
        return s_geosep; //default values return
    }
    else if (s_statusF.isFalse_geoSep) {
        printf("ERROR: GEOIDAL SEPARATION data is NOT Correct!!!\n");
        return s_geosep; //default values return
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
        return s_parsedData.gpsData_hdop; //default values return
    }
    else if (s_statusF.isFalse_hdop) {
        printf("ERROR: HDOP data is NOT Correct!!!\n");
        return s_parsedData.gpsData_hdop; //default values return
    }
    else {
        printf("HDOP--------------------------------> %.1f\n", gethdop.gpsData_hdop);
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
        return s_parsedData.gpsData_tDgps; //default values return
    }
    else if (s_statusF.isFalse_tDgps) {
        printf("ERROR: TIME OF LAST DGPS UPDATE data is NOT Correct!!!\n");
        return s_parsedData.gpsData_tDgps; //default values return
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
        return s_parsedData.gpsData_satTracked; //default values return
    }
    else if (s_statusF.isFalse_satellite) {
        printf("ERROR: SATELLITE TRACKED data is NOT Correct i.e., Range (0-12)!!!\n");
        return s_parsedData.gpsData_satTracked; //default values return
    }
    else {
        printf("SATELLITE TRACKED-------------------> %d\n", getsat.gpsData_satTracked);
        return getsat.gpsData_satTracked;
    }
}

/**
 * @brief getSatData function prints the satellite data to console and gives satTracked value to user
 * @param NMEA_SENTENCE is given as the parameter
 * @return int is the return type which returns the gps quality indicator
 */
int getQInd (char* SENTENCE)
{
    nmea_Parsed_t getqind = Parse_gps_data(SENTENCE);
    if (s_statusE.isEmpty_satellite) {
        printf("WARNING: GPS-QUALITY INDICATOR data field is empty!\n");
        return s_parsedData.gpsData_qIndicator; //return default value
    }
    else if (s_statusF.isFalse_qInd) {
        printf("ERROR: GPS-QUALITY INDICATOR is NOT Correct!!!\n");
        return s_parsedData.gpsData_qIndicator; //return default value
    }
    else {
        printf("GPS-QUALITY INDICATOR---------------> %d\n", getqind.gpsData_qIndicator);
        return getqind.gpsData_qIndicator;
    }
}

/**
 * @brief getDrs function prints the DIFFERENTIAL REFERENCE STATION ID data to console and gives drsID value to user
 * @param SENTENCE is given as the parameter
 * @param buffer is the buffer to store the drsID value the buffer size must be atleast 5
 * @return void
 */
void getDrs(char* SENTENCE, char* buffer)
{
    nmea_Parsed_t getdrsID = Parse_gps_data(SENTENCE);
    if (s_statusE.isEmpty_satellite) {
        printf("WARNING: DIFFERENTIAL REFERENCE STATION ID data field is empty!\n");
        strcpy(buffer, s_parsedData.gpsData_drsID); // Copy default value to buffer
    }
    else if (s_statusF.isFalse_drsID) {
        printf("ERROR: DIFFERENTIAL REFERENCE STATION ID is NOT Correct i.e., Range (0000-1023)!!!\n");
        strcpy(buffer, s_parsedData.gpsData_drsID); // Copy default value to buffer
    }
    else {
        printf("DIFFERENTIAL REFERENCE STATION ID---> %s\n", getdrsID.gpsData_drsID);
        strcpy(buffer, getdrsID.gpsData_drsID); // Copy drsID value to buffer
    }
}