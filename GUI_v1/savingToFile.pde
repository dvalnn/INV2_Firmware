//  for now I am creating a logFile to save every packet that's sent and received as well as every user interaction and a
//data file to save just the important sensor data 

PrintWriter logFile, dataFile;

String logFileName = "log.txt";
String dataFileName = "data.csv";

void initializeWriters(){
    logFile = createWriter(logFileName);
    dataFile = createWriter(dataFileName);
    
    log("INITIALIZED WRITERS");
}

void flushWriters(){
    logFile.flush();
    logFile.close();

    dataFile.flush();
    dataFile.close();
}

void log(String toLog){
    logFile.print(toLog);
    logFile.println("\t--> " + millis() + "ms");
}