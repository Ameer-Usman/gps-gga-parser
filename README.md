# GGA PARSING README file
## VERSION v1.0

## gga_parser LIBRARY

### OVERVIEW
- This library provides the functionality of parsing the NMEA-0183 based GGA sentence into individual fields and handles the related errors. 
- It takes the string as an input and checks the string for valid data through checksum algorithm.
- After successful validation, the string is then given to the empty field handler function, which introduces a special character in each of the empty fields.
- Each field has its own Empty status handler variable of type bool within gpsData_isEmpty_t struct to handle empty fields accordingly and generate warnings about the empty fields.
- For incorrect data handling, each field has its own isFalse status handler variable of type bool within gpsData_isFalse_t struct to handle incorrect data and generate error messages accordingly.
- If the data is valid and the sentence is GGA, the data will be returned from the Parse_gps_data function.
- A function is also included to automatically print the data on the console with proper formatting.
- Individual fields can also be obtained by using the relevant getter functions that are discussed in the "FUNCTIONS" section.

This header file provides a library to parse the GGA sentence of NMEA-0183 GPS format.

### FEATURES
The library can parse the following data from the GGA sentence:
1) GPS time (UTC)
2) Latitude
3) Longitude
4) Altitude
5) Geoid height
6) HDOP
7) TIME OF LAST DGPS UPDATE
8) Number of satellites tracked
9) GPS quality indicator
10) DIFFERENTIAL REFERENCE STATION ID
The library provides functions to print the parsed data to the console.
The library also provides functions to get the individual data fields from the GGA sentence.

## USAGE
To use the library, you will need to include the gga_parser.h header file in your project. You can then use the following functions to parse the GGA sentence:

- `nmea_Parsed_t Parse_gps_data(char* );`
- `void printParsedData (nmea_Parsed_t );`

The `Parse_gps_data()` function will parse the GGA sentence and return an `nmea_Parsed_t` struct that contains the parsed data.
The `printParsedData()` function will print the parsed data to the console.

You can also use the following functions to get the individual data fields from the GGA sentence:

- `gpsData_Time_t getTime (char* );`
- `gpsData_latitude_t getLatitude (char* );`
- `gpsData_longitude_t getLongitude (char* );`
- `gpsData_altitude_t getAltitude (char* );`
- `gpsData_GeoSep_t getGeoSep (char* );`
- `float getHdop (char* );`
- `float getTdgps (char* );`
- `int getSatData (char* );`
- `int getQInd (char* );`
- `void getDrs(char*, char*); //The second parameter is a pointer to a character to hold the string of drsID`

## TEST CODE
The TestCode.c file is provided in the main folder, which demonstrates the basic implementation of the library with extensive comments.

## EXAMPLE OUTPUT FOR ALL DATA PRINTOUT
For a valid GGA sentence, the output of the `printParsedData(nmea_Parsed_t )` is shown:
## GGA SENTENCE:
### $GPGGA,002153.000,3342.6618,N,11751.3858,W,1,10,1.2,27.0,M,-34.2,M,,0000*5E
INFO: GGA packet confirmed.

UTC-TIME----------------------------> 0:21:53.000

LATITUDE----------------------------> 33° 42.6618' (N)

LONGITUDE---------------------------> 117° 51.3858' (W)

ALTITUDE(Above MSL)-----------------> 27.0 (M)

GPS-QUALITY INDICATOR---------------> 1

SATELLITE TRACKED-------------------> 10

HDOP--------------------------------> 1.2

GEOIDAL SEPARATION------------------> -34.2 (M)

WARNING: TIME OF LAST DGPS UPDATE data field is empty!

DIFFERENTIAL REFERENCE STATION ID---> 0000

## CONCLUSION
The library is written using simple implementation string operations such as strcmp, strcpy, strtok, strncpy, etc. It provides extensive error handling along with the flexibility of getting individual field data with simple getter functions.
Feel free to contact back for any queries and suggestions.

## AUTHOR
- NAME: M-AMEER USMAN
- EMAIL: ameeru572@gmail.com
- GIT-HUB: @Ameer-Usman
