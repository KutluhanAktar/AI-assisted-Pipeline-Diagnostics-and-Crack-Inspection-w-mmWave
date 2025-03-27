         /////////////////////////////////////////////  
        //    AI-assisted Pipeline Diagnostics     //
       //      and Crack Inspection w/ mmWave     //
      //             ---------------             //
     //          (Arduino Nicla Vision)         //           
    //             by Kutluhan Aktar           // 
   //                                         //
  /////////////////////////////////////////////

//
// Export data items from a 60GHz mmWave sensor, train a NN to diagnose pipeline issues, and inspect model results w/ output images on a web app.
//
// For more information:
// https://www.theamplituhedron.com/projects/AI_assisted_Pipeline_Diagnostics_and_Crack_Inspection_w_mmWave/
//
//
// Connections
// Arduino Nicla Vision :  
//                                Arduino Nano
// UART_TX (PA_9)   --------------- A0
// UART_RX (PA_10)  --------------- A1


// Include the required libraries:
#include <WiFi.h>
#include "camera.h"
#include "gc2145.h"

// Include the Edge Impulse model converted to an Arduino library:
#include <AI-assisted_Pipeline_Diagnostics_inferencing.h>

// Define the required parameters to run an inference with the Edge Impulse model.
#define FREQUENCY_HZ        EI_CLASSIFIER_FREQUENCY
#define INTERVAL_MS         (1000 / (FREQUENCY_HZ + 1))

// Define the features array to classify one frame of data.
float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
size_t feature_ix = 0;

// Define the threshold value for the model outputs (predictions).
float threshold = 0.60;

// Define the pipeline diagnostic class names:
String classes[] = {"Clogged", "Cracked", "Leakage"};

char ssid[] = "<_SSID_>";        // your network SSID (name)
char pass[] = "<_PASSWORD_>";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                // your network key Index number (needed only for WEP)

// Define the server on LattePanda 3 Delta.
char server[] = "192.168.1.22";
// Define the web application path.
String application = "/pipeline_diagnostics_interface/update_server.php";

// Initialize the WiFiClient object.
WiFiClient client; /* WiFiSSLClient client; */

// Define the required camera settings for the 2-megapixel CMOS camera (GC2145).
GC2145 galaxyCore;
Camera cam(galaxyCore);

// Define the camera frame buffer.
FrameBuffer fb;

// Create a struct (data) including all 60GHz mmWave sensor data parameters:
struct data {
  float p1;
  float p2;
  float p3;
  float p4;
  float p5;
  float p6;
  float p7;
};

// Define the data holders:
struct data mm;
int predicted_class = -1;
String data_packet = "";
int del_1, del_2, del_3, del_4, del_5, del_6;

void setup(){
  Serial.begin(115200);

  // Initialize the hardware serial port (Serial1) to communicate with Arduino Nano via serial communication. 
  Serial1.begin(115200, SERIAL_8N1);

  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(ssid, pass);
  // Attempt to connect to the Wi-Fi network:
  while(WiFi.status() != WL_CONNECTED){
    // Wait for the connection:
    delay(500);
    Serial.print(".");
  }
  // If connected to the network successfully:
  Serial.println("Connected to the Wi-Fi network successfully!");

  // Define the pixel format and the FPS settings.
  // Then, initialize the GC2145 camera.
  if (!cam.begin(CAMERA_R320x320, CAMERA_RGB565, 30)) { // CAMERA_R320x240, CAMERA_R320x320
    Serial.println("GC2145 camera: initialization failed!");
  }else{
    Serial.println("GC2145 camera: initialized successfully!");
  }
}

void loop(){
  // Obtain the data packet and commands transferred by Arduino Nano via serial communication.
  if(Serial1.available() > 0){
    data_packet = Serial1.readString();
  }

  if(data_packet != ""){
    
    Serial.print("\nReceived Data Packet: "); Serial.println(data_packet+"\n");
    
    // If Arduino Nano sends the Data command via serial communication:
    if(data_packet.startsWith("Data")){
      // Glean information as substrings from the transferred data packet by Arduino Nano.
      del_1 = data_packet.indexOf("&");
      del_2 = data_packet.indexOf("&", del_1 + 1);
      String data_record = data_packet.substring(del_1 + 1, del_2);
      String selected_class = data_packet.substring(del_2 + 1);

      // Create the request string.
      String request = "?data=OK&mmWave=" + String(data_record)
                     + "&class=" + String(selected_class);
      // Send the obtained mmWave data parameters and the selected pipeline diagnostic class to the web application via an HTTP GET request.
      make_a_get_post_request(false, request);
    }
    
    // If Arduino Nano sends the Run command via serial communication:
    if(data_packet.startsWith("Run")){
      // Glean information as substrings from the transferred data packet by Arduino Nano.
      del_1 = data_packet.indexOf("&");
      String data_record = data_packet.substring(del_1 + 1);
      // Elicit data items from the generated substring.
      del_1 = data_record.indexOf(",");
      del_2 = data_record.indexOf(",", del_1 + 1);
      del_3 = data_record.indexOf(",", del_2 + 1);
      del_4 = data_record.indexOf(",", del_3 + 1);
      del_5 = data_record.indexOf(",", del_4 + 1);
      del_6 = data_record.indexOf(",", del_5 + 1);
      // Convert and store the elicited data items.
      mm.p1 = data_record.substring(0, del_1).toFloat();
      mm.p2 = data_record.substring(del_1 + 1, del_2).toFloat();
      mm.p3 = data_record.substring(del_2 + 1, del_3).toFloat();
      mm.p4 = data_record.substring(del_3 + 1, del_4).toFloat();
      mm.p5 = data_record.substring(del_4 + 1, del_5).toFloat();
      mm.p6 = data_record.substring(del_5 + 1, del_6).toFloat();
      mm.p7 = data_record.substring(del_6 + 1).toFloat();

      // Run the Edge Impulse model to make predictions on the pipeline diagnostic classes.
     run_inference_to_make_predictions(1);

      // Capture a picture with the GC2145 camera.
      take_picture();

      // Create the request string.
      String request = "?results=OK&mmWave=" + String(data_record)
                     + "&class=" + classes[predicted_class];
      // Send the obtained mmWave data parameters, the recently captured image, and the model detection result to the web application via an HTTP POST request.
      make_a_get_post_request(true, request);
    }

    // Clear the received data packet.
    data_packet = "";
  }
}

void run_inference_to_make_predictions(int multiply){
  // Scale (normalize) data items depending on the given model:
  float scaled_p1 = mm.p1;
  float scaled_p2 = mm.p2;
  float scaled_p3 = mm.p3;
  float scaled_p4 = mm.p4;
  float scaled_p5 = mm.p5;
  float scaled_p6 = mm.p6;
  float scaled_p7 = mm.p7;

  // Copy the scaled data items to the features buffer.
  // If required, multiply the scaled data items while copying them to the features buffer.
  for(int i=0; i<multiply; i++){  
    features[feature_ix++] = scaled_p1;
    features[feature_ix++] = scaled_p2;
    features[feature_ix++] = scaled_p3;
    features[feature_ix++] = scaled_p4;
    features[feature_ix++] = scaled_p5;
    features[feature_ix++] = scaled_p6;
    features[feature_ix++] = scaled_p7;
  }

  // Display the progress of copying data to the features buffer.
  Serial.print("Features Buffer Progress: "); Serial.print(feature_ix); Serial.print(" / "); Serial.println(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
  
  // Run inference:
  if(feature_ix == EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE){    
    ei_impulse_result_t result;
    // Create a signal object from the features buffer (frame).
    signal_t signal;
    numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    // Run the classifier:
    EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
    ei_printf("\nrun_classifier returned: %d\n", res);
    if(res != 0) return;

    // Print the inference timings on the serial monitor.
    ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n", 
        result.timing.dsp, result.timing.classification, result.timing.anomaly);

    // Obtain the prediction results for each label (class).
    for(size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++){
      // Print the prediction results on the serial monitor.
      ei_printf("%s:\t%.5f\n", result.classification[ix].label, result.classification[ix].value);
      // Get the predicted label (class).
      if(result.classification[ix].value >= threshold) predicted_class = ix;
    }
    Serial.print("\nPredicted Class: "); Serial.println(predicted_class);

    // Detect anomalies, if any:
    #if EI_CLASSIFIER_HAS_ANOMALY == 1
      ei_printf("Anomaly : \t%.3f\n", result.anomaly);
    #endif

    // Clear the features buffer (frame):
    feature_ix = 0;
  }
}

void make_a_get_post_request(bool post, String request){
  // Connect to the web application named pipeline_diagnostics_interface. Change '80' with '443' if you are using SSL connection.
  if (client.connect(server, 80)){
    // If successful:
    Serial.println("\nConnected to the web application successfully!\n");
    // Create the query string:
    String query = application + request;
    // Transfer information to the web application via an HTTP POST or GET request depending on the given data parameter type.
    if(post){
      // Make an HTTP POST request:
      String head = "--PipeDetection\r\nContent-Disposition: form-data; name=\"captured_image\"; filename=\"new_image.txt\"\r\nContent-Type: text/plain\r\n\r\n";
      String tail = "\r\n--PipeDetection--\r\n";
      // Get the total message length.
      uint32_t totalLen = head.length() + cam.frameSize() + tail.length();
      // Start the request:
      client.println("POST " + query + " HTTP/1.1");
      client.println("Host: 192.168.1.22");
      client.println("Content-Length: " + String(totalLen));
      client.println("Content-Type: multipart/form-data; boundary=PipeDetection");
      client.println();
      client.print(head);
      client.write(fb.getBuffer(), cam.frameSize());
      client.print(tail);
      client.println("Connection: close");
      client.println();
      // Wait until transferring the image buffer.
      delay(3000);
      // If successful:
      Serial.println("HTTP POST => Data transfer completed!\n");
    }else{
      // Make an HTTP GET request:
      // Start the request:
      client.println("GET " + query + " HTTP/1.1");
      client.println("Host: 192.168.1.22");
      client.println("Connection: close");
      client.println();
      //client.println("Connection: close");
      delay(2000);
      // If successful:
      Serial.println("HTTP GET => Data transfer completed!\n");
    }
  }else{
    Serial.println("\nConnection failed to the web application!\n");
    delay(2000);
  }
}

void take_picture(){
  // Capture a picture with the GC2145 camera.
  // If successful:
  if(cam.grabFrame(fb, 3000) == 0){
   Serial.println("\nGC2145 camera: image captured successfully!");
  }else{
    Serial.println("\nGC2145 camera: image capture failed!");
  }
  delay(2000);
}
