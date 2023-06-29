#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gga_parser.h"

void app_main()
{
    //Valid GGA-SENTENCE
    char* nmea = "$GPGGA,002153.000,3342.6618,N,11751.3858,W,1,10,1.2,27.0,M,-34.2,M,,0000*5E";
    char* drs = malloc(5); //to hold drs ID minimum memory size is 5

    /**
     * @brief In this statement Parse_gps_data(nmea) results a nmea_Parsed_t struct which is then given to printParsed data
     * -to print the parsed data on console
    */
    printf("\n\nPrinting all the parameters at the same time.\n\n");
    printParsedData(Parse_gps_data(nmea));

    /**
     * @brief These functions returns indvidual parameters
    */
    printf("\n\nPrinting all the parameters indvidually.\n\n");
    getTime(nmea);
    getLatitude(nmea);
    getLongitude(nmea);
    getAltitude(nmea);
    getGeoSep(nmea);
    getTdgps(nmea);
    getSatData(nmea);
    getQInd(nmea);
    getHdop(nmea);
    getDrs(nmea, drs);
    
    //invalid gga sentence i.e., error handling of sentences other than GGA
    nmea = "$GPGSA,A,3,10,07,05,02,29,04,08,13,,,,,1.72,1.03,1.38*0A";
    printf("\n\nPrinting all the parameters at the same time.\nThe default values will be printed out as the sentence is invalid.\n\n");
    printParsedData(Parse_gps_data(nmea));
    
    //Valid gga string parsing with error handling of incorrect fields
    nmea = "$GPGGA,002153.000,3342.6618,N,11751.3858,W,-1,10,1.2,27.0,M,34.2,M,,0000*5E";
    //Sentence is valid but Quality indicator data is incorrect (done intentionally by writing negative sign with it)
    //The negative sign is taken from the GeoSep value this way checksum will be valid but there will be incorrect field
    //i.e., Quality indicator field
    //GeoSep value will still be formatted correctly because the positive value is also possible for the data
    printf("\n\nThe value of Quality Indicator will be incorrect here, because it cannot be negative.\n\n");
    getQInd(nmea);

    //Valid GGA sentence but invalid utc time error handling
    //similarly minutes and seconds are handled
    nmea = "$GPGGA,500213.000,3342.6618,N,11751.3858,W,1,10,1.2,27.0,M,-34.2,M,,0000*5E";
    printf("\n\nUTC-time (hour) is out of range so an error will be generated.\n\n");
    getTime(nmea);

    //Invalid checksum is also detected now I will take the valid GGA string and remove any random charachter from the string
    //to invalidate the data for detection
    //I removed first charachter of longitude string to make the data invalid, the default values will be shown on the console
    nmea = "$GPGGA,002153.000,3342.6618,N,1751.3858,W,1,10,1.2,27.0,M,-34.2,M,,0000*5E";
    printf("\n\nThe GGA sentence will be confirmed but the data will invalid and an error will be generated.\n\n");
    printParsedData(Parse_gps_data(nmea));

    free(drs); //free the allocated memory
}