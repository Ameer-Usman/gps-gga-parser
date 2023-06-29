


#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define IND_ARR_LEN 2                   //indicator array length
#define MAX_DATA_FIELDS 15              //Maximum number of data fields in GGA sentence
#define MAX_ARR_LEN_INDV_FIELDS 15      //Maximum length set for indvidual fields (null charachter included) (min. val = 10)

#define TIME_FIELD_LEN 9                //Time field length (without null charachter)
#define TIME_DEC_PNT_POS 6              //Decimal point position in raw time string
#define TIME_HOUR_STR_LEN 2             //Hour string length (without null charachter)
#define TIME_MIN_STR_LEN 2              //Minute string length (without null charachter)
#define TIME_SEC_STR_LEN 6              //Seconds string length (without null charachter)

#define LAT_FIELD_LEN 8                 //Latitude field length (without null charachter)
#define LAT_DEC_PNT_POS 4               //Decimal point position in raw time string
#define LAT_DEG_LEN 2                   //Latitude field length (without null charachter)

#define LON_FIELD_LEN 9                 //Latitude field length (without null charachter)
#define LON_DEC_PNT_POS 5               //Decimal point position in raw time string
#define LON_DEG_LEN 3                   //Latitude field length (without null charachter)

#define QI_FIELD_LEN 1                  //Maximum length of GPS_QI string (without null charachter)
#define MIN_QI_VAL 0                    //Minimum gps-qIndicator value
#define MAX_QI_VAL 8                    //Maximum gps-qIndicator value

#define SAT_FIELD_LEN 2                 //Satellite data field length
#define MIN_SAT_VAL 0                   //Minimum gps-qIndicator value
#define MAX_SAT_VAL 12                  //Maximum gps-qIndicator value

#define DRS_ID_ARR_LEN 5                //DRS array length (null charachter included)

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
    char latInd[IND_ARR_LEN];
} gpsData_latitude_t;

/**
 * @brief GPS position (longitude)
 */
typedef struct {
    int longDeg;
    float longMin;
    char longInd[IND_ARR_LEN];
} gpsData_longitude_t;

/**
 * @brief GPS position (altitude)
 */
typedef struct {
    float alt;
    char altInd[IND_ARR_LEN];
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
    char gpsData_geoSepInd[IND_ARR_LEN];
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
    char gpsData_drsID[DRS_ID_ARR_LEN];
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
    bool isFalse_gga;
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

/**
 * @brief default_values upon Incorrect data for time
 */
#define DEFAULT_TIME {  \
    .hour = 99,         \
    .minutes = 99,      \
    .seconds = 99.999   \
}

/**
 * @brief default_values upon Incorrect data for latitude
 */
#define DEFAULT_LATITUDE {  \
    .latDeg = 99,           \
    .latMin = 99.9999,      \
    .latInd = "#"           \
}

/**
 * @brief default_values upon Incorrect data for longitude
 */
#define DEFAULT_LONGITUDE { \
    .longDeg = 99,          \
    .longMin = 999.9999,    \
    .longInd = "#"          \
}

/**
 * @brief default_values upon Incorrect data for altitude
 */
#define DEFAULT_ALTITUDE {  \
    .alt = 999.9,           \
    .altInd = "#"           \
}

/**
 * @brief default_values upon Incorrect data for position i.e., latitude, longitude and altitude
 */
#define DEFAULT_POSITION {          \
    .LATITUDE = DEFAULT_LATITUDE,   \
    .LONGITUDE = DEFAULT_LONGITUDE, \
    .ALTITUDE = DEFAULT_ALTITUDE    \
}

/**
 * @brief default_values upon Incorrect data for geo-separation
 */
#define DEFAULT_GEOSEP {            \
    .gpsData_geoSep = 999.9, \
    .gpsData_geoSepInd = "#" \
}

/**
 * @brief default_values upon Incorrect data of full parsed sentence i.e, if the whole data is incorrect
 */
#define DEFAULT_PARSED_DATA {               \
    .gpsData_time = DEFAULT_TIME,           \
    .gpsData_position = DEFAULT_POSITION,   \
    .gpsData_gS = DEFAULT_GEOSEP,           \
    .gpsData_satTracked = 99,               \
    .gpsData_qIndicator = 9,                \
    .gpsData_hdop = 999.9,                  \
    .gpsData_tDgps = 999.99,                \
    .gpsData_drsID = "####"                 \
}

/**
 * @brief nmea_gga_validator function validates and check the input NMEA sentence by checking its integrity.
 * @param The NMEA sentence is given to the function as the parameter through a pointer.
 * @return The return type is bool which indicates whether the sentence is valid or not (true or false)
 */
bool nmea_gga_validator(char* );

/**
 * @brief Parse_gps_data function parses the validated string into readable information such as longitude, latitude, time etc.
 * @param NMEA_SENTENCE is given to the function as the input parameter.
 * @return The return type of the function is nmea_Parsed_t type which will provide the parsed data.
 */
nmea_Parsed_t Parse_gps_data(char* );

/**
 * @brief printParseData function prints the parsed data in accordance with the isEmpty and isFalse status.
 * @param nmea_Parsed_t i.e., the parsed data (struct) is given as a parameter.
 * @return void
 */
void printParsedData (nmea_Parsed_t );

/**
 * @brief getTime function prints the UTC-TIME to console and gives the hour, minute and second values to the user
 * @param NMEA_SENTENCE is given as the parameter
 * @return gpsData_Time_t is the return type which returns the utc-time
 */
gpsData_Time_t getTime (char* );

/**
 * @brief getLongitude function prints the longitude to console and gives the longitude data to user
 * @param NMEA_SENTENCE is given as the parameter
 * @return gpsData_longitude_t is the return type which returns the longitude
 */
gpsData_longitude_t getLongitude (char* );

/**
 * @brief getLatitude function prints the latitude to console and gives the latitude data to user
 * @param NMEA_SENTENCE is given as the parameter
 * @return gpsData_latitude_t is the return type which returns the latitude
 */
gpsData_latitude_t getLatitude (char* );

/**
 * @brief getAltitude function prints the altitude to console and gives the altitude data to user
 * @param NMEA_SENTENCE is given as the parameter
 * @return gpsData_altitude_t is the return type which returns the altitude
 */
gpsData_altitude_t getAltitude (char* );

/**
 * @brief getGeoSep function prints the geoid height to console and gives the geoid height data to user
 * @param NMEA_SENTENCE is given as the parameter
 * @return gpsData_GeoSep_t is the return type which returns the geoid height
 */
gpsData_GeoSep_t getGeoSep (char* );

/**
 * @brief getHdop function prints the HDOP to console and gives HDOP value to user
 * @param NMEA_SENTENCE is given as the parameter
 * @return float is the return type which returns the HDOP
 */
float getHdop (char* );

/**
 * @brief getTdgps function prints the TIME OF LAST DGPS UPDATE to console and gives tDgps value to user
 * @param NMEA_SENTENCE is given as the parameter
 * @return float is the return type which returns the gpsData_tDgps
 */
float getTdgps (char* );

/**
 * @brief getSatData function prints the satellite data to console and gives satTracked value to user
 * @param NMEA_SENTENCE is given as the parameter
 * @return int is the return type which returns the gpsData_satTracked
 */
int getSatData (char* SENTENCE);

/**
 * @brief getSatData function prints the satellite data to console and gives satTracked value to user
 * @param NMEA_SENTENCE is given as the parameter
 * @return int is the return type which returns the gpsData_satTracked
 */
int getQInd (char* SENTENCE);

/**
 * @brief getDrs function prints the DIFFERENTIAL REFERENCE STATION ID data to console and gives drsID value to user
 * @param NMEA_SENTENCE is given as the parameter
 * @return char* is the return type which returns the pointer to gpsData_drsID
 */
char* getDrs (char* SENTENCE);

#ifdef __cplusplus
}
#endif